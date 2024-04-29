/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "AntiTauDecoratorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include "TauAnalysisTools/HelperFunctions.h"

#include <AthenaKernel/Units.h>


namespace HHBBTT
{
  AntiTauDecoratorAlg::AntiTauDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode AntiTauDecoratorAlg::initialize()
  {
    ATH_CHECK (m_eventInfoKey.initialize());

    m_yearKey = "EventInfo.dataTakingYear";
    m_is2016_periodA_key = "EventInfo.is2016_periodA";
    m_is2016_periodB_D3_key = "EventInfo.is2016_periodB_D3";
    m_is2022_75bunches_key = "EventInfo.is2022_75bunches";
    ATH_CHECK(m_yearKey.initialize());
    ATH_CHECK(m_is2016_periodA_key.initialize());
    ATH_CHECK(m_is2016_periodB_D3_key.initialize());
    ATH_CHECK(m_is2022_75bunches_key.initialize());

    m_passSLTDecorKey = "EventInfo.pass_trigger_SLT";
    m_passLTTDecorKey = "EventInfo.pass_trigger_LTT";
    m_passSTTDecorKey = "EventInfo.pass_trigger_STT";
    m_passDTTDecorKey = "EventInfo.pass_trigger_DTT";
    ATH_CHECK(m_passSLTDecorKey.initialize());
    ATH_CHECK(m_passLTTDecorKey.initialize());
    ATH_CHECK(m_passSTTDecorKey.initialize());
    ATH_CHECK(m_passDTTDecorKey.initialize());

    ATH_CHECK (m_tausInKey.initialize());

    if(m_tauIDWP_name=="Loose") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigLoose;
    else if(m_tauIDWP_name=="Medium") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigMedium;
    else if(m_tauIDWP_name=="Tight") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigTight;
    else{
      ATH_MSG_ERROR("Unknown Tau ID WP ");
      return StatusCode::FAILURE;
    }

    m_triggerMatchSTTKey = m_tausInKey.key() + ".trigMatch_STT";
    m_triggerMatchLTTKey = m_tausInKey.key() + ".trigMatch_LTT";
    m_triggerMatchDTTKey = m_tausInKey.key() + ".trigMatch_DTT";
    ATH_CHECK (m_triggerMatchSTTKey.initialize());
    ATH_CHECK (m_triggerMatchLTTKey.initialize());
    ATH_CHECK (m_triggerMatchDTTKey.initialize());

    m_IDTauDecorKey = m_tausInKey.key() + ".isIDTau";
    m_antiTauDecorKey = m_tausInKey.key() + ".isAntiTau";
    m_eventCategoryDecorKey = m_tausInKey.key() + ".antiTauEventCategory";
    ATH_CHECK (m_IDTauDecorKey.initialize());
    ATH_CHECK (m_antiTauDecorKey.initialize());
    ATH_CHECK (m_eventCategoryDecorKey.initialize());

    ATH_CHECK (m_muonsInKey.initialize());
    ATH_CHECK (m_elesInKey.initialize());

    m_muonIdDecorKey = m_muonsInKey.key() + "." + m_muonIdDecorName;
    m_muonPreselDecorKey = m_muonsInKey.key() + "." + m_muonPreselDecorName;
    m_eleIdDecorKey = m_elesInKey.key() + "." + m_eleIdDecorName;
    ATH_CHECK (m_muonIdDecorKey.initialize());
    ATH_CHECK (m_muonPreselDecorKey.initialize());
    ATH_CHECK (m_eleIdDecorKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode AntiTauDecoratorAlg ::execute(const EventContext& ctx) const
  {
    // Event info handles
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
    ATH_CHECK (eventInfo.isValid());

    SG::ReadDecorHandle<xAOD::EventInfo, bool> isSLT(m_passSLTDecorKey);
    SG::ReadDecorHandle<xAOD::EventInfo, bool> isLTT(m_passLTTDecorKey);
    SG::ReadDecorHandle<xAOD::EventInfo, bool> isSTT(m_passSTTDecorKey);
    SG::ReadDecorHandle<xAOD::EventInfo, bool> isDTT(m_passDTTDecorKey);

    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> year(m_yearKey);
    SG::ReadDecorHandle<xAOD::EventInfo, bool> is2016_periodA(m_is2016_periodA_key);
    SG::ReadDecorHandle<xAOD::EventInfo, bool> is2016_periodB_D3(m_is2016_periodB_D3_key);
    SG::ReadDecorHandle<xAOD::EventInfo, bool> is2022_75bunches(m_is2022_75bunches_key);

    // Tau handles
    SG::ReadHandle<xAOD::TauJetContainer> tausIn(m_tausInKey,ctx);
    ATH_CHECK (tausIn.isValid());

    SG::ReadDecorHandle<xAOD::TauJetContainer, bool> isSTTMatched(m_triggerMatchSTTKey);
    SG::ReadDecorHandle<xAOD::TauJetContainer, bool> isLTTMatched(m_triggerMatchLTTKey);
    SG::ReadDecorHandle<xAOD::TauJetContainer, bool> isDTTMatched(m_triggerMatchDTTKey);

    SG::WriteDecorHandle<xAOD::TauJetContainer, char> idTauDecorHandle(m_IDTauDecorKey);
    SG::WriteDecorHandle<xAOD::TauJetContainer, char> antiTauDecorHandle(m_antiTauDecorKey);
    SG::WriteDecorHandle<xAOD::TauJetContainer, int> eventCategoryDecorHandle(m_eventCategoryDecorKey);

    // lepton handles
    SG::ReadHandle<xAOD::MuonContainer> muonsIn(m_muonsInKey,ctx);
    ATH_CHECK (muonsIn.isValid());
    SG::ReadHandle<xAOD::ElectronContainer> elesIn(m_elesInKey,ctx);
    ATH_CHECK (elesIn.isValid());

    SG::ReadDecorHandle<xAOD::MuonContainer, char> muonIdDecorHandle(m_muonIdDecorKey);
    SG::ReadDecorHandle<xAOD::MuonContainer, char> muonPreselDecorHandle(m_muonPreselDecorKey);
    SG::ReadDecorHandle<xAOD::ElectronContainer, char> eleIdDecorHandle(m_eleIdDecorKey);


    std::unordered_map<HHBBTT::TriggerChannel,
		       std::unordered_map<HHBBTT::Var, float>> ptThresholds;
    setThresholds(year(*eventInfo),
		  is2016_periodA(*eventInfo),
		  is2016_periodB_D3(*eventInfo),
		  is2022_75bunches(*eventInfo),
		  ptThresholds);

    int nIDMatchedTauSTT = 0;
    bool passTauPtSTTThreshold = false;

    for(const xAOD::TauJet* tau : *tausIn) {
      bool isTauID = tau->isTau(m_tauIDWP);
      if(isSTTMatched(*tau)){
	if(isTauID) nIDMatchedTauSTT++;
	passTauPtSTTThreshold |=
	  tau->pt() > ptThresholds[HHBBTT::TriggerChannel::STT][HHBBTT::Var::leadingtau];
      }
    }

    int nLeptons = 0;
    bool passLeptonPtSLTThreshold = false;
      
    for(const xAOD::Muon* muon : *muonsIn) {
      if(muonIdDecorHandle(*muon) && muonPreselDecorHandle(*muon) &&
	 muon->pt() > 7 * Athena::Units::GeV) {
	nLeptons++;
	passLeptonPtSLTThreshold |=
	  muon->pt() > ptThresholds[HHBBTT::TriggerChannel::SLT][HHBBTT::Var::mu];
      }
    }

    for(const xAOD::Electron* ele : *elesIn) {
      if(eleIdDecorHandle(*ele) && ele->pt() > 7 * Athena::Units::GeV){
	nLeptons++;
	passLeptonPtSLTThreshold |=
	  ele->pt() > ptThresholds[HHBBTT::TriggerChannel::SLT][HHBBTT::Var::ele];
      }
    }

    bool STT = isSTT(*eventInfo) && passTauPtSTTThreshold && nLeptons==0;
    bool DTT = isDTT(*eventInfo) && !passTauPtSTTThreshold && nLeptons==0;
    bool SLT = isSLT(*eventInfo) && passLeptonPtSLTThreshold && nLeptons>0;
    bool LTT = isLTT(*eventInfo) && !passLeptonPtSLTThreshold && nLeptons>0;

    for(const xAOD::TauJet* tau : *tausIn) {
      bool isTauID = tau->isTau(m_tauIDWP);
      idTauDecorHandle(*tau) = isTauID;
      float RNNScore = tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
      int decayMode = -1;
      tau->panTauDetail(xAOD::TauJetParameters::PanTau_DecayMode, decayMode);
      bool isAntiTau = !isTauID && RNNScore>m_antiTauRNNThreshold && decayMode!=xAOD::TauJetParameters::Mode_NotSet;
    
      // for SLT no anti-tau trigger matching is required
      if (LTT) isAntiTau &= isLTTMatched(*tau);
      else if (DTT) isAntiTau &= isDTTMatched(*tau);
      else if (STT && nIDMatchedTauSTT == 0) isAntiTau &= isSTTMatched(*tau); // in STT if ID tau not trig matched anti tau needs be matched to trigger
      antiTauDecorHandle(*tau) = isAntiTau;
        
      int antiTauCategory = 0;
      if (isAntiTau) {
	if(SLT || LTT) antiTauCategory = 1;
	else if (STT || DTT) antiTauCategory = 2;
      }
      eventCategoryDecorHandle(*tau) = antiTauCategory;
    }
  
    return StatusCode::SUCCESS;
  }
  
  void AntiTauDecoratorAlg::setThresholds
  (unsigned int year,
   bool is2016_periodA, bool is2016_periodB_D3,
   bool is2022_75bunches,
   std::unordered_map<HHBBTT::TriggerChannel, std::unordered_map<HHBBTT::Var, float>>& ptThresholds) const{

    // References:
    // https://twiki.cern.ch/twiki/bin/view/Atlas/LowestUnprescaled
  
    // Single-lepton triggers
    float min_ele = 27. * Athena::Units::GeV;
    if(year==2015) min_ele = 25. * Athena::Units::GeV;
    else if(is2022_75bunches) min_ele = 18. * Athena::Units::GeV;
    ptThresholds[HHBBTT::TriggerChannel::SLT][HHBBTT::Var::ele] = min_ele;

    float min_mu = 25. * Athena::Units::GeV;
    if(year==2015) min_mu = 21. * Athena::Units::GeV;
    else if(year>=2016 && year<=2018) min_mu = 27. * Athena::Units::GeV;
    ptThresholds[HHBBTT::TriggerChannel::SLT][HHBBTT::Var::mu] = min_mu;

    // Single tau triggers
    float min_tau_STT = 180. * Athena::Units::GeV;
    if(year==2015 || is2016_periodA) min_tau_STT = 100. * Athena::Units::GeV;
    else if(is2016_periodB_D3) min_tau_STT = 140. * Athena::Units::GeV;
    ptThresholds[HHBBTT::TriggerChannel::STT][HHBBTT::Var::leadingtau] = min_tau_STT;
  }

}

