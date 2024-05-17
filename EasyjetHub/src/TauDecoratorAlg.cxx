/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam


#include "TauDecoratorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include "TauAnalysisTools/HelperFunctions.h"

#include <AthenaKernel/Units.h>


namespace Easyjet
{
  TauDecoratorAlg ::TauDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode TauDecoratorAlg ::initialize()
  {
    ATH_CHECK (m_tausInKey.initialize());

    m_nProngDecorKey = m_tausInKey.key() + "." + m_nProngDecorName;
    m_decayModeDecorKey = m_tausInKey.key() + "." + m_decayModeDecorName;
    m_truthTypeDecorKey = m_tausInKey.key() + "." + m_truthTypeDecorName;

    ATH_CHECK (m_nProngDecorKey.initialize());
    ATH_CHECK (m_decayModeDecorKey.initialize());
    ATH_CHECK (m_truthTypeDecorKey.initialize(m_isMC));

    ATH_CHECK(m_truthJetsInKey.initialize(m_isMC));
    m_truthLabelDecorKey = m_truthJetsInKey.key() + "." + m_truthLabelDecorName;
    ATH_CHECK(m_truthLabelDecorKey.initialize(m_isMC));

    m_tauTruthJetLabelDecorKey = m_tausInKey.key() + "." + m_tauTruthJetLabelDecorName;
    ATH_CHECK (m_tauTruthJetLabelDecorKey.initialize(m_isMC));

    m_tauLinkedJetDecorKey = m_tausInKey.key() + "." + m_tauLinkedJetDecorName;
    ATH_CHECK (m_tauLinkedJetDecorKey.initialize(m_isMC));

    return StatusCode::SUCCESS;
  }

  StatusCode TauDecoratorAlg ::execute(const EventContext& ctx) const
  {
    // input handles
    SG::ReadHandle<xAOD::TauJetContainer> tausIn(m_tausInKey,ctx);
    ATH_CHECK (tausIn.isValid());

    SG::WriteDecorHandle<xAOD::TauJetContainer, int> nProngDecorHandle(m_nProngDecorKey);
    SG::WriteDecorHandle<xAOD::TauJetContainer, int> decayModeDecorHandle(m_decayModeDecorKey);

    for(const xAOD::TauJet* tau : *tausIn) {
      nProngDecorHandle(*tau) = tau->nTracks();
      int decayMode = -1;
      tau->panTauDetail(xAOD::TauJetParameters::PanTau_DecayMode, decayMode);
      decayModeDecorHandle(*tau) = decayMode;
    }

    if(m_isMC){
      SG::WriteDecorHandle<xAOD::TauJetContainer, int> truthTypeDecorHandle(m_truthTypeDecorKey);

      SG::ReadDecorHandle<xAOD::JetContainer, int> truthLabelLinkedJet(m_truthLabelDecorKey);
      SG::WriteDecorHandle<xAOD::TauJetContainer, int> tauTruthJetLabelDecorHandle(m_tauTruthJetLabelDecorKey);
    
      typedef ElementLink< xAOD::JetContainer > Link_t; 
      SG::ReadDecorHandle<xAOD::TauJetContainer, Link_t> tauLinkedJetLink(m_tauLinkedJetDecorKey);

      for(const xAOD::TauJet* tau : *tausIn) {
        truthTypeDecorHandle(*tau) = int(TauAnalysisTools::getTruthParticleType(*tau));

        const Link_t tauTruthJetLink = tauLinkedJetLink(*tau);
        const xAOD::Jet* tauTruthJet = tauTruthJetLink.isValid() ? tauTruthJetLink.cachedElement() : nullptr;
        
        tauTruthJetLabelDecorHandle(*tau) = tauTruthJet ? truthLabelLinkedJet(*tauTruthJet) : -99;
      }
    }
  
    return StatusCode::SUCCESS;
  }

}
