#include "TruthParentDecoratorAlg.h"

#include "StoreGate/WriteDecorHandle.h"

namespace {

  using Barcodex = TruthParentDecoratorAlg::Barcodex;
  using IPMap = TruthParentDecoratorAlg::IPMap;
  using JC = TruthParentDecoratorAlg::JC;
  using TPC = TruthParentDecoratorAlg::TPC;

  // structure to hold info on a matched parent particle
  struct MatchedParent
  {
    const xAOD::TruthParticle* parent;
    const xAOD::TruthParticle* child;
    float deltaR;
    unsigned int parent_index;
  };

  unsigned long long matchMask(const std::vector<MatchedParent>& matches) {
    unsigned long long mask = 0x0;
    for (const auto& match: matches) {
      constexpr size_t max_idx = std::numeric_limits<decltype(mask)>::digits;
      if (match.parent_index > max_idx) {
        throw std::runtime_error(
          "parent index overflowed the match mask "
          "[index: "  + std::to_string(match.parent_index) +
          " , max_mask: " + std::to_string(max_idx) + "]");
      }
      mask |= (0x1 << match.parent_index);
    }
    return mask;
  }

  // debugging functions
  std::string join(
    const std::vector<std::string>& v,
    const std::string& sep = ", ")
  {
    std::string out;
    for (unsigned int pos = 0; pos < v.size(); pos++) {
      out.append(v.at(pos));
      if (pos + 1 < v.size()) out.append(sep);
    }
    return out;
  }
  template <typename T>
  std::vector<std::string> stringify(const T& container) {
    std::vector<std::string> out;
    for (const auto& v: container) out.push_back(std::to_string(v));
    return out;
  }
  std::string listAllChildren(const xAOD::TruthParticle* p) {
    std::vector<std::string> output;
    for (unsigned int child_n = 0; child_n < p->nChildren(); child_n++) {
      output.push_back(std::to_string(p->child(child_n)->pdgId()));
    }
    if (output.empty()) return "none";
    return "[" + join(output) + "]";
  }


  // debugging functions using MsgStream
  void logInputs(
    MsgStream& msg,
    SG::ReadHandle<JC>& targets,
    SG::ReadHandle<TPC>& truth,
    std::vector<SG::ReadHandle<TPC>>& cascades_raw,
    const MSG::Level level = MSG::DEBUG)
  {
    if (msg.level() > level) return;
    unsigned int n_cascade_candidates = 0;
    for (auto& cascade: cascades_raw) {
      n_cascade_candidates += cascade->size();
    }
    msg << level <<
      "n_targets: " << targets->size() << ", "
      "n_parents: " << truth->size() << ", "
      "n_cascade_candidates: " << n_cascade_candidates <<
      endmsg;
  }
  void logIPMap(
    MsgStream& msg,
    const IPMap& ipmap,
    const MSG::Level level = MSG::VERBOSE)
  {
    if (msg.level() > level) return;
    for (const auto& [barcode, children]: ipmap) {
      std::set<int> ids;
      for (const xAOD::TruthParticle* dup: children) ids.insert(dup->pdgId());
      msg << level << "barcode: " << barcode << ", n_dup: " << children.size()
          << " idg_ids: [" << join(stringify(ids)) << "]" << endmsg;
    }
  }


  // functions to traverse barcodex and disambiguate the selected
  // child
  std::set<int> findAllChildren(
    int parent, const Barcodex& barcodex,
    std::set<int> history = {})
  {
    auto itr = barcodex.find(parent);
    if (itr == barcodex.end()) {
      auto hist = stringify(history);
      throw std::runtime_error(
        "can't find barcode " + std::to_string(parent) + " history:"
        " {" + join(hist) + "}");
    }
    const std::set<int>& children = itr->second;
    if (children.empty()) return std::set<int>{parent};
    std::set<int> all_children;
    if (!history.insert(parent).second) {
      auto hist = stringify(history);
      throw std::runtime_error(
        "found cycle, tried to add " + std::to_string(parent) + " to "
        "{" + join(hist) + "}");
    }
    for (int child: children) {
      all_children.merge(findAllChildren(child, barcodex, history));
    }
    return all_children;
  }


