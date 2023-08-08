#include "h5histograms.h"
#include "MassPlaneBoostHistograms.h"
#include "xAODJet/Jet.h"

#include "H5Cpp.h"

#include "boost/histogram.hpp"

namespace {

  constexpr float mhiggs = 125e3;

  namespace bh = boost::histogram;
  using namespace bhist;

  using dax_t = bh::axis::regular<double>;
  auto make_mass_hist(size_t nbins = 150) {
    return bh::make_weighted_histogram(
      dax_t(nbins, 50e3, 200e3, "mh1"),
      dax_t(nbins, 50e3, 200e3, "mh2"));
  }
  auto make_dihiggs_mass_hist(size_t nbins = 500) {
    return bh::make_weighted_histogram(
      dax_t(nbins, 0, 1e6, "mhh"));
  }

  // sortable pair container
  struct JetPair
  {
    const xAOD::Jet* j1;
    const xAOD::Jet* j2;
    float loss;
    bool operator<(const JetPair& other) const {
      return loss < other.loss;
    }
    float pt() const {
      return (j1->p4() + j2->p4()).Pt();
    }
    float mass() const {
      return (j1->p4() + j2->p4()).M();
    }
  };
  // generic pairing functions
  auto dr_loss = [](const xAOD::Jet* j1, const xAOD::Jet* j2) -> float {
    return j1->p4().DeltaR(j2->p4());
  };
  auto dmh_loss = [](const xAOD::Jet* j1, const xAOD::Jet* j2) -> float {
    return std::abs((j1->p4() + j2->p4()).M() - mhiggs);
  };
  auto pt_loss = [](const xAOD::Jet* j1, const xAOD::Jet* j2) -> float {
    return -(j1->pt() + j2->pt());
  };
  // generic pairing impelmentation
  template <typename F>
  std::vector<JetPair> get_pairs(
    const std::vector<const xAOD::Jet*>& jets,
    F loss = dr_loss) {
    // build all possible pairs
    std::vector<JetPair> pairs;
    for (size_t i = 0; i < jets.size(); i++) {
      for (size_t j = i+1; j < jets.size(); j++) {
        auto j1 = jets.at(i);
        auto j2 = jets.at(j);
        pairs.emplace_back<JetPair>({j1, j2, loss(j1,j2)});
      }
    }
    std::sort(pairs.begin(), pairs.end());
    std::vector<JetPair> unique_pairs;
    std::set<const xAOD::Jet*> used;
    for (const auto& pair: pairs) {
      if (!used.count(pair.j1) && !used.count(pair.j2)) {
        unique_pairs.push_back(pair);
        used.insert(pair.j1);
        used.insert(pair.j2);
      }
    }
    return unique_pairs;
  }
  // the baseline pairing minimizes the delta-R for the jet pair with
  // the highest pt
  std::vector<JetPair> get_baseline_pairs(const Jets& jets) {
    if (jets.size() != 4) {
      throw std::logic_error("baseline pairing isn't defined for njets != 4");
    }
    std::vector<std::pair<JetPair,JetPair>> pairs;
    // pairing the first jet with every other jet should get us all
    // the combinations
    for (size_t i = 1; i < jets.size(); i++) {
      Jets pair{jets.at(0), jets.at(i)};
      Jets complement;
      std::set_difference(
        jets.begin(), jets.end(),
        pair.begin(), pair.end(),
        std::back_inserter(complement));
      auto make_pair = [](auto& v) {
        assert(v.size() == 2);
        auto* j1 = v.at(0);
        auto* j2 = v.at(1);
        return JetPair{j1, j2, dr_loss(j1, j2)};
      };
      JetPair h1 = make_pair(pair);
      JetPair h2 = make_pair(complement);
      if (h1.pt() > h2.pt()) {
        pairs.emplace_back(h1, h2);
      } else {
        pairs.emplace_back(h2, h1);
      }
    }
    auto [h1, h2] = *std::min_element(pairs.begin(), pairs.end());
    return {h1, h2};
  }

  // utility functions
  template <typename T>
  std::string stringify(const T& c) {
    std::string out;
    bool first = true;
    for (const auto& item: c) {
      if (!first) out.append(", ");
      out.append(item);
      first = false;
    }
    return out;
  }

