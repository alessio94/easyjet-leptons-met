/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam


#include "TauDecoratorAlg.h"

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

    return StatusCode::SUCCESS;
  }

  StatusCode TauDecoratorAlg ::execute(const EventContext& ctx) const
  {

    SG::ReadHandle<xAOD::TauJetContainer> tausIn(m_tausInKey,ctx);
    ATH_CHECK (tausIn.isValid());
    SG::AuxElement::Decorator<float> dec("nProng");
    SG::AuxElement::Decorator<char> Taudec("isIDTau");
    SG::AuxElement::Decorator<char> antiTaudec("isAntiTau");

    for(const xAOD::TauJet* tau : *tausIn) {

      dec(*tau) = tau->nTracks();

      auto tauIDWP= xAOD::TauJetParameters::JetRNNSigLoose;
      if (m_tauIDWP == "Medium") tauIDWP = xAOD::TauJetParameters::JetRNNSigMedium;
      else if (m_tauIDWP == "Tight") tauIDWP = xAOD::TauJetParameters::JetRNNSigTight;
      else if (m_tauIDWP != "Loose") {
          ATH_MSG_ERROR("Unknown Tau ID WP ");
          return StatusCode::FAILURE;
      }
      
      bool isTauID = tau->isTau(tauIDWP);
      Taudec(*tau) = isTauID;

      float RNNScore = tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
      bool isAntiTau = !isTauID && RNNScore>0.01;
      antiTaudec(*tau) = isAntiTau;

    }

    return StatusCode::SUCCESS;
  }
}
