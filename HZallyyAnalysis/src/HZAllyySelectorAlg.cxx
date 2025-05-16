/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HZAllyySelectorAlg.h"
#include <AthenaKernel/Units.h>
#include <AthContainers/ConstDataVector.h>
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace HZALLYY
{
  
  HZAllyySelectorAlg::HZAllyySelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }
  
  StatusCode HZAllyySelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     HZAllyySelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");
    
    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    
    if (m_saveCutFlow) ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));
    m_photonLIWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_photon_LooseID_Iso_WPName+"_%SYS%", this);
    ATH_CHECK(m_photonLIWPDecorHandle.initialize(m_systematicsList, m_photonHandle));
    
    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is17_periodB5_B8.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is22_75bunches.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is23_75bunches.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is23_400bunches.initialize(m_systematicsList, m_eventHandle));
    
    ATH_CHECK (m_matchingTool.retrieve());
    
    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    }
    
    // make trigger decorators
    for (auto trig : m_triggers){
      CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
      m_triggerdecos.emplace(trig, deco);
      ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    }
          
    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));
    
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    if(m_saveCutFlow) ATH_CHECK (initialiseCutflow());
    return StatusCode::SUCCESS;
  }
  
  
  StatusCode HZAllyySelectorAlg::execute()
  {
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);
      
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
      {
	CP::SysFilterReporter filter (filterCombiner, sys);
	
	// Retrive inputs
	const xAOD::EventInfo *event = nullptr;
	ANA_CHECK (m_eventHandle.retrieve (event, sys));
	
	const xAOD::MuonContainer *muons = nullptr;
	ANA_CHECK (m_muonHandle.retrieve (muons, sys));
	
	const xAOD::ElectronContainer *electrons = nullptr;
	ANA_CHECK (m_electronHandle.retrieve (electrons, sys));
	
	const xAOD::PhotonContainer *photons = nullptr;
	ANA_CHECK (m_photonHandle.retrieve (photons, sys));    
	
	m_bools.at(HZALLYY::pass_trigger_SLT) = false;
	m_bools.at(HZALLYY::pass_trigger_DLT) = false;
	m_bools.at(HZALLYY::PASS_TRIGGER) = false;
	m_bools.at(HZALLYY::EXACTLY_TWO_LEPTONS) = false;	
	m_bools.at(HZALLYY::TWO_OPPOSITE_CHARGE_LEPTONS) = false;
	m_bools.at(HZALLYY::LEP1_LEP2_DR) = false;
	m_bools.at(HZALLYY::LEP1_LEP2_PT) = false;
	m_bools.at(HZALLYY::DILEP_MASS) = false;
	m_bools.at(HZALLYY::DILEP_PT) = false;
	m_bools.at(HZALLYY::ATLEAST_ONE_LOOSE_NonIso_PHOTON) = false;
		
	setThresholds(event, sys);
	
	// Leptons
	const xAOD::Electron* ele0 = nullptr;
	const xAOD::Electron* ele1 = nullptr;
	
	const xAOD::Muon* mu0 = nullptr;
	const xAOD::Muon* mu1 = nullptr;
	
	if (electrons->size() >= 2) {
	  ele0 = electrons->at(0);
	  ele1 = electrons->at(1);
	}
	
	if (muons->size() >= 2) {
	  mu0 = muons->at(0);
	  mu1 = muons->at(1);
	}
	
	evaluateTriggerCuts(event, ele0, ele1, mu0, mu1, m_llyyCuts, sys);
	evaluateLeptonCuts(*electrons, *muons, m_llyyCuts);
	evaluatePhotonCuts(*photons, m_llyyCuts);
	
	bool passedall = true;
	for (const auto& [key, value] : m_boolnames) {
	  auto it = std::find(m_STANDARD_CUTS.begin(), m_STANDARD_CUTS.end(), value);
	  if (it != m_STANDARD_CUTS.end()) {
	    passedall &= m_bools.at(key);
	  }
	}
	m_passallcuts.set(*event, passedall, sys);
	
	bool pass_baseline=false;
	if(m_bools.at(HZALLYY::PASS_TRIGGER) &&
	   m_bools.at(HZALLYY::EXACTLY_TWO_LEPTONS) &&
	   m_bools.at(HZALLYY::TWO_OPPOSITE_CHARGE_LEPTONS) &&
	   m_bools.at(HZALLYY::LEP1_LEP2_DR) && 
	   m_bools.at(HZALLYY::LEP1_LEP2_PT) &&
	   m_bools.at(HZALLYY::DILEP_MASS) &&
	   m_bools.at(HZALLYY::DILEP_PT) &&
	   m_bools.at(HZALLYY::ATLEAST_ONE_LOOSE_NonIso_PHOTON)) pass_baseline=true;
	
	if ((m_bypass or pass_baseline)) filter.setPassed(true);
	
	// do the CUTFLOW only with sys="" -> NOSYS
	if (sys.name()=="" && m_saveCutFlow) {
	  
	  // Compute total_events
	  m_total_events+=1;
	  if (m_isMC) m_total_mcEventWeight+=  m_generatorWeight.get(*event, sys);
	  
	  for (const auto &cut : m_inputCutKeys) {
	    if (m_llyyCuts.exists(m_boolnames.at(cut))) {
	      m_llyyCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
	      if (m_llyyCuts(m_boolnames.at(cut)).passed) {
		m_llyyCuts(m_boolnames.at(cut)).counter += 1;
		if (m_isMC) m_llyyCuts(m_boolnames.at(cut)).w_counter += m_generatorWeight.get(*event, sys);
	      }
	    }
	  }
	}
	
	// Check how many consecutive cuts are passed by the event.
	unsigned int consecutive_cuts = 0;
	for (size_t i = 0; i < m_llyyCuts.size(); ++i) {
	  if (m_llyyCuts[i].passed)
	    consecutive_cuts++;
	  else
	    break;
	}
	
	// Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
	// I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes.
	for (unsigned int i=0; i<consecutive_cuts; i++) {
	  m_llyyCuts[i].relativeCounter+=1;
	}
	
	for (auto& [key, var] : m_bools) {
	  m_Bbranches.at(key).set(*event, var, sys);
	}
      }
    return StatusCode::SUCCESS;
  }

  StatusCode HZAllyySelectorAlg::finalize()
  {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_llyyCuts.CheckCutResults(); // Print CheckCutResults
    
    if(m_saveCutFlow) {
      m_llyyCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_llyyCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_llyyCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_llyyCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
      
    }
    
    return StatusCode::SUCCESS;
  }
  
  void HZAllyySelectorAlg::evaluateTriggerCuts
  (const xAOD::EventInfo *event,
   const xAOD::Electron* ele0, const xAOD::Electron* ele1,
   const xAOD::Muon* mu0, const xAOD::Muon* mu1,
   CutManager& llyyCuts, const CP::SystematicSet& sys) {
    
    if (!llyyCuts.exists("PASS_TRIGGER"))
      return;
    
    if (ele0 || mu0) evaluateSingleLeptonTrigger(event, ele0, mu0, sys);
    if (ele1 || mu1) evaluateSingleLeptonTrigger(event, ele1, mu1, sys);
    if ((ele0 && ele1) || (mu0 && mu1)) evaluateDiLeptonTrigger(event, ele0, ele1, mu0, mu1, sys);
    
    if (m_bools.at(HZALLYY::pass_trigger_SLT) || m_bools.at(HZALLYY::pass_trigger_DLT) ) m_bools.at(HZALLYY::PASS_TRIGGER) = true;
  }
  
  void HZAllyySelectorAlg::evaluateSingleLeptonTrigger
  (const xAOD::EventInfo *event,
   const xAOD::Electron *ele, const xAOD::Muon *mu,
   const CP::SystematicSet& sys)
  {
    // Check single electron triggers
    std::vector<std::string> single_ele_paths;
    
    int year = m_year.get(*event, sys);
    if(year==2015){
      single_ele_paths = {
        "HLT_e24_lhmedium_L1EM20VH", "HLT_e60_lhmedium",
        "HLT_e120_lhloose"
      };
    }
    else if(2016<=year && year<=2018){
      single_ele_paths = {
        "HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0",
        "HLT_e140_lhloose_nod0"
      };
    }
    else if(m_is22_75bunches.get(*event, sys)){
      single_ele_paths = {
        "HLT_e17_lhvloose_L1EM15VHI", "HLT_e20_lhvloose_L1EM15VH",
        "HLT_e250_etcut_L1EM22VHI"
      };
    }
    else if(year==2022){
      single_ele_paths = {
        "HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
        "HLT_e140_lhloose_L1EM22VHI", "HLT_e300_etcut_L1EM22VHI"
      };
    }
    else if(m_is23_75bunches.get(*event, sys)){
      single_ele_paths = {
        "HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
        "HLT_e140_lhloose_L1EM22VHI", "HLT_e140_lhloose_noringer_L1EM22VHI",
        "HLT_e300_etcut_L1EM22VHI"
      };
    }
    else if(year==2023){
      single_ele_paths = {
        "HLT_e26_lhtight_ivarloose_L1eEM26M", "HLT_e60_lhmedium_L1eEM26M",
        "HLT_e140_lhloose_L1eEM26M", "HLT_e140_lhloose_noringer_L1eEM26M",
        "HLT_e300_etcut_L1eEM26M"
      };
    }
    
    bool trigPassed_SET = false;
    if(ele){
      for(const auto& trig : single_ele_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          bool match = m_matchingTool->match(*ele, trig);
          trigPassed_SET |= match;
        }
      }
      trigPassed_SET &= ele->pt() > m_pt_threshold[HZALLYY::SLT][HZALLYY::ele];
    }

    // Check single muon triggers
    std::vector<std::string> single_mu_paths;
    
    if(year==2015){
      single_mu_paths = {"HLT_mu20_iloose_L1MU15", "HLT_mu50"};
    }
    else if(2016<=year && year<=2018){
      single_mu_paths = {"HLT_mu26_ivarmedium", "HLT_mu50"};
    }
    else if(2022<=year && year<=2023 &&
	    !m_is22_75bunches.get(*event, sys) &&
	    !m_is23_75bunches.get(*event, sys) &&
	    !m_is23_400bunches.get(*event, sys)){
      single_mu_paths = {
        "HLT_mu24_ivarmedium_L1MU14FCH", "HLT_mu50_L1MU14FCH",
        "HLT_mu60_0eta105_msonly_L1MU14FCH", "HLT_mu60_L1MU14FCH",
        "HLT_mu80_msonly_3layersEC_L1MU14FCH"
      };
    }
    
    bool trigPassed_SMT = false;
    if (mu){
      for(const auto& trig : single_mu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          bool match = m_matchingTool->match(*mu, trig);
          trigPassed_SMT |= match;
        }
      }
      trigPassed_SMT &= mu->pt() > m_pt_threshold[HZALLYY::SLT][HZALLYY::mu];
    }
    
    m_bools.at(HZALLYY::pass_trigger_SLT) |= (trigPassed_SET || trigPassed_SMT);
  }
  
  void HZAllyySelectorAlg::evaluateDiLeptonTrigger
  (const xAOD::EventInfo *event,
   const xAOD::Electron *ele0, const xAOD::Electron *ele1,
   const xAOD::Muon *mu0, const xAOD::Muon *mu1,
   const CP::SystematicSet& sys)
  {
    std::vector<std::string> di_ele_paths;
    
    int year = m_year.get(*event, sys);
    if(year==2015){
      di_ele_paths = {"HLT_2e12_lhloose_L12EM10VH"};
    }
    else if(year==2016){
      di_ele_paths = {"HLT_2e17_lhvloose_nod0"};
    }
    else if(m_is17_periodB5_B8.get(*event, sys)){
      di_ele_paths = {
        "HLT_2e24_lhvloose_nod0"
      };
    }
    else if(2017<=year && year<=2018){
      di_ele_paths = {
        "HLT_2e17_lhvloose_nod0_L12EM15VHI", "HLT_2e24_lhvloose_nod0"
      };
    }
    else if(year==2022){
      di_ele_paths = {
        "HLT_2e17_lhvloose_L12EM15VHI", "HLT_2e24_lhvloose_L12EM20VH"
      };
    }
    else if(year==2023){
      di_ele_paths = {
        "HLT_2e17_lhvloose_L12eEM18M", "HLT_2e24_lhvloose_L12eEM24L"
      };
    }
    
    bool trigPassed_DET = false;
    if (ele0 && ele1) {
      for (const auto &trig : di_ele_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass) {
          bool match = m_matchingTool->match({ele0, ele1}, trig);
          trigPassed_DET |= match;
        }
      }
      trigPassed_DET &= ele0->pt() > m_pt_threshold[HZALLYY::DLT][HZALLYY::leadingele];
      trigPassed_DET &= ele1->pt() > m_pt_threshold[HZALLYY::DLT][HZALLYY::subleadingele];
    }
    
    // Check di-muon triggers
    std::vector<std::string> di_mu_paths;
    
    if(year==2015){
      di_mu_paths = {"HLT_mu18_mu8noL1"};
    }
    else if(2016<=year && year<=2018){
      di_mu_paths = {"HLT_mu22_mu8noL1"};
    }
    else if(2022<=year && year<=2023){
      di_mu_paths = {"HLT_mu22_mu8noL1_L1MU14FCH", "HLT_2mu14_L12MU8F"};
    }
    
    bool trigPassed_DMT = false;
    if (mu0 && mu1) {
      for (const auto &trig : di_mu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass) {
          bool match = m_matchingTool->match({mu0, mu1}, trig);
          trigPassed_DMT |= match;
        }
      }
      trigPassed_DMT &= mu0->pt() > m_pt_threshold[HZALLYY::DLT][HZALLYY::leadingmu];
      trigPassed_DMT &= mu1->pt() > m_pt_threshold[HZALLYY::DLT][HZALLYY::subleadingmu];
    }
    
    m_bools.at(HZALLYY::pass_trigger_DLT) = (trigPassed_DET || trigPassed_DMT);
  }
  
  void HZAllyySelectorAlg::evaluateLeptonCuts
  (const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons,
   CutManager& llyyCuts)
  {
    bool Two_Opposite_Sign_Electrons = false;
    bool Two_Opposite_Sign_Muons = false;
    bool ee_deltaR = false;
    bool mumu_deltaR = false;
    bool electron_pT_thr_pass = false;
    bool muon_pT_thr_pass = false;
    bool ee_mass_thr_pass = false;
    bool mumu_mass_thr_pass = false;
    bool ee_pT_thr_pass = false;
    bool mumu_pT_thr_pass = false;
    
    
    if ( (electrons.size() == 2 || muons.size() == 2) &&
	 llyyCuts.exists("EXACTLY_TWO_LEPTONS"))
      {
	m_bools.at(HZALLYY::EXACTLY_TWO_LEPTONS) = true;
      }
    
    if (electrons.size() >= 2)
      {
	Two_Opposite_Sign_Electrons = electrons.at(0)->charge()*electrons.at(1)->charge() == -1;

	double ee_dR = electrons.at(0)->p4().DeltaR(electrons.at(1)->p4());
	if (ee_dR > 0.2)
	  ee_deltaR = true;
	
	if ( electrons.at(0)->pt() > 27 * Athena::Units::GeV &&  electrons.at(1)->pt() > 20 * Athena::Units::GeV  )
	  electron_pT_thr_pass = true;

	double m_ee = (electrons.at(0)->p4() + electrons.at(1)->p4()).M();
	if (m_ee >= 81 * Athena::Units::GeV && m_ee <= 101 * Athena::Units::GeV)
	  ee_mass_thr_pass = true;
	
	double pT_ee = (electrons.at(0)->p4() + electrons.at(1)->p4()).Pt();
	if (pT_ee > 10 * Athena::Units::GeV)
	  ee_pT_thr_pass = true;
      }
    
    if (muons.size() >= 2)
      {
	Two_Opposite_Sign_Muons = muons.at(0)->charge()*muons.at(1)->charge() == -1;

	double mumu_dR = muons.at(0)->p4().DeltaR(muons.at(1)->p4());
	if (mumu_dR > 0.2)
	  mumu_deltaR = true;
	
	if ( muons.at(0)->pt() > 27 * Athena::Units::GeV &&  muons.at(1)->pt() > 20 * Athena::Units::GeV  )
	  muon_pT_thr_pass = true;

	double m_mumu = (muons.at(0)->p4() + muons.at(1)->p4()).M();
	if (m_mumu >= 81 * Athena::Units::GeV && m_mumu <= 101 * Athena::Units::GeV)
	  mumu_mass_thr_pass = true;
	
	double pT_mumu = (muons.at(0)->p4() + muons.at(1)->p4()).Pt();
	if (pT_mumu > 10 * Athena::Units::GeV)
	  mumu_pT_thr_pass = true;
      }
    
    if ((Two_Opposite_Sign_Electrons || Two_Opposite_Sign_Muons )
	&& llyyCuts.exists("TWO_OPPOSITE_CHARGE_LEPTONS"))
      {
	m_bools.at(HZALLYY::TWO_OPPOSITE_CHARGE_LEPTONS) = true;
      }

    if ( (ee_deltaR || mumu_deltaR) && llyyCuts.exists("LEP1_LEP2_DR"))
      {
	m_bools.at(HZALLYY::LEP1_LEP2_DR) = true;
      }
    
    if ( (electron_pT_thr_pass || muon_pT_thr_pass) &&  llyyCuts.exists("LEP1_LEP2_PT"))
      {
	m_bools.at(HZALLYY::LEP1_LEP2_PT) = true;
      }

    if ( (ee_mass_thr_pass || mumu_mass_thr_pass) &&  llyyCuts.exists("DILEP_MASS"))
      {
	m_bools.at(HZALLYY::DILEP_MASS) = true;
      }
    
    if ( (ee_pT_thr_pass || mumu_pT_thr_pass) && llyyCuts.exists("DILEP_PT"))
      {
	m_bools.at(HZALLYY::DILEP_PT) = true;
      }

  }
  
 void HZAllyySelectorAlg::evaluatePhotonCuts
  (const xAOD::PhotonContainer& photons, CutManager& llyyCuts)
    
  {
    if (photons.size() > 0 && llyyCuts.exists("ATLEAST_ONE_LOOSE_NonIso_PHOTON"))
      m_bools.at(HZALLYY::ATLEAST_ONE_LOOSE_NonIso_PHOTON) = true;
  }
  
  void HZAllyySelectorAlg::setThresholds(const xAOD::EventInfo* event,
					 const CP::SystematicSet& sys) {
    
    int year = m_year.get(*event, sys);

    // Single-lepton triggers
    if(year==2015)
      m_pt_threshold[HZALLYY::SLT][HZALLYY::ele] = 25. * Athena::Units::GeV;
    // 2022 75 bunches
    else if(m_is22_75bunches.get(*event, sys))
      m_pt_threshold[HZALLYY::SLT][HZALLYY::ele] = 18. * Athena::Units::GeV;
    else
      m_pt_threshold[HZALLYY::SLT][HZALLYY::ele] = 27. * Athena::Units::GeV;

    if(year==2015)
      m_pt_threshold[HZALLYY::SLT][HZALLYY::mu] = 21. * Athena::Units::GeV;
    else if(year>=2016 && year<=2018)
      m_pt_threshold[HZALLYY::SLT][HZALLYY::mu] = 27. * Athena::Units::GeV;
    else
      m_pt_threshold[HZALLYY::SLT][HZALLYY::mu] = 25. * Athena::Units::GeV;

    //Di-lepton triggers
    //ee
    if(year==2015) {
      m_pt_threshold[HZALLYY::DLT][HZALLYY::leadingele] = 13. * Athena::Units::GeV;
      m_pt_threshold[HZALLYY::DLT][HZALLYY::subleadingele] = 13. * Athena::Units::GeV;
    }
    // prescaled periods B5-B8
    // https://twiki.cern.ch/twiki/bin/view/Atlas/TrigEgammaRecommendedTriggers2017
    else if(m_is17_periodB5_B8.get(*event, sys)) {
      m_pt_threshold[HZALLYY::DLT][HZALLYY::leadingele] = 25. * Athena::Units::GeV;
      m_pt_threshold[HZALLYY::DLT][HZALLYY::subleadingele] = 25. * Athena::Units::GeV;
    } else {
      m_pt_threshold[HZALLYY::DLT][HZALLYY::leadingele] = 18. * Athena::Units::GeV;
      m_pt_threshold[HZALLYY::DLT][HZALLYY::subleadingele] = 18. * Athena::Units::GeV;
    }

    //mm
    if(year==2015) {
      m_pt_threshold[HZALLYY::DLT][HZALLYY::leadingmu] = 19. * Athena::Units::GeV;
      m_pt_threshold[HZALLYY::DLT][HZALLYY::subleadingmu] = 9. * Athena::Units::GeV;
    }
    else if(year>=2016 && year<=2018) {
      m_pt_threshold[HZALLYY::DLT][HZALLYY::leadingmu] = 23. * Athena::Units::GeV;
      m_pt_threshold[HZALLYY::DLT][HZALLYY::subleadingmu] = 9. * Athena::Units::GeV;
    } else {
      m_pt_threshold[HZALLYY::DLT][HZALLYY::leadingmu] = 15. * Athena::Units::GeV;
      m_pt_threshold[HZALLYY::DLT][HZALLYY::subleadingmu] = 15. * Athena::Units::GeV;
    }

  }

  StatusCode  HZAllyySelectorAlg::initialiseCutflow(){
    
    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_llyyCuts.CheckInputCutList(m_inputCutList, boolnameslist);

    m_inputCutKeys.resize(m_inputCutList.size());
    std::vector<bool> inputWasFound (m_inputCutList.size(), false);
    for (const auto& [key, value]: m_boolnames) {
      auto it = std::find(m_inputCutList.begin(), m_inputCutList.end(), value);
      if (it != m_inputCutList.end()) {
        auto index = it - m_inputCutList.begin();
        m_inputCutKeys.at(index) = key;
        inputWasFound.at(index) = true;
      }
    }

    for (unsigned int index = 0; index < inputWasFound.size(); index++) {
      if(inputWasFound.at(index)) continue;
      ATH_MSG_ERROR("Doubled or falsely spelled cuts in CutList (see config file)." + m_inputCutList[index]);
    }

    for (const auto &cut : m_inputCutKeys) {
      m_llyyCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_llyyCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of H->Za->llyy cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of H->Za->llyy cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of H->Za->llyy cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }

  
}