  const xAOD::TruthParticle* selectChild(
    MsgStream& msg,
    const IPMap::mapped_type& barkids,
    MSG::Level level = MSG::DEBUG)
  {
    const xAOD::TruthParticle* child = *barkids.begin();
    if (barkids.size() > 1) {
      std::set<int> pdg_ids;
      TLorentzVector sum_p4;
      // look at duplicates, take the one with the most children
      for (const xAOD::TruthParticle* dupkid: barkids) {
        sum_p4 += dupkid->p4();
        pdg_ids.insert(dupkid->pdgId());
        if (dupkid->nChildren() > child->nChildren()) child = dupkid;
      }
      if (pdg_ids.size() != 1) {
        throw std::runtime_error(
          "same barcode, different pdgid: [" + join(stringify(pdg_ids)) + "]");
      }
      if (float dr = child->p4().DeltaR(sum_p4); dr > 0.001) {
        throw std::runtime_error(
          "Same barcode, different vector: "
          "{"
          "deltaR: " + std::to_string(dr) + ", "
          "pdgid: " + std::to_string(child->pdgId()) + "}"
          );
      }
    }
    if (msg.level() <= level) {
      std::vector<std::string> child_info {
        "child_pdgid: " + std::to_string(child->pdgId()),
        "child_n_dups: " + std::to_string(barkids.size()),
      };
      std::vector<std::string> dup_info;
      for (const xAOD::TruthParticle* dupkid: barkids) {
        dup_info.push_back(listAllChildren(dupkid));
      }
      child_info.push_back("children: " + join(dup_info));
      msg << level << "Found child: {" + join(child_info) + "}" << endmsg;
    }
    return child;
  }


  bool isOriginal(const xAOD::TruthParticle* p) {
    for (unsigned int parent_n = 0; parent_n < p->nParents(); parent_n++) {
      const xAOD::TruthParticle* parent = p->parent(parent_n);
      if (!parent) throw std::runtime_error("broken truth record");
      if (parent->pdgId() == p->pdgId()) return false;
    }
    return true;
  }


  // these functions are for dealing with specific vertices
  bool isHeavyFlavor(const xAOD::TruthParticle* p) {
    return p->hasCharm() || p->hasBottom();
  }
  const xAOD::TruthParticle* getParent(const xAOD::TruthParticle* p) {
    if (int n_parents = p->nParents(); n_parents != 1) {
      throw std::logic_error(
        "can't get parent [n_parents: " + std::to_string(n_parents) + "]");
    }
    return p->parent(0);
  }
  bool isSoftLepton(const xAOD::TruthParticle* p) {
    const xAOD::TruthParticle* parent = getParent(p);
    return isHeavyFlavor(parent) && p->isChLepton();
  }
  bool isSoftCharm(const xAOD::TruthParticle* p) {
    const xAOD::TruthParticle* parent = getParent(p);
    return parent->hasBottom() && p->hasCharm();
  }


}

TruthParentDecoratorAlg::TruthParentDecoratorAlg(
  const std::string& name,
  ISvcLocator* loc):
  AthReentrantAlgorithm(name, loc)
{
  // these aren't user configurable
  declare(m_target_pdgid_key);
  declare(m_target_dr_truth_key);
  declare(m_target_link_key);
  declare(m_target_index_key);
  declare(m_target_n_matched_key);
  declare(m_target_match_mask_key);
  declare(m_match_pdgid_key);
  declare(m_match_children_key);
  declare(m_match_link_key);
}

