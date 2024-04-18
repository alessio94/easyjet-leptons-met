/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  JetDecoratorAlg:
  An alg that copies jet information to aux decorations so can be
  output branch.
*/


#ifndef EASYJET_JETDECORATORALG
#define EASYJET_JETDECORATORALG

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODJet/JetContainer.h>

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

    SG::ReadDecorHandleKey<xAOD::JetContainer> m_truthLabelDecorKey;

    SG::WriteDecorHandleKey<xAOD::JetContainer> m_bJetTruthPtDecorKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_bJetTruthDRDecorKey;

  };

}

#endif
