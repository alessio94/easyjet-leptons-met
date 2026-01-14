/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  JetDecoratorAlg:
  An alg that copies jet information to aux decorations so can be
  output branch.
*/


#ifndef EASYJET_JETDECORATORALG
#define EASYJET_JETDECORATORALG

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODTrigger/JetRoIContainer.h>
#include <xAODJet/JetContainer.h>
#include <TrigDecisionTool/TrigDecisionTool.h>
#include <TrigBtagEmulationTool/ITrigBtagEmulationTool.h>

#include <StoreGate/WriteDecorHandleKeyArray.h>

#include <AthenaKernel/Units.h>


namespace Easyjet
{

  class JetDecoratorAlg final : public AthReentrantAlgorithm
  {
  public:
    JetDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::JetContainer> m_jetsInKey{
      this, "jetsIn", "", "reco jet container"
    };

    SG::ReadHandleKey<xAOD::JetContainer> m_truthJetsInKey{
      this, "truthJets", "AntiKt4TruthDressedWZJets", "truth jet container"
    };

    Gaudi::Property<float> m_minDR{
      this, "minDR", 0.3, "DR matching between reco and truth jets"
    };

    Gaudi::Property<float> m_minTruthPt{
      this, "minTruthPt", 20.*Athena::Units::GeV, "minimum pT of truth jets"
    };

    Gaudi::Property<bool> m_useEmulationTool{this, "useEmulationTool", false};

    SG::ReadDecorHandleKey<xAOD::JetContainer> m_truthLabelDecorKey;

    SG::WriteDecorHandleKey<xAOD::JetContainer> m_bJetTruthPtDecorKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_bJetTruthDRDecorKey;

    Gaudi::Property<bool> m_isMC{
      this, "isMC", false, "is this simulation?"
    };

    Gaudi::Property<bool> m_doL1Matching{this, "doL1Matching", false, "do trigger HLT matching?" };
    Gaudi::Property<bool> m_doHLTMatching{this, "doHLTMatching", false, "do trigger L1 matching?" };

    PublicToolHandle<Trig::TrigDecisionTool> m_trigDecTool{
      this, "TrigDecisionTool", "", "Trigger decision tool"
    };

    ToolHandle<Trig::ITrigBtagEmulationTool> m_emulationTool {this, "trigBtagEmulationTool", "", ""};

    Gaudi::Property<std::vector<std::string>> m_triggers{
      this, "triggerList", {}, "List of triggers to match"
    };

    SG::ReadHandleKey<xAOD::JetRoIContainer> m_L1JetsInKey{
      this, "L1Jets", "LVL1JetRoIs", "L1 jet container"
    };

    SG::ReadHandleKey<xAOD::JetContainer> m_HLTJetsInKey {
      this, "HLTJets", "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets", "HLT jet container"
    };

    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, std::vector<int>> m_jetL1ThresholdsDecorKeys{this,"L1ThresholdKeys",{},"DO NOT CONFIGURE"};
    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, float> m_jetL1EtDecorKeys{this,"L1EtKeys",{},"DO NOT CONFIGURE"};
    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, float> m_jetL1EtaDecorKeys{this,"L1EtaKeys",{},"DO NOT CONFIGURE"};
    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, float> m_jetL1PhiDecorKeys{this,"L1PhiKeys",{},"DO NOT CONFIGURE"};
    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, float> m_jetL1DRDecorKeys{this,"L1DRKeys",{},"DO NOT CONFIGURE"};

    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, std::vector<int> > m_jetHLTThresholdsDecorKeys{this,"HLTThresholdKeys",{},"DO NOT CONFIGURE"};
    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, float> m_jetHLTPtDecorKeys{this,"HLTPtKeys",{},"DO NOT CONFIGURE"};
    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, float> m_jetHLTEtaDecorKeys{this,"HLTEtaKeys",{},"DO NOT CONFIGURE"};
    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, float> m_jetHLTPhiDecorKeys{this,"HLTPhiKeys",{},"DO NOT CONFIGURE"};
    SG::WriteDecorHandleKeyArray<xAOD::JetContainer, float> m_jetHLTDRDecorKeys{this,"HLTDRKeys",{},"DO NOT CONFIGURE"};

    bool isSameJet(const xAOD::IParticle *jet1, const xAOD::IParticle *jet2) const;

  };

}

#endif
