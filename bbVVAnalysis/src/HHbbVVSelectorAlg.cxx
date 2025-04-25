/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Kira Abeling, JaeJin Hong

#include "HHbbVVSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>

namespace HHBBVV
{
  HHbbVVSelectorAlg::HHbbVVSelectorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator) {}

  StatusCode HHbbVVSelectorAlg::initialize()
  {

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    
    // Choose the first GN2X wp
    m_Pass_GN2X = CP::SysReadDecorHandle<int>
      ("xbb_select_GN2Xv01_" + m_GN2X_wps.value().front(), this);
    ATH_CHECK (m_Pass_GN2X.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK (m_Tau2_wta.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK (m_Tau4_wta.initialize(m_systematicsList, m_lrjetHandle));

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    m_eleWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_" + m_eleWPName+"_%SYS%", this);

    m_muonWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_muonWPName+"_%SYS%", this);
    
    ATH_CHECK(m_eleWPDecorHandle.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_muonWPDecorHandle.initialize(m_systematicsList, m_muonHandle));
    
    ATH_CHECK(m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_selected_mu.initialize(m_systematicsList, m_muonHandle));

    ATH_CHECK(m_Whad.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK(m_Whad2.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK(m_Hbb.initialize(m_systematicsList, m_lrjetHandle));

    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));


    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    // Intialise syst-aware output decorators
    ATH_CHECK(m_pass_sr.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());
      
    for ( auto name : m_channel_names){
      if (name.std::string::find("1Lep") != std::string::npos)m_run_lep = true;
      if (name.std::string::find("0Lep") != std::string::npos)m_run_had = true;
      if( name == "Boosted1Lep") m_channels.push_back(HHBBVV::Boosted1Lep);
      else if ( name == "SplitBoosted1Lep") m_channels.push_back(HHBBVV::SplitBoosted1Lep);
      else if( name == "Boosted0Lep") m_channels.push_back(HHBBVV::Boosted0Lep);
      else if ( name == "SplitBoosted0Lep") m_channels.push_back(HHBBVV::SplitBoosted0Lep);
      else{
        ATH_MSG_ERROR("Unknown channel: "
          << name << std::endl
          << "Available are: [\"Boosted1Lep\", \"SplitBoosted1Lep\", \"Boosted0Lep\", \"SplitBoosted0Lep\"]");
        return StatusCode::FAILURE;
      }
    }

    if(m_saveCutFlow) ATH_CHECK (initialiseCutflow());
    return StatusCode::SUCCESS;
  }

  StatusCode HHbbVVSelectorAlg::execute()
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

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

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

      // reset all cut flags to default=false
      for (CutEntry& cut : m_bbVVCuts) {
        cut.passed = false;
      }
      n_evt+=1;


      // Apply selection
      // baseline selection for boosted in bbVV_boosted_config.py

      // flags
      bool TWO_LRJETS = false;
      bool THREE_LRJETS = false;
      bool ONE_LEP = false;
      bool VETO_LEP = false;
      bool ONELEPBOOSTED_TOPO = false;
      bool ONELEPSPLITBOOSTED_TOPO = false;
      bool ZEROLEPBOOSTED_TOPO = false;
      bool ZEROLEPSPLITBOOSTED_TOPO = false;
      bool HBB_BTAG = false;
      bool WHAD_BTAG = false;
      bool WHAD2_BTAG = false;

      //************
      // lepton
      //************
      int n_leptons = 0;
      TLorentzVector signal_lepton;
      //example - needs to match bbVV definition
      // requirements for signal electrons
      for (const xAOD::Electron *electron : *electrons)
      {
        bool passElectronWP = m_eleWPDecorHandle.get(*electron, sys);
        m_selected_el.set(*electron, false, sys);
        if (passElectronWP && electron->pt() > 18. * Athena::Units::GeV)
        {
          // give passing electron decoration "selected_el"
          m_selected_el.set(*electron, true, sys);
          signal_lepton = electron->p4();
          n_leptons += 1;
        }
      }

      // requirements for signal muons
      for (const xAOD::Muon *muon : *muons)
      {
        bool passMuonWP = m_muonWPDecorHandle.get(*muon, sys);
        m_selected_mu.set(*muon, false, sys);
        if (passMuonWP && std::abs(muon->eta()) < 2.5 && muon->pt() > 15. * Athena::Units::GeV)
        {
          m_selected_mu.set(*muon, true, sys); // currently could be multiple selected per event?
          signal_lepton = muon->p4(); // might get overwritten?
          n_leptons += 1;
        }
      }
      // check for exactly one lepton
      if (m_run_lep && n_leptons == 1)
      {
        ONE_LEP = true;
        m_bbVVCuts("EXACTLY_ONE_LEPTON").passed = true;
      }
      else if (m_run_had && n_leptons == 0)
      {
        VETO_LEP = true;
        m_bbVVCuts("VETO_SIGNAL_LEPTON").passed = true;
      }

      //************
      // pflow jet
      //************
      int n_jets = 0;
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          SG::VIEW_ELEMENTS);

      for (const xAOD::Jet *jet : *jets)
      {
        if (std::abs(jet->eta()) < 2.5)
        {
          n_jets += 1;
          if (WPgiven)
          {
            if (m_isBtag.get(*jet, sys))
              bjets->push_back(jet);
          }
        }
      }
      if (n_jets >= 2)
      {
        //TWO_JETS = true;
        Jet_evt+=1;
        //further selections can go here
      }



      //************
      // large-R jet
      //************
      int n_lrjets = lrjets->size();
      float leadLRJ_pt = -999.;

      for (const xAOD::Jet *lrjet : *lrjets)
      {
        m_Hbb.set(*lrjet,false,sys);
        m_Whad.set(*lrjet,false,sys);
        m_Whad2.set(*lrjet,false,sys);
      }
      ATH_MSG_DEBUG("# lrjets: " << n_lrjets);
      if (n_lrjets >= 2)
      {
        TWO_LRJETS = true;
        m_bbVVCuts("AT_LEAST_TWO_LRJETS").passed = true;
        //further selections can go here

        if(n_lrjets >= 3 && (std::find(m_channels.begin(), m_channels.end(),HHBBVV::SplitBoosted0Lep)!=m_channels.end())){
          THREE_LRJETS = true;
          m_bbVVCuts("AT_LEAST_THREE_LRJETS").passed = true;
        }

        leadLRJ_pt = lrjets->get(0)->pt();
      }

      if (leadLRJ_pt > 500. * Athena::Units::GeV)
      {
        m_bbVVCuts("LEAD_LRJ_PT").passed = true;
      }

      // use different classification methods for different channels
      const xAOD::Jet *Hbb = nullptr;
      const xAOD::Jet *Whad = nullptr;
      const xAOD::Jet *Whad2 = nullptr;
      for(const auto& channel : m_channels){
        ATH_MSG_DEBUG("channel: " << channel);
        if(channel == HHBBVV::Boosted1Lep)
        {
          ATH_MSG_DEBUG("---BOOSTED 1LEP JET CLASSIFICATION---");
          if (n_lrjets >= 2 && ONE_LEP)
          {
            distJetClassification(*lrjets, Hbb, Whad, signal_lepton, sys);
            signalBtagging(Hbb, Whad, sys, HBB_BTAG, WHAD_BTAG);
            ONELEPBOOSTED_TOPO = (Whad->p4().DeltaR(signal_lepton) < 1.0); // Run 2: < 1.0
            if (ONELEPBOOSTED_TOPO) m_bbVVCuts("DR_CUT").passed = true;
            if (Hbb && Hbb->pt() > 500. * Athena::Units::GeV) m_bbVVCuts("HBB_PT").passed = true;
            if (HBB_BTAG) m_bbVVCuts("HBB_BTAG").passed = true;
          }
        }
        else if(channel == HHBBVV::SplitBoosted1Lep)
        {
          ATH_MSG_DEBUG("---SPLITBOOSTED 1LEP JET CLASSIFICATION---");
          if (n_lrjets >= 2 && ONE_LEP)
          {
            massJetClassification(*lrjets, Hbb, Whad, sys);
            ONELEPSPLITBOOSTED_TOPO = (Whad->p4().DeltaR(signal_lepton) > 1.0); //condition split-boosted
            if (ONELEPSPLITBOOSTED_TOPO)
            {
              m_bbVVCuts("DR_CUT").passed = true;
            }
          }
        }
        else if(channel == HHBBVV::Boosted0Lep)
        {
          ATH_MSG_DEBUG("---BOOSTED 0LEP JET CLASSIFICATION---");
          if (n_lrjets >= 2 && VETO_LEP)
          {
            SubjetnessJetClassification(*lrjets, Hbb, Whad, sys); // WHad is the lrjet with the smaller Tau42
            signalBtagging(Hbb, Whad, sys, HBB_BTAG, WHAD_BTAG);
            ZEROLEPBOOSTED_TOPO = true; // Temporary, need to figure out how to define boosted WHad
            if (HBB_BTAG) m_bbVVCuts("HBB_BTAG").passed = true;
          }
        }

        else if(channel == HHBBVV::SplitBoosted0Lep)
        {
          ATH_MSG_DEBUG("---SPLITBOOSTED 0LEP JET CLASSIFICATION---");
          if (n_lrjets >= 3 && VETO_LEP)
          {
            splitboostedJetClassification(*lrjets, Hbb, Whad, Whad2, sys); // Hbb is the lrjet with the higher Hbb score
            signalBtagging(Hbb, Whad, Whad2, sys, HBB_BTAG, WHAD_BTAG, WHAD2_BTAG);
            ZEROLEPSPLITBOOSTED_TOPO = true; // Temporary, need to figure out how to define boosted WHad
            if (HBB_BTAG) m_bbVVCuts("HBB_BTAG").passed = true;
          }
        }
      }

      bool passedall = true;
      for (CutEntry& cut : m_bbVVCuts) {
        passedall = passedall && cut.passed;
      }
      m_passallcuts.set(*event, passedall, sys); // Mark this event all passed
      if(passedall)m_bbVVCuts.PassAllCuts += 1; // Count all passed event N


      // Count how many cuts the event passed and increase the relative counter
      for (const auto &cut : m_STANDARD_CUTS)  {
        if(m_bbVVCuts.exists(cut)) {
          if (m_bbVVCuts(cut).passed)
            m_bbVVCuts(cut).counter+=1;
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_bbVVCuts.size(); ++i) {
        if (m_bbVVCuts[i].passed)
          consecutive_cuts++;
        else
          break;
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_bbVVCuts[i].relativeCounter+=1;
      }



      //****************
      // event level info
      //****************
      //for example masses of the system, met, fired triggers etc

      bool pass = false;
      //implement dedicated selection flags here
      for(const auto& channel : m_channels){
      if(channel == HHBBVV::Boosted1Lep) pass |= (leadLRJ_pt > 500. * Athena::Units::GeV && TWO_LRJETS && ONE_LEP && ONELEPBOOSTED_TOPO);
      else if(channel == HHBBVV::SplitBoosted1Lep) pass |= (TWO_LRJETS && ONE_LEP && ONELEPSPLITBOOSTED_TOPO);
      else if(channel == HHBBVV::Boosted0Lep) pass |= (leadLRJ_pt > 500. * Athena::Units::GeV && TWO_LRJETS && VETO_LEP && ZEROLEPBOOSTED_TOPO);
      else if(channel == HHBBVV::SplitBoosted0Lep) pass |= (leadLRJ_pt > 500. * Athena::Units::GeV && THREE_LRJETS && VETO_LEP && ZEROLEPSPLITBOOSTED_TOPO);
      }
      ATH_MSG_DEBUG("pass:" << pass);
      ATH_MSG_DEBUG("NEXT EVENT");
      if (!m_bypass && !pass) continue; // maybe add skipped_evts += evt_weights;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    }
    
    return StatusCode::SUCCESS;
  }

  StatusCode HHbbVVSelectorAlg::finalize() {

    m_bbVVCuts.CheckCutResults(); // Print CheckCutResults
    if(m_saveCutFlow) 
    {
      m_bbVVCuts.DoAbsoluteEfficiency(n_evt, efficiency("AbsoluteEfficiency"));
      m_bbVVCuts.DoRelativeEfficiency(n_evt, efficiency("RelativeEfficiency"));
      m_bbVVCuts.DoStandardCutFlow(n_evt, efficiency("StandardCutFlow"));
      m_bbVVCuts.DoCutflowLabeling(n_evt, hist("EventsPassed_BinLabeling"));
    }

    // Cut information
    ATH_MSG_DEBUG("###########cuts##########");
    ATH_MSG_DEBUG("events:" << n_evt);
    ATH_MSG_DEBUG("min. 2 jets:" << Jet_evt); // Temporary, need to decide whether bbVV needs AT_LEAST_TWO_JETS cut
    ANA_CHECK (m_filterParams.finalize ());

    return StatusCode::SUCCESS;
  }

  float HHbbVVSelectorAlg::Tau42(const xAOD::Jet* lrjet, const CP::SystematicSet& sys) {
    float tau2 = m_Tau2_wta.get(*lrjet, sys);
    float tau4 = m_Tau4_wta.get(*lrjet, sys);

    float tau42 = ((tau2 >= 1.0e-8) && (tau4 >= 1.0e-8)) ? tau4 / tau2 : -99.;
    return tau42;
  }

  void HHbbVVSelectorAlg::distJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad, const TLorentzVector& lepton, const CP::SystematicSet& sys) {
    // the closest jet to the lepton is labeled as Whad
    // the pT leading jet and not Whad is labeled as Hbb
    double dRmin = 99.;
    Hbb = nullptr;
    Whad = nullptr;
    // Find the jet which is closest to the signal lep
    for (const auto lrjet : lrjets){
      float dR = lrjet->p4().DeltaR(lepton);
      if (dR < dRmin){
        dRmin = dR;
        Whad = lrjet;
      }
    }

    // Hbb is chosen to be the leading pt jet from the rest
    for (const auto lrjet : lrjets){
      if(lrjet==Whad) continue;
      Hbb = lrjet; break;
    }

    for (const auto lrjet : lrjets){
      if(Whad) m_Whad.set(*lrjet, lrjet==Whad, sys);
      if(Hbb) m_Hbb.set(*lrjet, lrjet==Hbb, sys);
    }

  }

  void HHbbVVSelectorAlg::massJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad, const CP::SystematicSet& sys) {
    //classify jets as coming from W and H
    // The logic apply mH first, then from the remaining LRJs apply mW method
    double closest_dist_Hbb = FLT_MAX; // Tolerance for finding the jet mass closet to mW
    double closest_dist_Whad = FLT_MAX;
    Hbb = nullptr;
    Whad = nullptr;

    for (const auto lrjet : lrjets){
      float dist = std::abs(lrjet->m() - m_Hmass); // HMass distance
      if (dist < closest_dist_Hbb){
        closest_dist_Hbb = dist;
        Hbb = lrjet;
      }
    }

    // Find the jet which has the closest jet mass to the W boson
    for (const auto lrjet : lrjets){
      if(lrjet == Hbb)continue;
      float dist = std::abs(lrjet->m() - m_Wmass); // WMass distance
      if (dist < closest_dist_Whad){
        closest_dist_Whad = dist;
        Whad = lrjet;
      }
    }

    for (const auto lrjet : lrjets){
      if(Whad) m_Whad.set(*lrjet, lrjet==Whad, sys);
      if(Hbb) m_Hbb.set(*lrjet, lrjet==Hbb, sys);
    }
  }

  void HHbbVVSelectorAlg::SubjetnessJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad, const CP::SystematicSet& sys) {
    // classify jets as coming from Hbb and WHad
    // One with the smallest Tau42 ratio is WHad candidate. The other is the Hbb candidate, closest to the m_Hmass

    float closest_dist_Hbb = FLT_MAX;
    float Tau42_val = FLT_MAX;
    Hbb = nullptr;
    Whad = nullptr;
    ATH_MSG_DEBUG("lrjetn: " << lrjets.size());

    // Find the jet which has the smallest Tau42 ratio
    for (const auto lrjet : lrjets){
      float this_Tau42_val = Tau42(lrjet, sys);
      ATH_MSG_DEBUG("this_Tau42_val: " << this_Tau42_val);
      if((this_Tau42_val < Tau42_val) && (this_Tau42_val != -99.)){ // 4-prong object(H->WW->4q) has smaller Tau43, Tau42
        Tau42_val = this_Tau42_val;
        Whad = lrjet;
      }
    }

    // From the rest of the jets find the jet which has the closest jet mass to the Higgs boson
    for (const auto lrjet : lrjets){
      if(lrjet == Whad)continue;
      float dist = std::abs(lrjet->m() - m_Hmass); // HMass distance
      ATH_MSG_DEBUG("dist: " << dist);
      if (dist < closest_dist_Hbb){
        closest_dist_Hbb = dist;
        Hbb = lrjet;
      }
    }

    for (const auto lrjet : lrjets){
      if(Whad) m_Whad.set(*lrjet, lrjet==Whad, sys);
      if(Hbb) m_Hbb.set(*lrjet, lrjet==Hbb, sys);
    }
  }

  void HHbbVVSelectorAlg::splitboostedJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad, const xAOD::Jet *&Whad2, const CP::SystematicSet& sys) {
    //classify jets as coming from W and H
    // The logic apply mW first, then from the remaining LRJs apply mH method
    double closest_dist_Hbb = FLT_MAX; // Tolerance for finding the jet mass closet to mW
    double closest_dist_Whad = FLT_MAX;
    Hbb = nullptr;
    Whad = nullptr;
    Whad2 = nullptr;

    for (const auto lrjet : lrjets){
      float dist = std::abs(lrjet->m() - m_Wmass);
      if(dist < closest_dist_Whad){
        closest_dist_Whad = dist;
        Whad = lrjet;
      }
    }

    closest_dist_Whad = FLT_MAX; // Reset for Whad2
    for (const auto lrjet : lrjets){
      if(lrjet == Whad)continue;
      float dist = std::abs(lrjet->m() - m_Wmass);
      if(dist < closest_dist_Whad){
        closest_dist_Whad = dist;
        Whad2 = lrjet;
      }
    }

    for (const auto lrjet : lrjets){
      if((lrjet == Whad) || (lrjet == Whad2))continue;
      float dist = std::abs(lrjet->m() - m_Hmass);
      if(dist < closest_dist_Hbb){
        closest_dist_Hbb = dist;
        Hbb = lrjet;
      }
    }

    for (const auto lrjet : lrjets){
      if(Whad) m_Whad.set(*lrjet, lrjet==Whad, sys);
      if(Whad2) m_Whad2.set(*lrjet, lrjet==Whad2, sys);
      if(Hbb) m_Hbb.set(*lrjet, lrjet==Hbb, sys);
    }
  }

  void HHbbVVSelectorAlg::signalBtagging(const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad, const CP::SystematicSet& sys, bool& HBB_BTAG, bool& WHAD_BTAG) {

    if(Hbb)HBB_BTAG = (bool)m_Pass_GN2X.get(*Hbb, sys); // Choose the first GN2X wp
    if(Whad)WHAD_BTAG = (bool)m_Pass_GN2X.get(*Whad, sys);

  }

  void HHbbVVSelectorAlg::signalBtagging(const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad, const xAOD::Jet *&Whad2, const CP::SystematicSet& sys, bool& HBB_BTAG, bool& WHAD_BTAG, bool& WHAD2_BTAG) {

    if(Hbb)HBB_BTAG = (bool)m_Pass_GN2X.get(*Hbb, sys); // Choose the first GN2X wp
    if(Whad)WHAD_BTAG = (bool)m_Pass_GN2X.get(*Whad, sys);
    if(Whad2)WHAD2_BTAG = (bool)m_Pass_GN2X.get(*Whad2, sys);

  }
  
  StatusCode HHbbVVSelectorAlg::initialiseCutflow(){
    m_bbVVCuts.CheckInputCutList(m_inputCutList,m_STANDARD_CUTS); // Check our inputCutList is valid

    for (const std::string &cut : m_inputCutList)  {
      // Initialize a vector of CutEntry structs based on the input Cut List
      m_bbVVCuts.add(cut);
      ATH_MSG_DEBUG("bbVVCut = " << cut);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_bbVVCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->bbVV cuts;Cuts;#epsilon", nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->bbVV cuts;Cuts;#epsilon", nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->bbVV cuts;Cuts;#epsilon", nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }

}

