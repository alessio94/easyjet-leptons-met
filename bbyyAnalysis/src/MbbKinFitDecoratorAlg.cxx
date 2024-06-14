/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MbbKinFitDecoratorAlg.h"

#include "TLorentzVector.h"

namespace HHBBYY{
      MbbKinFitDecoratorAlg ::MbbKinFitDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {
    
  }
    StatusCode MbbKinFitDecoratorAlg ::initialize()
  {
    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    // Intialise syst-aware output decorators
    ATH_CHECK (m_jetOutHandle.initialize(m_systematicsList));
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ANA_CHECK (m_systematicsList.initialize());    

    // Initialise KF Tool
    m_KFTool.reset(new KinematicFitTool("KinematicFitTool"));
    ATH_CHECK (m_KFTool->initialize());
    ATH_CHECK (m_KFTool->setProperty("JetMinPT", m_Jet_Min_pt));
    ATH_CHECK (m_KFTool->setProperty("JetCollection", m_JetAlgo));
    ATH_CHECK (m_KFTool->setProperty("bTagWPDecorName", m_BtaggingWP));
    ATH_CHECK (m_KFTool->setProperty("AnglesResolution", m_angles_Res));
    ATH_CHECK (m_KFTool->setProperty("FixAnglesFit", m_isFixAngles));
    

    return StatusCode::SUCCESS;
  }

  StatusCode MbbKinFitDecoratorAlg::execute() {
    
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

        // Retrieve inputs
        const xAOD::EventInfo *event = nullptr;
        ANA_CHECK (m_eventHandle.retrieve (event, sys));
        
        const xAOD::PhotonContainer *photons = nullptr;
        ANA_CHECK (m_photonHandle.retrieve (photons, sys));

        const xAOD::JetContainer *jets = nullptr;
        ANA_CHECK (m_jetHandle.retrieve (jets, sys));

        //setup KF tool inputs
        double KF1_Mbb = -99.;
        if (jets->size()>=2) {
          TLorentzVector j1 = jets->at(0)->p4();
          TLorentzVector j2 = jets->at(1)->p4();
          TLorentzVector jj = j1 + j2;
          KF1_Mbb = jj.M();
        }
        //run KF tool
        ATH_CHECK(m_KFTool->applyKF(*photons, *jets, KF1_Mbb));
        //retrieve decorations
        static const SG::AuxElement::ConstAccessor<float> pTDecor("KF_PT");
        static const SG::AuxElement::ConstAccessor<float> etaDecor("KF_ETA");
        static const SG::AuxElement::ConstAccessor<float> phiDecor("KF_PHI");
        static const SG::AuxElement::ConstAccessor<float> mDecor("KF_M");
        static const SG::AuxElement::ConstAccessor<char> isBDecor("KF_isB");

        static SG::AuxElement::Decorator<float> KF_MBB("KF1_Mbb");

        auto workJetContainer =
        std::make_unique<ConstDataVector<xAOD::JetContainer> >(); 
        for (auto jet : *jets) {
          auto thisJet = std::make_unique<xAOD::Jet>(*jet);
          xAOD::JetFourMom_t newp4 (pTDecor(*jet), etaDecor(*jet), phiDecor(*jet), mDecor(*jet));
          thisJet->setJetP4(newp4);
          workJetContainer->push_back(thisJet.release());
        }

        KF_MBB(*event) = KF1_Mbb;
        //write to eventstore
        ATH_CHECK(m_jetOutHandle.record(std::move(workJetContainer), sys));

      }
      return StatusCode::SUCCESS;
    }
}