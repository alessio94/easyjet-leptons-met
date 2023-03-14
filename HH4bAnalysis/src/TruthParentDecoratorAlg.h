#ifndef TRUTH_PARENT_DECORATOR_ALG
#define TRUTH_PARENT_DECORATOR_ALG

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// no need for forward declaration here, no one reads this header
#include "xAODBase/IParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

class TruthParentDecoratorAlg: public AthReentrantAlgorithm
{
public:
  using Barcodex = std::map<int, std::set<int>>;
  using IPMap = std::map<int, std::set<const xAOD::TruthParticle*>>;
  using JC = xAOD::IParticleContainer;
  using J = xAOD::IParticle;
  using TPC = xAOD::TruthParticleContainer;
  TruthParentDecoratorAlg(const std::string& name, ISvcLocator* loc);
  virtual StatusCode initialize () override;
  virtual StatusCode execute (const EventContext&) const override;
  virtual StatusCode finalize () override;
private:
  void addTruthContainer(Barcodex&, IPMap&, const TPC&) const;
  SG::ReadHandleKey<JC> m_target_container_key{
    this, "targetContainer", "", "target container to decorate"
  };
  Gaudi::Property<std::string> m_prefix{
    this, "decoratorPrefix", "parentBoson", "prefix for decorations"
  };
  Gaudi::Property<std::vector<int>> m_parent_pdgids{
    this, "parentPdgIds", {25}, "PDGIDs of allowed parent particles"
  };
  Gaudi::Property<std::vector<int>> m_cascade_pdgids{
    this, "cascadePdgIds", {-5, 5}, "PDGIDs of particles in the decay chain"
  };
  Gaudi::Property<bool> m_add_b{
    this, "addBs", true, "add all bhadrons to cascade"
  };
  Gaudi::Property<bool> m_add_c{
    this, "addCs", true, "add all chadrons to cascade"
  };
  Gaudi::Property<bool> m_veto_soft_lepton{
    this, "vetoSoftLepton", true, "veto soft lepton decays from cascade"
  };
  Gaudi::Property<bool> m_veto_soft_charm{
    this, "vetoSoftCharm", true, "veto soft charm decays from cascade"
  };
  SG::ReadHandleKey<TPC> m_parents_key{
    this, "parents", "", "truth parent container"
  };
  SG::ReadHandleKeyArray<TPC> m_cascades_key{
    this, "cascades", {"TruthBottom"}, "truth hadron container"
  };
  SG::WriteDecorHandleKey<JC> m_target_pdgid_key;
  SG::WriteDecorHandleKey<JC> m_target_barcode_key;
  SG::WriteDecorHandleKey<JC> m_target_dr_truth_key;
  SG::WriteDecorHandleKey<JC> m_match_pdgid_key;
  SG::WriteDecorHandleKey<JC> m_match_children_key;
  SG::WriteDecorHandleKey<JC> m_match_barcode_key;
};

#endif
