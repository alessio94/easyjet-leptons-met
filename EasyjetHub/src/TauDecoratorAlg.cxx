/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam


#include "TauDecoratorAlg.h"
#include <AsgDataHandles/WriteDecorHandle.h>

namespace Easyjet
{
  TauDecoratorAlg ::TauDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
    declareProperty("tauIDWP", m_tauIDWP);
  }

  StatusCode TauDecoratorAlg ::initialize()
  {
    ATH_CHECK (m_tausInKey.initialize());

    m_nProngDecorKey = m_tausInKey.key() + "." + m_nProngDecorName;
    m_IDTauDecorKey = m_tausInKey.key() + "." + m_IDTauDecorName;
    m_antiTauDecorKey = m_tausInKey.key() + "." + m_antiTauDecorName;

    ATH_CHECK (m_nProngDecorKey.initialize());
    ATH_CHECK (m_IDTauDecorKey.initialize());
    ATH_CHECK (m_antiTauDecorKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TauDecoratorAlg ::execute(const EventContext& ctx) const
  {

    SG::ReadHandle<xAOD::TauJetContainer> tausIn(m_tausInKey,ctx);
    ATH_CHECK (tausIn.isValid());

    SG::WriteDecorHandle<xAOD::TauJetContainer, int> nProngDecorHandle(m_nProngDecorKey);
    SG::WriteDecorHandle<xAOD::TauJetContainer, char> idTauDecorHandle(m_IDTauDecorKey);
    SG::WriteDecorHandle<xAOD::TauJetContainer, char> antiTauDecorHandle(m_antiTauDecorKey);

    for(const xAOD::TauJet* tau : *tausIn) {

      nProngDecorHandle(*tau) = tau->nTracks();

      auto tauIDWP= xAOD::TauJetParameters::JetRNNSigLoose;
      if (m_tauIDWP == "Medium") tauIDWP = xAOD::TauJetParameters::JetRNNSigMedium;
      else if (m_tauIDWP == "Tight") tauIDWP = xAOD::TauJetParameters::JetRNNSigTight;
      else if (m_tauIDWP != "Loose") {
          ATH_MSG_ERROR("Unknown Tau ID WP ");
          return StatusCode::FAILURE;
      }
      
      bool isTauID = tau->isTau(tauIDWP);
      idTauDecorHandle(*tau) = isTauID;

      float RNNScore = tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
      bool isAntiTau = !isTauID && RNNScore>0.01;
      antiTauDecorHandle(*tau) = isAntiTau;

    }

    return StatusCode::SUCCESS;
  }
}
