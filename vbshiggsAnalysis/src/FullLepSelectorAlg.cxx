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
    
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));    

  
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
  
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      bool WPgiven = !m_isBtag.empty();
      std::vector<const xAOD::Jet*> bjets;
      std::vector<const xAOD::Jet*> nonbjets;
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets.push_back(jet);
          else nonbjets.push_back(jet);
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

      m_bools.at(VBSHIGGS::Pass_ll) = false;
      m_bools.at(VBSHIGGS::IS_SF) = false;
      m_bools.at(VBSHIGGS::IS_ee) = false;
      m_bools.at(VBSHIGGS::IS_mm) = false;
      m_bools.at(VBSHIGGS::IS_em) = false;
      m_bools.at(VBSHIGGS::PASS_TRIGGER) = false;
      m_bools.at(VBSHIGGS::EXACTLY_TWO_LEPTONS) = false;
      m_bools.at(VBSHIGGS::TWO_SS_CHARGE_LEPTONS) = false;
      m_bools.at(VBSHIGGS::PASS_MET) = false;
      m_bools.at(VBSHIGGS::EXACTLY_TWO_B_JETS) = false;
      m_bools.at(VBSHIGGS::PASS_DELTA_R_BB) = false;
      m_bools.at(VBSHIGGS::PASS_mBB) = false;
      m_bools.at(VBSHIGGS::PASS_VBS_BASELINE) = false;
      
      leptonSelection(*electrons, *muons, met);
      bjetSelection(bjets);
      vbsjetsSelection(nonbjets);
      
      bool passedall = true;
      for (const auto& [key, value] : m_boolnames) {
        auto it = std::find(m_STANDARD_CUTS.begin(), m_STANDARD_CUTS.end(), value);
        if (it != m_STANDARD_CUTS.end()) {
          passedall &= m_bools.at(key);
        }
      }

      m_passallcuts.set(*event, passedall, sys);

      bool pass_baseline_SR=false;
      if(m_bools.at(VBSHIGGS::EXACTLY_TWO_LEPTONS)
        && m_bools.at(VBSHIGGS::TWO_SS_CHARGE_LEPTONS)
        && m_bools.at(VBSHIGGS::PASS_MET)
        && m_bools.at(VBSHIGGS::EXACTLY_TWO_B_JETS)
        && m_bools.at(VBSHIGGS::PASS_DELTA_R_BB)
        && m_bools.at(VBSHIGGS::PASS_mBB)
        && m_bools.at(VBSHIGGS::PASS_VBS_BASELINE) ) pass_baseline_SR=true;
      
      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()!="") continue;
      
      // Compute total_events
      m_total_events+=1;
      
      // Count how many cuts the event passed and increase the relative counter
      for (const auto &cut : m_inputCutList) {
        if(m_vbshiggsCuts.exists(cut)) {
          if (m_vbshiggsCuts(cut).passed)
            m_vbshiggsCuts(cut).counter+=1;
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
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes.
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_vbshiggsCuts[i].relativeCounter+=1;
      }

      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      }

      if (!m_bypass && !pass_baseline_SR) continue;
      
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


  void FullLepSelectorAlg :: leptonSelection (const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons, const xAOD::MissingET *met){

    bool two_same_sign_electrons = false;
    bool two_same_sign_muons = false;
    bool two_same_sign_leptons = false;

    // Exactly two leptons selection
    if (electrons.size() + muons.size() == 2) m_bools.at(VBSHIGGS::EXACTLY_TWO_LEPTONS) = true;
    // two same sign leptons
    if (electrons.size() >= 2){
      two_same_sign_electrons = electrons.at(0)->charge()*electrons.at(1)->charge() == 1;
    }
    if (muons.size() >= 2){
      two_same_sign_muons = muons.at(0)->charge()*muons.at(1)->charge() == 1;
    }
    if (electrons.size() == 1 && muons.size() == 1){
      two_same_sign_leptons = electrons.at(0)->charge()*muons.at(0)->charge() == 1;
    }
    
    if ( two_same_sign_electrons || two_same_sign_muons || two_same_sign_leptons)
      m_bools.at(VBSHIGGS::TWO_SS_CHARGE_LEPTONS)= true;

    // met cut
    if (met->met() > 30 * Athena::Units::GeV) m_bools.at(VBSHIGGS::PASS_MET) = true;
  }//Lepton Selection

  void FullLepSelectorAlg :: bjetSelection(std::vector<const xAOD::Jet*> bjets) {
    if (bjets.size()<2) return;

    // require exactly 2 bjets in the event
    if (bjets.size()==2) m_bools.at(VBSHIGGS::EXACTLY_TWO_B_JETS)= true;

    const xAOD::Jet * lead_bjet = bjets.at(0);
    const xAOD::Jet * sublead_bjet = bjets.at(1);
    
    //Delta R(b, b) cut 
    float min_DR_bb = 2.;
    float dR_bb = lead_bjet->p4().DeltaR(sublead_bjet->p4());
    if (dR_bb < min_DR_bb)
      m_bools.at(VBSHIGGS::PASS_DELTA_R_BB) = true;
    
    float low_mbb = 100.;
    float high_mbb = 160.; 
    float mbb = (lead_bjet->p4() + sublead_bjet->p4()).M();
    if (mbb > low_mbb * Athena::Units::GeV && mbb < high_mbb * Athena::Units::GeV ){
      m_bools.at(VBSHIGGS::PASS_mBB) = true;
    }
  }//bjet selections
  
  void FullLepSelectorAlg :: vbsjetsSelection(std::vector<const xAOD::Jet*> nonbjets){
    float max_mjj = 0.;
    float delta_eta_jj = 0.;

    //TODO, create an object holding the vbs jets passing the mjj + deta selections
    const xAOD::Jet* vbsJet1 = nullptr;
    const xAOD::Jet* vbsJet2 = nullptr;

    for(unsigned int i=0;i<nonbjets.size();i++){
      for(unsigned int j=0;j<i;j++){

        if (nonbjets[i]->eta() * nonbjets[j]->eta() > 0) continue; //back-to-back?
        const xAOD::Jet* nonbjet1 = nonbjets.at(i);
        const xAOD::Jet* nonbjet2 = nonbjets.at(j);

        // TODO, perhaps we need a minimum pt requirement on vbs jets
        //if (nonbjet1->pt() < 30. * Athena::Units::GeV || nonbjet2->pt() < 30. * Athena::Units::GeV) continue;
        double mjj = (nonbjet1->p4() + nonbjet2->p4()).M();
        if (mjj > max_mjj) {
          max_mjj = mjj;
          delta_eta_jj = std::abs(nonbjet1->eta() - nonbjet2->eta());;
          vbsJet1 = nonbjet1;
          vbsJet2 = nonbjet2;
        } 
      }
    }
    if ( vbsJet1 && vbsJet2 && max_mjj > 900 * Athena::Units::GeV  && delta_eta_jj > 3.0 ) {
      m_bools.at(VBSHIGGS::PASS_VBS_BASELINE) = true;
    }
  }//vbsjetsSelection
}//name space