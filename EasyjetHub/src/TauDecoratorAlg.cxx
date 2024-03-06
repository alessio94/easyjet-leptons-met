/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam


#include "TauDecoratorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include "TauAnalysisTools/HelperFunctions.h"

namespace Easyjet
{
  TauDecoratorAlg ::TauDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode TauDecoratorAlg ::initialize()
  {
    ATH_CHECK (m_tausInKey.initialize());

    m_nProngDecorKey = m_tausInKey.key() + "." + m_nProngDecorName;
    m_truthTypeDecorKey = m_tausInKey.key() + "." + m_truthTypeDecorName;
    m_IDTauDecorKey = m_tausInKey.key() + "." + m_IDTauDecorName;

    ATH_CHECK (m_nProngDecorKey.initialize());
    ATH_CHECK (m_truthTypeDecorKey.initialize(m_isMC));
    ATH_CHECK (m_IDTauDecorKey.initialize());

    // Muons + ele stuff to be cleaned up after trigger matching is used
    ATH_CHECK (m_muonsInKey.initialize());
    ATH_CHECK (m_elesInKey.initialize());

    m_muonIdDecorKey = m_muonsInKey.key() + "." + m_muonIdDecorName;
    m_muonPreselDecorKey = m_muonsInKey.key() + "." + m_muonPreselDecorName;
    m_eleIdDecorKey = m_elesInKey.key() + "." + m_eleIdDecorName;

    ATH_CHECK (m_muonIdDecorKey.initialize());
    ATH_CHECK (m_muonPreselDecorKey.initialize());
    ATH_CHECK (m_eleIdDecorKey.initialize());

    m_antiTauDecorKey = m_tausInKey.key() + "." + m_antiTauDecorName;
    ATH_CHECK (m_antiTauDecorKey.initialize());

    if(m_tauIDWP_name=="Loose") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigLoose;
    else if(m_tauIDWP_name=="Medium") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigMedium;
    else if(m_tauIDWP_name=="Tight") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigTight;
    else{
      ATH_MSG_ERROR("Unknown Tau ID WP ");
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

    SG::ReadHandle<xAOD::MuonContainer> muonsIn(m_muonsInKey,ctx);
    SG::ReadHandle<xAOD::ElectronContainer> elesIn(m_elesInKey,ctx);
    ATH_CHECK (muonsIn.isValid());
    ATH_CHECK (elesIn.isValid());

    SG::ReadDecorHandle<xAOD::MuonContainer, char> muonIdDecorHandle(m_muonIdDecorKey);
    SG::ReadDecorHandle<xAOD::MuonContainer, char> muonPreselDecorHandle(m_muonPreselDecorKey);
    SG::ReadDecorHandle<xAOD::ElectronContainer, char> eleIdDecorHandle(m_eleIdDecorKey);

    SG::WriteDecorHandle<xAOD::TauJetContainer, char> antiTauDecorHandle(m_antiTauDecorKey);
    
    int nidtau = 0;
    for(const xAOD::TauJet* tau : *tausIn) {

      nProngDecorHandle(*tau) = tau->nTracks();
      bool isTauID = tau->isTau(m_tauIDWP);
      if(isTauID) nidtau++; 
      idTauDecorHandle(*tau) = isTauID;

    }

    if(m_isMC){
      SG::WriteDecorHandle<xAOD::TauJetContainer, int> truthTypeDecorHandle(m_truthTypeDecorKey);
      for(const xAOD::TauJet* tau : *tausIn) {
	truthTypeDecorHandle(*tau) = int(TauAnalysisTools::getTruthParticleType(*tau));
      }
    }

    if(m_doAntiTauDecor){
      int nlepton = 0;
      for(const xAOD::Muon* muon : *muonsIn) {
	if(muonIdDecorHandle(*muon) && muonPreselDecorHandle(*muon)) nlepton++;
      }
      for(const xAOD::Electron* ele : *elesIn) {
	if(eleIdDecorHandle(*ele)) nlepton++;
      }

      int nantitau = 0;
      int nantitau_max = -1;
      if(nlepton>0) nantitau_max = 1 - nidtau;
      else nantitau_max = 2 - nidtau;

      for(const xAOD::TauJet* tau : *tausIn) {
	bool isAntiTau = false;

	// Trigger matching to be added here
	if(nantitau < nantitau_max){
	  bool isTauID = idTauDecorHandle(*tau);
	  float RNNScore = tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
	  isAntiTau = !isTauID && RNNScore>0.01;
	}
	if (isAntiTau) nantitau++;

	antiTauDecorHandle(*tau) = isAntiTau;
      }
    }

    return StatusCode::SUCCESS;
  }
}