  // convenience aliases
  using Pairs = std::vector<JetPair>;
  using PairingFunction = std::function<Pairs(const Jets&)>;
  using TruthMatchFunction = std::function<bool(const Pairs&)>;
}

namespace bhist {

  // default truth matcing function, always true
  TruthMatchFunction true_func = [](const Pairs&) { return true; };


  // This contains the histograms for one set of masses (mass plane
  // and dihiggs).
  class MassHist
  {
  public:
    MassHist(
      const std::string name,
      PairingFunction,
      TruthMatchFunction = true_func);
    void fill(const Jets, float weight);
    void write(H5::Group& output_group);
  private:
    decltype(make_mass_hist()) m_hist;
    decltype(make_dihiggs_mass_hist()) m_dihiggs;
    std::string m_name;
    PairingFunction m_pair;
    TruthMatchFunction m_truth_match;
  };


  MassHist::MassHist(
    std::string name,
    PairingFunction func,
    TruthMatchFunction truth_func
    ):
    m_hist(make_mass_hist()),
    m_dihiggs(make_dihiggs_mass_hist()),
    m_name(name),
    m_pair(func),
    m_truth_match(truth_func)
  {
  }


  void MassHist::fill(const Jets jets, float weight) {
    auto pairs = m_pair(jets);
    assert(pairs.size() == 2);
    if (!m_truth_match(pairs)) return;
    m_hist(pairs.at(0).mass(), pairs.at(1).mass(), bh::weight(weight));
    m_dihiggs(
      (pairs.at(0).j1->p4() + pairs.at(0).j2->p4() +
       pairs.at(1).j1->p4() + pairs.at(1).j2->p4()).M(),
      bh::weight(weight));
  }


  void MassHist::write(H5::Group& group) {
    H5::Group sub = group.createGroup(m_name);
    h5h::write_hist_to_group(sub, m_hist, "h1h2");
    h5h::write_hist_to_group(sub, m_dihiggs, "hh");
  }


  // this is the top level class, which owns a lot of the above
  // objects.
  MassPlaneHists::MassPlaneHists(const MassPlaneHistsConfig& cfg)
  {
    // set up for matching possibilities
    auto meths = cfg.matching_methods;
    if (meths.empty()) meths = {"none given"};
    auto& mpl = m_mpl;
    const std::string& pfx = cfg.truth_matching_prefix;
    // build the truth matching function, assumes that there will be a
    // parent particle "Index" that shoudl match between the children.
    TruthMatchFunction truthmatch = [
      idx=SG::AuxElement::ConstAccessor<char>(pfx + "Index")
      ] (const Pairs& pairs) -> bool
    {
      for (const auto& pair: pairs) {
        if (idx(*pair.j1) != idx(*pair.j2)) return false;
      }
      return true;
    };
    // pass everyting if the matching prefix is empty
    const auto truth = pfx.empty() ? true_func : truthmatch;
    std::set<std::string> all;
    auto consider = [&meths, &mpl, &all, &truth]
      (const std::string& key, auto func)
    {
      if (auto h = meths.extract(key)) {
        mpl.push_back(std::make_unique<MassHist>(h.value(), func, truth));
      }
      all.insert(key);
    };
    consider("dr", [](const Jets& j) { return get_pairs(j, dr_loss); });
    consider("dmh", [](const Jets& j) { return get_pairs(j, dmh_loss); });
    consider("pt", [](const Jets& j) { return get_pairs(j, pt_loss); });
    consider("2301_03212", [](const Jets& j) { return get_baseline_pairs(j); });
    if (!meths.empty()) {
      throw std::runtime_error(
        "unknown matching methods [" + stringify(meths) + "],"
        " options: [" + stringify(all) + "]");
    }
  }


  MassPlaneHists::~MassPlaneHists() = default;


  void MassPlaneHists::fill(Jets jets, float weight)
  {
    if (jets.size() < 4) return;
    // nth element is kind of weird: it tries to get the nth position
    // correct, which in the case of 4 jets is the beginning + 3.
    std::nth_element(
      jets.begin(),
      jets.begin() + 3,
      jets.end(),
      [](auto& j1, auto& j2){ return j1->pt() > j2->pt(); });
    jets.resize(4);
    for (auto& hist: m_mpl) hist->fill(jets, weight);
  }


  void MassPlaneHists::write(H5::Group& group) const {
    for (const auto& hist: m_mpl) {
      hist->write(group);
    }
  }


}
