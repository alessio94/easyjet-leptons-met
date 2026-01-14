#include "HDF5Utils/histogram.h"
#include "JetBoostHistograms.h"
#include "xAODJet/Jet.h"

#include "H5Cpp.h"

#include "boost/histogram.hpp"

namespace {
  constexpr const double & pi = std::numbers::pi;

  namespace bh = boost::histogram;

  using dax_t = bh::axis::regular<double>;
  using iax_t = bh::axis::integer<int>;
  using ldax_t = bh::axis::regular<double,bh::axis::transform::log>;
  using pdax_t = bh::axis::regular<
    double, bh::use_default, bh::use_default,
    decltype(bh::axis::option::circular)>;

  auto make_weight_hist() {
    return bh::make_histogram_with(
      std::vector<int>(),
      ldax_t(110, 1e-4, 1e7, "weights"));
  }
}

namespace bhist {

  class IJetHist
  {
  public:
    virtual ~IJetHist() {};
    virtual void fill(const xAOD::Jet&, double weight) = 0;
    virtual void write(H5::Group& group) = 0;
  };

  template <typename T, typename F>
  class JetHist: public IJetHist
  {
  public:
    JetHist(const std::string& name, T hist, F filler);
    void fill(const xAOD::Jet& jet, double weight) override;
    void write(H5::Group& group) override;
  private:
    T m_histogram;
    F m_filler;
    std::string m_name;
  };

  class WeightHist: public IJetHist
  {
  public:
    WeightHist();
    void fill(const xAOD::Jet& jets, double weight) override;
    void write(H5::Group& group) override;
  private:
    decltype(make_weight_hist()) m_histogram;
  };

  JetHists::JetHists()
  {
    const unsigned int n_bins = 100;
    m_hists.emplace_back(
      new JetHist(
        "pt",
        bh::make_weighted_histogram(
          dax_t(n_bins, 20e3, 200e3, "pt")),
        [](const xAOD::Jet& j) { return j.pt(); }));
    m_hists.emplace_back(
      new JetHist(
        "eta",
        bh::make_weighted_histogram(
          dax_t(n_bins, -2.6, 2.6, "eta")),
        [](const xAOD::Jet& j) { return j.eta(); }));
    m_hists.emplace_back(
      new JetHist(
        "phi",
        bh::make_weighted_histogram(
          pdax_t(n_bins, -pi/2, pi/2, "phi")),
        [](const xAOD::Jet& j) { return j.phi(); }));
    m_hists.emplace_back(
      new JetHist(
        "pt_vs_eta",
        bh::make_weighted_histogram(
          dax_t(n_bins, 20e3, 200e3, "pt"),
          dax_t(n_bins, -2.6, 2.6, "eta")),
        [](const xAOD::Jet& j) -> std::tuple<float,float> {
          return {j.pt(), j.eta()};
        }));
    m_hists.emplace_back(
      new JetHist(
        "n_constituents",
        bh::make_weighted_histogram(
          iax_t(0, 100, "n")),
        [](const xAOD::Jet& j) { return j.numConstituents(); }
        ));
    m_hists.emplace_back(new WeightHist());
  }

  JetHists::~JetHists()
  {
  }

  void JetHists::fill(const xAOD::Jet& j, float weight) {
    for (auto& hist: m_hists) {
      hist->fill(j, weight);
    }
  }
  void JetHists::write(H5::Group& group) const {
    for (auto& hist: m_hists) {
      hist->write(group);
    }
  }



// more stuff that should hopefully be cleaned up with concepts
  template<typename>
  struct is_tuple : std::false_type {};
  template<typename ...Types>
  struct is_tuple<std::tuple<Types...>> : std::true_type {};

// JetHist definition
  template <typename T, typename F>
  JetHist<T,F>::JetHist(const std::string& name, T hist, F filler):
    m_histogram(std::move(hist)),
    m_filler(filler),
    m_name(name)
  {
  }
  template <typename T, typename F>
  void JetHist<T,F>::fill(const xAOD::Jet& jet, double weight) {
    // boost::histogram only accepts a list of arguments or a tuple. We
    // could make this part simpler by requiring that all the m_filler
    // functions return a tuple, but that seems a bit cumbersome when
    // most of them only return one value.
    auto fillval = m_filler(jet);
    if constexpr (is_tuple<decltype(fillval)>::value) {
      m_histogram(std::tuple_cat(fillval, std::tuple{bh::weight(weight)}));
    } else {
      m_histogram(m_filler(jet), bh::weight(weight));
    }
  }
  template <typename T, typename F>
  void JetHist<T,F>::write(H5::Group& group) {
    H5Utils::hist::write_hist_to_group(group, m_histogram, m_name);
  }


// weights hist
  WeightHist::WeightHist():
    m_histogram(make_weight_hist())
  {
  }
  void WeightHist::fill([[maybe_unused]] const xAOD::Jet& jets, double weight) {
    m_histogram(weight);
  }
  void WeightHist::write(H5::Group& group) {
    H5Utils::hist::write_hist_to_group(group, m_histogram, "weights");
  }

}
