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
    declareProperty("tauIDWP", m_tauIDWP_name);
    declareProperty("channel", m_channel_name);

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

    if(m_tauIDWP_name=="Loose") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigLoose;
    else if(m_tauIDWP_name=="Medium") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigMedium;
    else if(m_tauIDWP_name=="Tight") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigTight;
    else{
      ATH_MSG_ERROR("Unknown Tau ID WP ");
      return StatusCode::FAILURE;
    }

    if(m_channel_name == "lephad") m_channel = bbtautau::LepHad;
    else if(m_channel_name == "hadhad") m_channel = bbtautau::HadHad;
    else{
      ATH_MSG_ERROR("Unknown channel");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode TauDecoratorAlg ::execute(const EventContext& ctx) const
  {

    SG::ReadHandle<xAOD::TauJetContainer> tausIn(m_tausInKey,ctx);
    ATH_CHECK (tausIn.isValid());

    SG::WriteDecorHandle<xAOD::TauJetContainer, int> nProngDecorHandle(m_nProngDecorKey);
    SG::WriteDecorHandle<xAOD::TauJetContainer, char> idTauDecorHandle(m_IDTauDecorKey);
    SG::WriteDecorHandle<xAOD::TauJetContainer, char> antiTauDecorHandle(m_antiTauDecorKey);
    
    int nidtau = 0;
    for(const xAOD::TauJet* tau : *tausIn) {

      nProngDecorHandle(*tau) = tau->nTracks();

      bool isTauID = tau->isTau(m_tauIDWP);
      if(isTauID) nidtau++; 
      idTauDecorHandle(*tau) = isTauID;

    }

    int nantitau = 0;
    int nantitau_max = -1;
    if(m_channel==bbtautau::LepHad) nantitau_max = 1 - nidtau;
    else if(m_channel==bbtautau::HadHad) nantitau_max = 2 - nidtau;

    for(const xAOD::TauJet* tau : *tausIn) {
      bool isAntiTau = false;
      if(nantitau < nantitau_max){
        bool isTauID = idTauDecorHandle(*tau);
        float RNNScore = tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
        isAntiTau = !isTauID && RNNScore>0.01;
      }
      if (isAntiTau) nantitau++;
      antiTauDecorHandle(*tau) = isAntiTau;
    }
    return StatusCode::SUCCESS;
  }
}

