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
    ATH_CHECK (m_eventInfoKey.initialize());
    ATH_CHECK (m_runNumberKey.initialize());
    ATH_CHECK (m_rdmRunNumberKey.initialize());

    ATH_CHECK (m_tausInKey.initialize());

    m_nProngDecorKey = m_tausInKey.key() + "." + m_nProngDecorName;
    m_truthTypeDecorKey = m_tausInKey.key() + "." + m_truthTypeDecorName;
    m_IDTauDecorKey = m_tausInKey.key() + "." + m_IDTauDecorName;

    ATH_CHECK (m_nProngDecorKey.initialize());
    ATH_CHECK (m_truthTypeDecorKey.initialize(m_isMC));
    ATH_CHECK (m_IDTauDecorKey.initialize());


    ATH_CHECK (m_muonsInKey.initialize(m_doAntiTauDecor));
    ATH_CHECK (m_elesInKey.initialize(m_doAntiTauDecor));

    m_muonIdDecorKey = m_muonsInKey.key() + "." + m_muonIdDecorName;
    m_muonPreselDecorKey = m_muonsInKey.key() + "." + m_muonPreselDecorName;
    m_eleIdDecorKey = m_elesInKey.key() + "." + m_eleIdDecorName;
    ATH_CHECK (m_muonIdDecorKey.initialize(m_doAntiTauDecor));
    ATH_CHECK (m_muonPreselDecorKey.initialize(m_doAntiTauDecor));
    ATH_CHECK (m_eleIdDecorKey.initialize(m_doAntiTauDecor));
    m_antiTauDecorKey = m_tausInKey.key() + "." + m_antiTauDecorName;
    ATH_CHECK (m_antiTauDecorKey.initialize(m_doAntiTauDecor));
    
    m_triggerMatchSTTKey = m_tausInKey.key() + "." + m_triggerMatchSTTDecorName;
    m_triggerMatchLTTKey = m_tausInKey.key() + "." + m_triggerMatchLTTDecorName;
    m_triggerMatchDTTKey = m_tausInKey.key() + "." + m_triggerMatchDTTDecorName;
    ATH_CHECK (m_triggerMatchSTTKey.initialize(m_doAntiTauDecor));
    ATH_CHECK (m_triggerMatchLTTKey.initialize(m_doAntiTauDecor));
    ATH_CHECK (m_triggerMatchDTTKey.initialize(m_doAntiTauDecor));

    m_passSLTDecorKey = "EventInfo.pass_trigger_SLT";
    m_passLTTDecorKey = "EventInfo.pass_trigger_LTT";
    m_passSTTDecorKey = "EventInfo.pass_trigger_STT";
    m_passDTTDecorKey = "EventInfo.pass_trigger_DTT";
    ATH_CHECK(m_passSLTDecorKey.initialize(m_doAntiTauDecor));
    ATH_CHECK(m_passLTTDecorKey.initialize(m_doAntiTauDecor));
    ATH_CHECK(m_passSTTDecorKey.initialize(m_doAntiTauDecor));
    ATH_CHECK(m_passDTTDecorKey.initialize(m_doAntiTauDecor));

    m_eventCategoryDecorKey = m_tausInKey.key() + "." +   m_eventCategoryDecorName;
    ATH_CHECK (m_eventCategoryDecorKey.initialize(m_doAntiTauDecor));
    

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
    // input handles
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
    ATH_CHECK (eventInfo.isValid());
    SG::ReadHandle<xAOD::TauJetContainer> tausIn(m_tausInKey,ctx);
    ATH_CHECK (tausIn.isValid());

    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> m_runNumberHandle(m_runNumberKey);
    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> m_rdmRunNumberHandle(m_rdmRunNumberKey);
    SG::WriteDecorHandle<xAOD::TauJetContainer, int> nProngDecorHandle(m_nProngDecorKey);
    SG::WriteDecorHandle<xAOD::TauJetContainer, char> idTauDecorHandle(m_IDTauDecorKey);


    if(m_isMC){
      SG::WriteDecorHandle<xAOD::TauJetContainer, int> truthTypeDecorHandle(m_truthTypeDecorKey);
      for(const xAOD::TauJet* tau : *tausIn) {
	      truthTypeDecorHandle(*tau) = int(TauAnalysisTools::getTruthParticleType(*tau));
      }
    }

    for(const xAOD::TauJet* tau : *tausIn) {
      nProngDecorHandle(*tau) = tau->nTracks();
      bool isTauID = tau->isTau(m_tauIDWP);
      idTauDecorHandle(*tau) = isTauID;
      }
    
    if(m_doAntiTauDecor){
      // lepton read handles
      SG::ReadHandle<xAOD::MuonContainer> muonsIn(m_muonsInKey,ctx);
      ATH_CHECK (muonsIn.isValid());
      SG::ReadHandle<xAOD::ElectronContainer> elesIn(m_elesInKey,ctx);
      ATH_CHECK (elesIn.isValid());

      SG::ReadDecorHandle<xAOD::MuonContainer, char> muonIdDecorHandle(m_muonIdDecorKey);
      SG::ReadDecorHandle<xAOD::MuonContainer, char> muonPreselDecorHandle(m_muonPreselDecorKey);
      SG::ReadDecorHandle<xAOD::ElectronContainer, char> eleIdDecorHandle(m_eleIdDecorKey);

      // trigger tau read handles
      SG::ReadDecorHandle<xAOD::TauJetContainer, bool> isSTTMatched(m_triggerMatchSTTKey);
      SG::ReadDecorHandle<xAOD::TauJetContainer, bool> isLTTMatched(m_triggerMatchLTTKey);
      SG::ReadDecorHandle<xAOD::TauJetContainer, bool> isDTTMatched(m_triggerMatchDTTKey);

      // tau write decorators
      SG::WriteDecorHandle<xAOD::TauJetContainer, int> eventCategoryDecorHandle(m_eventCategoryDecorKey);
      SG::WriteDecorHandle<xAOD::TauJetContainer, char> antiTauDecorHandle(m_antiTauDecorKey);

      // trigger event read handles
      SG::ReadDecorHandle<xAOD::EventInfo, bool> isSLT(m_passSLTDecorKey);
      SG::ReadDecorHandle<xAOD::EventInfo, bool> isLTT(m_passLTTDecorKey);
      SG::ReadDecorHandle<xAOD::EventInfo, bool> isSTT(m_passSTTDecorKey);
      SG::ReadDecorHandle<xAOD::EventInfo, bool> isDTT(m_passDTTDecorKey);

      std::unordered_map<Easyjet::TriggerChannel,       std::unordered_map<Easyjet::Var, float>> ptThresholds;
      unsigned int rdmNumber = m_isMC ? m_rdmRunNumberHandle(*eventInfo) : m_runNumberHandle(*eventInfo);
      int year = 0;
      setRunNumberQuantities(rdmNumber, year, ptThresholds);

      int nIDMatchedTauSTT = 0;
      bool passTauPtSTTThreshold = false;

      for(const xAOD::TauJet* tau : *tausIn) {
        bool isTauID = tau->isTau(m_tauIDWP);
        if(m_doAntiTauDecor && isSTTMatched(*tau)){
          if(isTauID) nIDMatchedTauSTT++;
          passTauPtSTTThreshold |= tau->pt() > ptThresholds[Easyjet::TriggerChannel::STT][Easyjet::Var::leadingtau];
        }
      }

      int nLeptons = 0;
      bool passLeptonPtSLTThreshold = false;
      
      for(const xAOD::Muon* muon : *muonsIn) {
	      if(muonIdDecorHandle(*muon) && muonPreselDecorHandle(*muon) &&
         muon->pt() > 7 * Athena::Units::GeV) {
          nLeptons++;
          passLeptonPtSLTThreshold |= muon->pt() > ptThresholds[Easyjet::TriggerChannel::SLT][Easyjet::Var::mu];
        }
      }
      for(const xAOD::Electron* ele : *elesIn) {
	      if(eleIdDecorHandle(*ele) && ele->pt() > 7 * Athena::Units::GeV){
          nLeptons++;
          passLeptonPtSLTThreshold |= ele->pt() > ptThresholds[Easyjet::TriggerChannel::SLT][Easyjet::Var::ele];
        }
      }

      bool STT = isSTT(*eventInfo) && passTauPtSTTThreshold && nLeptons==0;
      bool DTT = isDTT(*eventInfo) && !passTauPtSTTThreshold && nLeptons==0;
      bool LTT = isLTT(*eventInfo) && !passLeptonPtSLTThreshold && nLeptons>0;
      bool SLT = isSLT(*eventInfo) && passLeptonPtSLTThreshold && nLeptons>0;

      for(const xAOD::TauJet* tau : *tausIn) {
        bool isTauID = tau->isTau(m_tauIDWP);
        float RNNScore = tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
        bool isAntiTau = !isTauID && RNNScore>m_antiTauRNNThreshold;
    
        // for SLT no anti-tau trigger matching is required
        if (LTT) isAntiTau &= isLTTMatched(*tau);
        else if (DTT) isAntiTau &= isDTTMatched(*tau);
        else if (STT && nIDMatchedTauSTT == 0) isAntiTau &= isSTTMatched(*tau); // in STT if ID tau not trig matched anti tau needs be matched to trigger
        
        int antiTauCategory = 0;
        if (isAntiTau) {
          if( SLT || LTT ) antiTauCategory = 1;
          else if ( STT || DTT ) antiTauCategory = 2;
        }

        eventCategoryDecorHandle(*tau) = antiTauCategory;
        antiTauDecorHandle(*tau) = isAntiTau;
      }
    }
  
    return StatusCode::SUCCESS;
  }
  
  void TauDecoratorAlg::setRunNumberQuantities(unsigned int runNumber, int& year, std::unordered_map<Easyjet::TriggerChannel, std::unordered_map<Easyjet::Var, float>>& ptThresholds) const{

    // References:
    // https://atlas-tagservices.cern.ch/tagservices/RunBrowser/runBrowserReport/rBR_Period_Report.php
    // https://twiki.cern.ch/twiki/bin/view/Atlas/LowestUnprescaled

    year = 0;
    if(266904 <= runNumber && runNumber <= 284484) year = 2015;
    else if(296939 <= runNumber && runNumber <= 311481) year = 2016;
  
    // Single-lepton triggers
    if(year==2015)
      ptThresholds[Easyjet::TriggerChannel::SLT][Easyjet::Var::ele] = 25. * Athena::Units::GeV;
    // 2022 75 bunches
    else if(427882 <= runNumber && runNumber < 428071)
      ptThresholds[Easyjet::TriggerChannel::SLT][Easyjet::Var::ele] = 18. * Athena::Units::GeV;
    else
      ptThresholds[Easyjet::TriggerChannel::SLT][Easyjet::Var::ele] = 27. * Athena::Units::GeV;

    if(year==2015)
      ptThresholds[Easyjet::TriggerChannel::SLT][Easyjet::Var::mu] = 21. * Athena::Units::GeV;
    else if(year>=2016 && year<=2018)
      ptThresholds[Easyjet::TriggerChannel::SLT][Easyjet::Var::mu] = 27. * Athena::Units::GeV;
    else
      ptThresholds[Easyjet::TriggerChannel::SLT][Easyjet::Var::mu] = 25. * Athena::Units::GeV;

    // Single tau triggers
    float min_tau_STT = 180. * Athena::Units::GeV;
    // 2015 + 2016 period A
    if(year==2015 || (296939 <= runNumber && runNumber <= 300287))
      min_tau_STT = 100. * Athena::Units::GeV;
    // 2016 period B-D3
    else if(300345 <= runNumber && runNumber <= 302872)
      min_tau_STT = 140. * Athena::Units::GeV;

    ptThresholds[Easyjet::TriggerChannel::STT][Easyjet::Var::leadingtau] = min_tau_STT;
  }


}

