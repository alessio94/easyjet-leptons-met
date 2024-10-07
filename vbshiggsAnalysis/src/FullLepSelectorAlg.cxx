/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FullLepSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace VBSHIGGS{

  FullLepSelectorAlg::FullLepSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator) : EL::AnaAlgorithm(name, pSvcLocator){  }
  StatusCode FullLepSelectorAlg::initialize(){
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     FullLepSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    ATH_CHECK (m_passTriggerSLT.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_signaljetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_HCandHandle.initialize(m_systematicsList));
    ATH_CHECK (m_vbsjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_largejetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_largejetHandle));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_signaljetHandle));
    }

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));    
    ATH_CHECK (m_year.initialize(m_systematicsList, m_eventHandle));

    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    }
   
    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_vbshiggsCuts.CheckInputCutList(m_inputCutList, boolnameslist);

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
      ATH_MSG_ERROR("CutLists don't match. Please double check your configuration " + m_inputCutList[index]);
    }

    for (const auto &cut : m_inputCutKeys) {
      m_vbshiggsCuts.add(m_boolnames[cut]);
    }

    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());
    
    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_vbshiggsCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of vbshiggs cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of vbshiggs cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of vbshiggs cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }
  
  StatusCode FullLepSelectorAlg::execute(){
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));
  
      const xAOD::JetContainer *signalJets = nullptr;
      ANA_CHECK (m_signaljetHandle.retrieve (signalJets, sys));

      const xAOD::JetContainer *HJets = nullptr;
      ANA_CHECK (m_HCandHandle.retrieve (HJets, sys));

      const xAOD::JetContainer *vbsjets = nullptr;
      if( !m_UseVBFRNN )
        ANA_CHECK (m_vbsjetHandle.retrieve (vbsjets, sys));

      const xAOD::JetContainer *largeJets = nullptr;
      ANA_CHECK (m_largejetHandle.retrieve (largeJets, sys));

      bool WPgiven = !m_isBtag.empty();
      std::vector<const xAOD::Jet*> bjets;
      for(const xAOD::Jet* jet : *signalJets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets.push_back(jet);
        }
      }

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;
      }

      for (auto& [key, value] : m_boolnames) m_bools.at(key) = false;

      if ( HJets->size() >= 2) m_bools.at(VBSHIGGS::PASS_TWO_SIGNAL_JETS) = true;
      if ( largeJets->size() >= 1 )  m_bools.at(VBSHIGGS::PASS_ONE_LARGE_JET) = true;
      
      if (!m_passTriggerSLT.empty() and m_passTriggerSLT.get(*event, sys)) {
        m_bools.at(VBSHIGGS::PASS_TRIGGER) = true;
      } 
      
      leptonSelection(electrons, muons, met);
      if( !m_UseVBFRNN )
        vbsjetsSelection(vbsjets);
      if (m_bools.at(VBSHIGGS::PASS_TWO_SIGNAL_JETS)) resolvedSelection(HJets, bjets, sys);
      if (m_bools.at(VBSHIGGS::PASS_ONE_LARGE_JET)) boostedSelection(largeJets, sys);

      bool pass_preselection = m_bools.at(VBSHIGGS::PASS_TWO_SS_CHARGE_LEPTONS) ;

      m_passallcuts.set(*event, pass_preselection, sys);

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()==""){
      
        // Compute total_events
        m_total_events+=1;

        // Count how many cuts the event passed and increase the relative counter
        for (const auto &cut : m_inputCutKeys) {
          if(m_vbshiggsCuts.exists(m_boolnames.at(cut))) {
            m_vbshiggsCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_vbshiggsCuts(m_boolnames.at(cut)).passed){
              m_vbshiggsCuts(m_boolnames.at(cut)).counter+=1;
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_vbshiggsCuts.size(); ++i) {
          if (m_vbshiggsCuts[i].passed)
            consecutive_cuts++;
          else
            break;
        }

        // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        for (unsigned int i=0; i<consecutive_cuts; i++) {
          m_vbshiggsCuts[i].relativeCounter+=1;
        }
      }

      // Fill syst-aware output decorators
      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      }

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      if (!m_bypass && !pass_preselection) continue;
      filter.setPassed(true);
    }
    return StatusCode::SUCCESS;
  }

  StatusCode FullLepSelectorAlg::finalize(){

    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_vbshiggsCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_vbshiggsCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_vbshiggsCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_vbshiggsCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_vbshiggsCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));

    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }
    return StatusCode::SUCCESS;
  }

  void FullLepSelectorAlg :: leptonSelection (const xAOD::ElectronContainer* electrons, const xAOD::MuonContainer* muons, const xAOD::MissingET *met){

    int n_leptons = 0;

    const xAOD::Electron* ele0 = nullptr;
    const xAOD::Electron* ele1 = nullptr;
    for (const xAOD::Electron *electron : *electrons){
      if(!ele0) ele0 = electron;
      else if(!ele1) ele1 = electron;

      n_leptons += 1;
    }

    if (ele1) {
      if (ele0->charge() * ele1->charge() == 1)
        m_bools.at(VBSHIGGS::PASS_TWO_SS_CHARGE_LEPTONS) = true; 
      else
        m_bools.at(VBSHIGGS::PASS_TWO_OS_CHARGE_LEPTONS) = true;
        
    }

    const xAOD::Muon* mu0 = nullptr;
    const xAOD::Muon* mu1 = nullptr;
    for (const xAOD::Muon *muon : *muons){
      if(!mu0) mu0 = muon;
      else if(!mu1) mu1 = muon;

      n_leptons += 1;
    }

    if (mu1) {
      if (mu0->charge() * mu1->charge() == 1)
        m_bools.at(VBSHIGGS::PASS_TWO_SS_CHARGE_LEPTONS) = true;
      else
        m_bools.at(VBSHIGGS::PASS_TWO_OS_CHARGE_LEPTONS) = true;
        
    }
    else if (n_leptons == 2 && mu0) {
      if (ele0->charge() * mu0->charge() == 1 )
        m_bools.at(VBSHIGGS::PASS_TWO_SS_CHARGE_LEPTONS) = true;
      else
        m_bools.at(VBSHIGGS::PASS_TWO_OS_CHARGE_LEPTONS) = true;
    }

    if (n_leptons >= 2)
      m_bools.at(VBSHIGGS::PASS_AT_LEAST_TWO_LEPTONS) = true;

    if (n_leptons == 2)
      m_bools.at(VBSHIGGS::PASS_EXACTLY_TWO_LEPTONS) = true;

    // met cut
    if (met->met() > 30 * Athena::Units::GeV) m_bools.at(VBSHIGGS::PASS_MET) = true;
  }//Lepton Selection

  void FullLepSelectorAlg :: vbsjetsSelection(const xAOD::JetContainer * vbsjets){
    
    if (vbsjets->size() >= 2){
      //TODO, create an object holding the vbs jets passing the mjj + deta selections
      const xAOD::Jet* vbsJet1 = vbsjets->at(0);
      const xAOD::Jet* vbsJet2 = vbsjets->at(1);;
      double mjj = (vbsJet1->p4() + vbsJet2->p4()).M();
      double dEta_jj = std::abs(vbsJet1->eta() - vbsJet2->eta());

      if ( mjj > 300 * Athena::Units::GeV  && dEta_jj > 3.0 ) {
        m_bools.at(VBSHIGGS::PASS_VBS_BASELINE) = true;
      }
    }
  }//vbsjetsSelection

  //Resolved Analysis
  void FullLepSelectorAlg :: resolvedSelection(const xAOD::JetContainer *HJets, std::vector<const xAOD::Jet*> bjets, const CP::SystematicSet& sys){
    int mNBJets = bjets.size();

    // require exactly 2 bjets in the event
    if (mNBJets==1) m_bools.at(VBSHIGGS::PASS_RES_EXACTLY_ONE_B_JET) = true;
    if (mNBJets==2) m_bools.at(VBSHIGGS::PASS_RES_EXACTLY_TWO_B_JETS) = true;
    if (mNBJets>=1) m_bools.at(VBSHIGGS::PASS_RES_AT_LEAST_ONE_B_JET) = true;

    const xAOD::Jet * mHJet1 = HJets->at(0);
    const xAOD::Jet * mHJet2 = HJets->at(1);

    int nSigBjets=0;
    if ( !m_isBtag.empty() && m_isBtag.get(*mHJet1, sys) ) nSigBjets++;
    if ( !m_isBtag.empty() && m_isBtag.get(*mHJet2, sys) ) nSigBjets++;

    //count extra bjets
    int nOtherBjets=mNBJets-nSigBjets;
    if (nOtherBjets == 0)
       m_bools.at(VBSHIGGS::PASS_RES_NOADD_B_JET) = true;

    //Delta R(b, b) cut 
    float min_dR_j1j2 = 2.;
    float dR_j1j2 = mHJet1->p4().DeltaR(mHJet2->p4());
    if (dR_j1j2 < min_dR_j1j2)
      m_bools.at(VBSHIGGS::PASS_DELTA_R_BB) = true;
    
    float low_mbb = 100.;
    float high_mbb = 160.;
    float mbb = (mHJet1->p4() + mHJet2->p4()).M();
    if (mbb > low_mbb * Athena::Units::GeV && mbb < high_mbb * Athena::Units::GeV ){
      m_bools.at(VBSHIGGS::PASS_RES_H_WINDOW) = true;
    }

    bool pass_resolved_baseline = m_bools.at(VBSHIGGS::PASS_TRIGGER) && m_bools.at(VBSHIGGS::PASS_AT_LEAST_TWO_LEPTONS) && m_bools.at(VBSHIGGS::PASS_RES_AT_LEAST_ONE_B_JET );

    m_bools.at(VBSHIGGS::PASS_RES_BASELINE) = pass_resolved_baseline;

  }

  //boosted Analysis
  void FullLepSelectorAlg :: boostedSelection(const xAOD::JetContainer *largeJets, const CP::SystematicSet& sys){
    //leading Large-R jet in the event
    const xAOD::Jet* largeJet = largeJets->at(0);
    
    //construct GN2X score
    float phbb = m_GN2Xv01_phbb.get(*largeJet, sys);
    float phcc = m_GN2Xv01_phcc.get(*largeJet, sys);
    float pqcd = m_GN2Xv01_pqcd.get(*largeJet, sys);
    float ptop = m_GN2Xv01_ptop.get(*largeJet, sys);
    float fcc = 0.02;
    float ftop = 0.25;
    float XbbScore= log (phbb / (fcc*phcc + ftop*ptop + pqcd*(1-fcc-ftop)));

    bool pass_merged_baseline = m_bools.at(VBSHIGGS::PASS_TRIGGER) && m_bools.at(VBSHIGGS::PASS_AT_LEAST_TWO_LEPTONS) && XbbScore > 1.560 ;

    m_bools.at(VBSHIGGS::PASS_MERG_BASELINE) = pass_merged_baseline;

  }
}//name space
