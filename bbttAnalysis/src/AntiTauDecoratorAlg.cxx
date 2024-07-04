/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AntiTauDecoratorAlg.h"

#include <AthenaKernel/Units.h>


namespace HHBBTT
{
  AntiTauDecoratorAlg::AntiTauDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator) { }

  StatusCode AntiTauDecoratorAlg::initialize()
  {
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_is2016_periodA.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is2016_periodB_D3.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is2022_75bunches.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_passSLT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_passLTT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_passSTT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_passDTT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_passDBT.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK(m_tauBaselineSelection.initialize(m_systematicsList, m_tauHandle));

    if(m_tauIDWP_name=="Loose") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigLoose;
    else if(m_tauIDWP_name=="Medium") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigMedium;
    else if(m_tauIDWP_name=="Tight") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigTight;
    else{
      ATH_MSG_ERROR("Unknown Tau ID WP ");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_triggerMatchSTT.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK(m_triggerMatchLTT.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK(m_triggerMatchDTT.initialize(m_systematicsList, m_tauHandle));

    ATH_CHECK(m_IDTau.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK(m_antiTau.initialize(m_systematicsList, m_tauHandle));

    ATH_CHECK(m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonHandle));

    ATH_CHECK(m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronHandle));

    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode AntiTauDecoratorAlg ::execute()
  {

    const static SG::AuxElement::Decorator<int> categoryDecorator("antiTauEventCategory");
    
    for (const auto& sys : m_systematicsList.systematicsVector()){

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK(m_eventHandle.retrieve (event, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK(m_tauHandle.retrieve (taus, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK(m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK(m_electronHandle.retrieve (electrons, sys));

      std::unordered_map<HHBBTT::TriggerChannel,
			 std::unordered_map<HHBBTT::Var, float>> ptThresholds;
      setThresholds(m_year.get(*event, sys),
		    m_is2016_periodA.get(*event, sys),
		    m_is2016_periodB_D3.get(*event, sys),
		    m_is2022_75bunches.get(*event, sys),
		    ptThresholds);

      int nIDMatchedTauSTT = 0;
      bool passTauPtSTTThreshold = false;

      for(const xAOD::TauJet* tau : *taus) {
        if(!m_tauBaselineSelection.getBool(*tau, sys)) continue;
        bool isTauID = tau->isTau(m_tauIDWP);
        if(m_triggerMatchSTT.get(*tau, sys)){
          if(isTauID) nIDMatchedTauSTT++;
          passTauPtSTTThreshold |=
            tau->pt() > ptThresholds[HHBBTT::TriggerChannel::STT][HHBBTT::Var::leadingtau];
        }
      }

      int nLeptons = 0;
      bool passLeptonPtSLTThreshold = false;

      for(const xAOD::Muon* muon : *muons) {
        if(m_muonSelection.getBool(*muon, sys) &&
           muon->pt() > 7 * Athena::Units::GeV) {
          nLeptons++;
          passLeptonPtSLTThreshold |=
	    muon->pt() > ptThresholds[HHBBTT::TriggerChannel::SLT][HHBBTT::Var::mu];
        }
      }

      for(const xAOD::Electron* ele : *electrons) {
        if(m_electronSelection.getBool(*ele, sys) && ele->pt() > 7 * Athena::Units::GeV){
          nLeptons++;
          passLeptonPtSLTThreshold |=
            ele->pt() > ptThresholds[HHBBTT::TriggerChannel::SLT][HHBBTT::Var::ele];
        }
      }

      // TODO DBT implementation not final. Like this we will have many events for which
      // anti-taus are selected according to DTT criteria in the offline DBT category
      bool STT = m_passSTT.get(*event, sys) && passTauPtSTTThreshold && nLeptons==0;
      bool DTT = m_passDTT.get(*event, sys) && !passTauPtSTTThreshold && nLeptons==0;
      bool DBT = m_passDBT.get(*event, sys) && !passTauPtSTTThreshold && nLeptons==0; // not orthogonal to DTT for now
      bool SLT = m_passSLT.get(*event, sys) && passLeptonPtSLTThreshold && nLeptons>0;
      bool LTT = m_passLTT.get(*event, sys) && !passLeptonPtSLTThreshold && nLeptons>0;

      for(const xAOD::TauJet* tau : *taus) {
        bool isTauID = false;
        bool isAntiTau = false;
        int antiTauCategory = 0;

        if(m_tauBaselineSelection.getBool(*tau, sys)){
	  isTauID = tau->isTau(m_tauIDWP);
          float RNNScore = tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
          int decayMode = -1;
          tau->panTauDetail(xAOD::TauJetParameters::PanTau_DecayMode, decayMode);
          isAntiTau = (!isTauID && RNNScore>m_antiTauRNNThreshold &&
		       decayMode!=xAOD::TauJetParameters::Mode_NotSet);
    
          // for SLT and DBT no anti-tau trigger matching is required
          if (LTT) isAntiTau &= m_triggerMatchLTT.get(*tau, sys);
          else if (DTT) isAntiTau &= m_triggerMatchDTT.get(*tau, sys);
          // in STT if ID tau not trig matched anti tau needs be matched to trigger
          else if (STT && nIDMatchedTauSTT == 0) isAntiTau &= m_triggerMatchSTT.get(*tau, sys);

          if (isAntiTau) {
            if(SLT || LTT) antiTauCategory = 1;
            else if (STT || DTT || DBT) antiTauCategory = 2;
          }
        }

        m_IDTau.setBool(*tau, isTauID, sys);
        m_antiTau.setBool(*tau, isAntiTau, sys);

        // Using a decorator might be ill-defined with systematics
        // but anti-tau usage is mostly for data-driven background
        // so let's neglect the subtlety involved
        categoryDecorator(*tau) = antiTauCategory;

      }
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

