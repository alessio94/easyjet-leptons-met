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
  using JL = ElementLink<JC>;
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
    this, "parentPdgIds", {}, "PDGIDs of allowed parent particles"
  };
  Gaudi::Property<std::vector<int>> m_cascade_pdgids{
    this, "cascadePdgIds", {}, "PDGIDs of particles in the decay chain"
  };
  Gaudi::Property<bool> m_add_b{
    this, "addBsToCascade", false, "add all bhadrons to cascade"
  };
  Gaudi::Property<bool> m_add_c{
    this, "addCsToCascade", false, "add all chadrons to cascade"
  };
  Gaudi::Property<bool> m_veto_soft_lepton{
    this, "vetoSoftLeptonCascade", false,
    "veto soft lepton decays from cascade"
  };
  Gaudi::Property<bool> m_veto_soft_charm{
    this, "vetoSoftCharmCascade", false,
    "veto soft charm decays from cascade"
  };
  SG::ReadHandleKey<TPC> m_parents_key{
    this, "parents", "", "truth parent container"
  };
  SG::ReadHandleKeyArray<TPC> m_cascades_key{
    this, "cascades", {}, "truth hadron container"
  };
  SG::WriteDecorHandleKey<JC> m_target_pdgid_key;
  SG::WriteDecorHandleKey<JC> m_target_barcode_key;
  SG::WriteDecorHandleKey<JC> m_target_dr_truth_key;
  SG::WriteDecorHandleKey<JC> m_target_link_key;
  SG::WriteDecorHandleKey<JC> m_match_pdgid_key;
  SG::WriteDecorHandleKey<JC> m_match_children_key;
  SG::WriteDecorHandleKey<JC> m_match_barcode_key;
  SG::WriteDecorHandleKey<JC> m_match_link_key;
};

#endif