StatusCode TruthParentDecoratorAlg::initialize() {
  // initialize inputs
  ATH_CHECK(m_parents_key.initialize());
  ATH_CHECK(m_target_container_key.initialize());
  ATH_CHECK(m_cascades_key.initialize());
  // weird hack because Gaudi can't handle an infinite default
  if (m_match_delta_r.value() <= 0) m_match_delta_r.value() = INFINITY;
  // initialize outputs
  std::string jc = m_target_container_key.key();
  std::string pfx = jc + "." + m_prefix.value();
  m_target_pdgid_key = pfx + "PdgId";
  m_target_dr_truth_key = pfx + "DRTruthParticle";
  m_target_link_key = pfx + "Link";
  m_target_index_key = pfx + "Index";
  m_target_n_matched_key = pfx + "NMatchedChildren";
  m_target_match_mask_key = pfx + "ParentsMask";
  m_match_pdgid_key = pfx + "MatchingParticlePdgId";
  m_match_children_key = pfx + "MatchingParticleNChildren";
  m_match_link_key = pfx + "MatchingParticleLink";
  ATH_CHECK(m_target_pdgid_key.initialize());
  ATH_CHECK(m_target_dr_truth_key.initialize());
  ATH_CHECK(m_target_link_key.initialize());
  ATH_CHECK(m_target_index_key.initialize());
  ATH_CHECK(m_target_n_matched_key.initialize());
  ATH_CHECK(m_target_match_mask_key.initialize());
  ATH_CHECK(m_match_pdgid_key.initialize());
  ATH_CHECK(m_match_children_key.initialize());
  ATH_CHECK(m_match_link_key.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TruthParentDecoratorAlg::execute(const EventContext& cxt) const
{
  ATH_MSG_DEBUG("Executing");

  //////////////////////////////////
  // part 1: read in the particles
  //////////////////////////////////
  SG::ReadHandle<JC> targets(m_target_container_key, cxt);
  if (targets->empty()) return StatusCode::SUCCESS;

  // for lack of a better idea, store truth parents sorted by mass
  SG::ReadHandle<TPC> phandle(m_parents_key, cxt);
  std::vector<const xAOD::TruthParticle*> psort(
    phandle->begin(), phandle->end());
  std::sort(
    psort.begin(), psort.end(),
    [](const auto* p1, const auto* p2) {return p1->m() > p2->m();}
    );

  std::vector<SG::ReadHandle<TPC>> cascades_raw;
  for (const auto& key: m_cascades_key) {
    cascades_raw.emplace_back(key, cxt);
  }
  logInputs(msgStream(), targets, phandle, cascades_raw);

  ///////////////////////////////
  // part 2: build the barcodex
  ///////////////////////////////
  Barcodex barcodex;
  IPMap ipmap;
  addTruthContainer(barcodex, ipmap, *phandle);
  for (auto& cascade: cascades_raw) {
    addTruthContainer(barcodex, ipmap, *cascade);
  }
  logIPMap(msg(), ipmap);

  ATH_MSG_DEBUG("merged cascade contains " << barcodex.size() << " particles");

  /////////////////////////////////////////////
  // Part 3: build map from targets to parents
  /////////////////////////////////////////////
  std::unordered_map<const J*, std::vector<MatchedParent>> labeled_targets;
  std::set<int> parentids(m_parent_pdgids.begin(), m_parent_pdgids.end());
  unsigned int n_parents = 0;
  for (const auto* p: psort) {
    if (!parentids.count(p->pdgId())) continue;
    if (!isOriginal(p)) continue;
    unsigned int parent_index = n_parents++;
    ATH_MSG_VERBOSE("pdgid: " << p->pdgId() << ", barcode: " << p->barcode());
    for (int child_barcode: findAllChildren(p->barcode(), barcodex)) {
      IPMap::mapped_type& barkids = ipmap.at(child_barcode);
      const xAOD::TruthParticle* child = selectChild(msg(), barkids);
      std::vector<std::pair<float, const J*>> drs;
      for (const auto* j: *targets) {
        drs.emplace_back(j->p4().DeltaR(child->p4()), j);
      };
      const auto& nearest = std::min_element(drs.begin(), drs.end());
      MatchedParent match;
      match.parent = p;
      match.child = child;
      match.deltaR = nearest->first;
      match.parent_index = parent_index;
      if (match.deltaR < m_match_delta_r) {
        labeled_targets[nearest->second].push_back(match);
      }
    }
  }

  ///////////////////////
  // Part 4: decorate!
  ///////////////////////
  using ull_t = unsigned long long;
  SG::WriteDecorHandle<JC,int> pdgid(m_target_pdgid_key, cxt);
  SG::WriteDecorHandle<JC,float> deltaR(m_target_dr_truth_key, cxt);
  SG::WriteDecorHandle<JC,JL> link(m_target_link_key, cxt);
  SG::WriteDecorHandle<JC,char> index(m_target_index_key, cxt);
  SG::WriteDecorHandle<JC,char> nMatched(m_target_n_matched_key, cxt);
  SG::WriteDecorHandle<JC,ull_t> mask(m_target_match_mask_key, cxt);
  SG::WriteDecorHandle<JC,int> matchPdgId(m_match_pdgid_key, cxt);
  SG::WriteDecorHandle<JC,int> matchChildCount(m_match_children_key, cxt);
  SG::WriteDecorHandle<JC,JL> matchLink(m_match_link_key, cxt);

  for (const J* j: *targets) {
    if (labeled_targets.count(j)) {
      const std::vector<MatchedParent>& matches = labeled_targets.at(j);
      auto min_dr = [](auto& p1, auto& p2) {
        return p1.deltaR < p2.deltaR;
      };
      const MatchedParent& nearest = *std::min_element(
        matches.begin(), matches.end(), min_dr);
      const xAOD::TruthParticle* p = nearest.parent;
      pdgid(*j) = p->pdgId();
      deltaR(*j) = nearest.deltaR;
      auto* container = static_cast<const TPC*>(p->container());
      link(*j) = JL(*container, p->index());
      index(*j) = nearest.parent_index;
      nMatched(*j) = matches.size();
      mask(*j) = matchMask(matches);
      const xAOD::TruthParticle* child = nearest.child;
      matchPdgId(*j) = child->pdgId();
      matchChildCount(*j) = child->nChildren();
      auto* matchedContainer = static_cast<const TPC*>(child->container());
      matchLink(*j) = JL(*matchedContainer, child->index());
    } else {
      pdgid(*j) = 0;
      deltaR(*j) = NAN;
      link(*j) = JL();
      index(*j) = -1;
      nMatched(*j) = -1;
      mask(*j) = 0x0;
      matchPdgId(*j) = 0;
      matchChildCount(*j) = 0;
      matchLink(*j) = JL();
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode TruthParentDecoratorAlg::finalize() {
  return StatusCode::SUCCESS;
}

void TruthParentDecoratorAlg::addTruthContainer(
  Barcodex& barcodex,
  IPMap& ipmap,
  const xAOD::TruthParticleContainer& container) const {

  std::set<int> targid(m_cascade_pdgids.begin(), m_cascade_pdgids.end());
  // we allow decays through any of the parent pdgids
  targid.insert(m_parent_pdgids.begin(), m_parent_pdgids.end());

  // this determines if a cascade vertex should be saved or not
  auto cascadeWants = [
    &targid,
    b=m_add_b,
    c=m_add_c,
    vsl=m_veto_soft_lepton,
    vsc=m_veto_soft_charm
    ] (const xAOD::TruthParticle* p) {
    if (int n_parents = p->nParents(); n_parents == 1) {
      if (vsl && isSoftLepton(p)) return false;
      if (vsc && isSoftCharm(p)) return false;
    }
    if (targid.count(p->pdgId())) return true;
    if (b && p->hasBottom()) return true;
    if (c && p->hasCharm()) return true;
    return false;
  };

  // insert a particle into the record, return the child set
  auto insert = [&barcodex, &ipmap](const xAOD::TruthParticle* p) -> auto& {
    ipmap[p->barcode()].insert(p);
    return barcodex[p->barcode()];
  };

  for (const xAOD::TruthParticle* p: container) {
    if (cascadeWants(p)) {
      auto& child_set = insert(p);
      for (unsigned int child_n = 0; child_n < p->nChildren(); child_n++) {
        const xAOD::TruthParticle* c = p->child(child_n);
        if (!c) throw std::runtime_error("null child in truth record");
        if (cascadeWants(c)) {
          insert(c);
          child_set.insert(c->barcode());
        }
      };
    }
  }
}
