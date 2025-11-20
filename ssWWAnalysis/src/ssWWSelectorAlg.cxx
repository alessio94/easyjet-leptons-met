/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ssWWSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>


namespace ssWWVBS
{

  ssWWSelectorAlg::ssWWSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode ssWWSelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     ssWWSelectorAlg      \n");
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
    
    // Pass the lepton WP
    m_eleWPDecorHandle = CP::SysReadDecorHandle<char>
    ("baselineSelection_" + m_eleWPName+"_%SYS%", this);
    m_muonWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_muonWPName+"_%SYS%", this);
    ATH_CHECK(m_eleWPDecorHandle.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_muonWPDecorHandle.initialize(m_systematicsList, m_muonHandle));
    
    // special decorators for leptons
    ATH_CHECK (m_ele_selected.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_mu_selected.initialize(m_systematicsList, m_muonHandle));

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
    for ( auto name : m_channel_names){
      //std::cout<<"name=  "<<name<<std::endl;
      if( name == "SR") m_channels.push_back(ssWWVBS::SR);
      else if ( name == "WZCR") m_channels.push_back(ssWWVBS::WZCR);
      else if ( name == "misIDCR") m_channels.push_back(ssWWVBS::misIDCR);
      else if ( name == "incVR") m_channels.push_back(ssWWVBS::incVR);
      else if ( name == "LowDyVR") m_channels.push_back(ssWWVBS::LowDyVR);
      else if ( name == "LowMjjVR") m_channels.push_back(ssWWVBS::LowMjjVR);
      else if ( name == "LowNjVR") m_channels.push_back(ssWWVBS::LowNjVR);
      else if ( name == "tFakeVR") m_channels.push_back(ssWWVBS::tFakeVR);
      else if ( name == "tEWKVR") m_channels.push_back(ssWWVBS::tEWKVR);
      else if ( name == "lllVR") m_channels.push_back(ssWWVBS::lllVR);
      else if ( name == "ZeeVR") m_channels.push_back(ssWWVBS::ZeeVR);
      else if ( name == "DijetsCR") m_channels.push_back(ssWWVBS::DijetsCR);
      else{
        ATH_MSG_ERROR("Unknown channel");
        return StatusCode::FAILURE;
      }
    }

    if(m_saveCutFlow) ATH_CHECK (initialiseCutflow());
    return StatusCode::SUCCESS;
  }


  StatusCode ssWWSelectorAlg::execute()
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
      

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto nonbjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet);
          else nonbjets->push_back(jet);
        }
      }
 
      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));
      
      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      //All Met containner names={"RefEle","Muons","MuonEloss","RefJet","PVSoftTrk","Final"}
      //The sum of the former 5 turns is the "Final"
      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"]; // To check
      const xAOD::MissingET* met_track = (*metCont)["PVSoftTrk"];
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;
      }
      if (!met_track) {
        ATH_MSG_ERROR("Could not retrieve PVSoftTrk MET");
        return StatusCode::FAILURE;
      }

      m_bools.at(ssWWVBS::IS_ee) = false;
      m_bools.at(ssWWVBS::IS_mm) = false;
      m_bools.at(ssWWVBS::IS_em) = false;
      m_bools.at(ssWWVBS::IS_me) = false;

      m_bools.at(ssWWVBS::pass_trigger_SLT) = false;
      m_bools.at(ssWWVBS::pass_trigger_prescaleSLT) = false;
      m_bools.at(ssWWVBS::PASS_TRIGGER) = false;
      m_bools.at(ssWWVBS::PASS_TWO_LEPTONS) = false;
      m_bools.at(ssWWVBS::PASS_LEPTON_ID) = false;
      m_bools.at(ssWWVBS::EXACTLY_TWO_LEPTONS) = false;
      m_bools.at(ssWWVBS::TWO_SAME_CHARGE_LEPTONS) = false;
      m_bools.at(ssWWVBS::DILEPTON_MASS_THRESHOLD) = false;
      m_bools.at(ssWWVBS::DILEPTON_MASS_SIDEBAND_EE) = false;
      m_bools.at(ssWWVBS::MET) = false;
      m_bools.at(ssWWVBS::AT_LEAST_TWO_JETS) = false;
      m_bools.at(ssWWVBS::DIJETS_MASS_LOW) = false;
      m_bools.at(ssWWVBS::DIJETS_MASS_HIGH) = false;
      m_bools.at(ssWWVBS::DIJETS_DELTA_RAPIDITY) = false;
      m_bools.at(ssWWVBS::BJET_VETO) = false;
      m_bools.at(ssWWVBS::pass_SR) = false;

      m_bools.at(ssWWVBS::PASS_THREE_LEPTONS) = false; 
      m_bools.at(ssWWVBS::EXACTLY_THREE_LEPTONS) = false;
      m_bools.at(ssWWVBS::pass_WZCR) = false;
      m_bools.at(ssWWVBS::pass_misIDCR) = false;
      m_bools.at(ssWWVBS::pass_incVR) = false;
      m_bools.at(ssWWVBS::pass_LowDyVR) = false;
      m_bools.at(ssWWVBS::pass_LowMjjVR) = false;
      m_bools.at(ssWWVBS::pass_LowNjVR) = false;
      m_bools.at(ssWWVBS::pass_tFakeVR) = false;
      m_bools.at(ssWWVBS::pass_tEWKVR) = false;
      m_bools.at(ssWWVBS::pass_lllVR) = false;
      m_bools.at(ssWWVBS::pass_ZeeVR) = false;
      m_bools.at(ssWWVBS::pass_DijetsCR) = false;

      m_bools.at(ssWWVBS::EXACTLY_ONE_LEPTON) = false;

      setThresholds(event, sys);

      // Leptons selection for leading two leptons

      for (const xAOD::Electron* ele : *electrons) {
        m_ele_selected.set(*ele, false, sys);
      }

      for (const xAOD::Muon* mu : *muons) {
        m_mu_selected.set(*mu, false, sys);
      }

      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;
      
      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;

      std::vector<std::pair<const xAOD::IParticle*, int>> leptons;
      if (electrons->size() >= 2) {
        leptons.emplace_back(electrons->at(0), -11*electrons->at(0)->charge());
        leptons.emplace_back(electrons->at(1), -11*electrons->at(1)->charge());
      }

      if (muons->size() >= 2) {
        leptons.emplace_back(muons->at(0), -13*muons->at(0)->charge());
        leptons.emplace_back(muons->at(1), -13*muons->at(1)->charge());
      }

      if (electrons->size() == 1 && muons->size() == 1) {
        leptons.emplace_back(electrons->at(0), -11*electrons->at(0)->charge());
        leptons.emplace_back(muons->at(0), -13*muons->at(0)->charge());
      }

      //Add the single lepton events for DijetsCR
      if (electrons->size() == 1 && muons->size() == 0) {
        leptons.emplace_back(electrons->at(0), -11*electrons->at(0)->charge());
      }
      if (electrons->size() == 0 && muons->size() == 1) {
        leptons.emplace_back(muons->at(0), -13*muons->at(0)->charge());
      }

      std::sort(leptons.begin(), leptons.end(),
        [](const std::pair<const xAOD::IParticle*, int>& a,
            const std::pair<const xAOD::IParticle*, int>& b) {
          return a.first->pt() > b.first->pt(); });

      for (size_t i = 0; i < leptons.size() && i < 2; ++i) {
        const xAOD::IParticle* lep = leptons[i].first;
        int id = leptons[i].second;
        if (std::abs(id) == 11) {
        if (!ele0) ele0 = dynamic_cast<const xAOD::Electron*>(lep);
        else       ele1 = dynamic_cast<const xAOD::Electron*>(lep);
        }
        else {
        if (!mu0) mu0 = dynamic_cast<const xAOD::Muon*>(lep);
        else      mu1 = dynamic_cast<const xAOD::Muon*>(lep);
        }
      }
      
      evaluateTriggerCuts(event, ele0, ele1, mu0, mu1, m_ssWWCuts, sys);
      //Prescale trigger SLT used to select the events for Dijest CR, and does not go into the nominal PASS_TRIGGER
      evaluatePrescaleTriggerCuts(event, *electrons, *muons, sys);
      evaulateLeptonIDCuts(ele0, ele1, mu0, mu1, m_ssWWCuts, sys);
      evaluateLeptonCuts(*electrons, *muons, ele0, ele1, mu0, mu1, m_ssWWCuts);
      evaluateMetCuts(met, m_ssWWCuts);
      evaluateJetCuts(*jets, m_ssWWCuts);
      evaluateBJetLeptonCuts(*bjets, *electrons, *muons, m_ssWWCuts);
      
      bool passedall = true;
      for (const auto& [key, value] : m_boolnames) {
        auto it = std::find(m_STANDARD_CUTS.begin(), m_STANDARD_CUTS.end(), value);
        if (it != m_STANDARD_CUTS.end()) {
          passedall &= m_bools.at(key);
        }
      }
      m_passallcuts.set(*event, passedall, sys);

      bool pass_baseline=false;
      if(m_bools.at(ssWWVBS::PASS_TRIGGER) && m_bools.at(ssWWVBS::PASS_TWO_LEPTONS) && m_bools.at(ssWWVBS::DILEPTON_MASS_THRESHOLD)) pass_baseline=true;

      // definition of SR and WZCR events: eee, mmm, mme, eem
      // std::cout<<"     ssWWSelectorAlg::execute()     : line  248"<<std::endl;
      if(pass_baseline && m_bools.at(ssWWVBS::EXACTLY_TWO_LEPTONS) && m_bools.at(ssWWVBS::TWO_SAME_CHARGE_LEPTONS)  ){ 
        if(m_bools.at(ssWWVBS::AT_LEAST_TWO_JETS) && m_bools.at(ssWWVBS::DIJETS_DELTA_RAPIDITY) && m_bools.at(ssWWVBS::DIJETS_MASS_HIGH) && m_bools.at(ssWWVBS::BJET_VETO) ){
          if( m_bools.at(ssWWVBS::MET) ){
              m_bools.at(ssWWVBS::pass_SR)=1;
          }
        }      
      }
      else if (pass_baseline && m_bools.at(ssWWVBS::EXACTLY_THREE_LEPTONS)) {
        // WZ control region (WZCR)
        std::vector<std::pair<TLorentzVector, int>> leptons; // Store 4-momenta and charges

        // Collect electrons 4-momenta and charges
        for (const auto& ele : *electrons) {
            leptons.emplace_back(ele->p4(), ele->charge());
        }

        // Collect muons 4-momenta and charges
        for (const auto& mu : *muons) {
            leptons.emplace_back(mu->p4(), mu->charge());
        }

        if (leptons.size() == 3) {
            // Sort leptons by pT in descending order
            std::sort(leptons.begin(), leptons.end(),
                      [](const std::pair<TLorentzVector, int>& a, const std::pair<TLorentzVector, int>& b) {
                          return a.first.Pt() > b.first.Pt();
                      });
                      //std::cout<<"WZCR:  "<< leptons[0].first.Pt() <<"     "<<leptons[1].first.Pt()<<"     "<<leptons[2].first.Pt()<<std::endl;

            // Calculate the invariant mass of the three leptons and opposite_charges
            TLorentzVector totalP4 = leptons[0].first + leptons[1].first + leptons[2].first;
            double mlll = totalP4.M();
            bool opposite_charges = (leptons[0].second * leptons[1].second < 0) ||
                                    (leptons[0].second * leptons[2].second < 0) ||
                                    (leptons[1].second * leptons[2].second < 0);

            if (mlll > 106. * Athena::Units::GeV && opposite_charges) {
                m_bools.at(ssWWVBS::pass_WZCR) = 1;
            }
        }
      }
      else if (pass_baseline && m_bools.at(ssWWVBS::EXACTLY_TWO_LEPTONS) && !( m_bools.at(ssWWVBS::TWO_SAME_CHARGE_LEPTONS) ) ) {
        if(m_bools.at(ssWWVBS::AT_LEAST_TWO_JETS) && m_bools.at(ssWWVBS::DIJETS_DELTA_RAPIDITY) && m_bools.at(ssWWVBS::DIJETS_MASS_HIGH) && m_bools.at(ssWWVBS::BJET_VETO) ){
              if( m_bools.at(ssWWVBS::MET)){
                    m_bools.at(ssWWVBS::pass_misIDCR) = 1;
              }
            }   
      }
  
      // VRs
      bool pass_ll = false ;
      bool is_eeEvent = false;
      if (electrons->size() == 2) is_eeEvent = true;

      //inclusive VR
      std::vector<TLorentzVector> leptons_p4;

      // Collect all electrons and muons 4-momenta
        for (const auto& ele : *electrons) {
          leptons_p4.push_back(ele->p4());
        }
        for (const auto& mu : *muons) {
          leptons_p4.push_back(mu->p4());
        }

        // Sort leptons by pT in descending order
          std::sort(leptons_p4.begin(), leptons_p4.end(),
                  [](const TLorentzVector& a, const TLorentzVector& b) {
                      return a.Pt() > b.Pt();
                  });

      if ( pass_baseline && m_bools.at(ssWWVBS::EXACTLY_TWO_LEPTONS) ){
          //std::cout<<"incVR:  "<< leptons_p4[0].Pt() <<"     "<<leptons_p4[1].Pt()<<std::endl;
          TLorentzVector totalP4 = leptons_p4[0] + leptons_p4[1];
          double mll = totalP4.M();
          if( mll>20 * Athena::Units::GeV && (mll -Z_mass)>15.* Athena::Units::GeV ){
            pass_ll = true;
          }
          if( pass_ll && m_bools.at(ssWWVBS::TWO_SAME_CHARGE_LEPTONS) ){
                if(!m_bools.at(ssWWVBS::BJET_VETO) ){
                  m_bools.at(ssWWVBS::pass_incVR)=1;
                  // Zee Peak VR: this part only for Zee events
                  if (is_eeEvent){
                    m_bools.at(ssWWVBS::pass_ZeeVR) = 1;
                  }
                  // Low DYjj VR
                  if ( m_bools.at(ssWWVBS::MET) && m_bools.at(ssWWVBS::AT_LEAST_TWO_JETS) ){
                    if( nonbjets->size()>1 && nonbjets->at(0)->pt() > 65*Athena::Units::GeV && nonbjets->at(1)->pt() > 35*Athena::Units::GeV ){
                          mjj = (nonbjets->at(0)->p4() + nonbjets->at(1)->p4()).M();
                          delta_yjj = std::abs((nonbjets->at(0)->p4()).Rapidity() - (nonbjets->at(1)->p4()).Rapidity());
                          if( mjj >= 200*Athena::Units::GeV && delta_yjj <= 2 ){
                              m_bools.at(ssWWVBS::pass_LowDyVR) = 1;
                          }
                      }
                  }
                  if ( m_bools.at(ssWWVBS::MET) && m_bools.at(ssWWVBS::AT_LEAST_TWO_JETS) ){
                    // Low mjj VR
                    if( nonbjets->size()>1 &&  nonbjets->at(0)->pt() > 65*Athena::Units::GeV && nonbjets->at(1)->pt() > 35*Athena::Units::GeV ){
                        mjj = (nonbjets->at(0)->p4() + nonbjets->at(1)->p4()).M();
                        if( mjj <= 500*Athena::Units::GeV ){
                            m_bools.at(ssWWVBS::pass_LowMjjVR) = 1;
                        }
                    }
                  }
                  if ( m_bools.at(ssWWVBS::MET) && !m_bools.at(ssWWVBS::AT_LEAST_TWO_JETS) ){
                    // Low Njet VR
                    if ( nonbjets->size() > 0 ){
                      if(  nonbjets->at(0)->pt() > 35*Athena::Units::GeV ){
                          m_bools.at(ssWWVBS::pass_LowNjVR) = 1;
                      }
                    }
                    else if ( nonbjets->size() == 0 ) m_bools.at(ssWWVBS::pass_LowNjVR) = 1;
                    
                  }
                }
                else if(bjets->size()==1){
                  // Tpo-Fakes VR 
                  if( bjets->size() + nonbjets->size() >= 2 ){
                    if( nonbjets->at(0)->pt() > bjets->at(0)->pt() ){
                      if ( nonbjets->size() >= 2 ){
                          if( jets->at(0)->pt() > 65*Athena::Units::GeV && jets->at(1)->pt() > 35*Athena::Units::GeV ){
                              m_bools.at(ssWWVBS::pass_tFakeVR) = 1; 
                          }
                      }
                      else if (nonbjets->size() == 1){
                        if( jets->at(0)->pt() > 35*Athena::Units::GeV  ){
                          m_bools.at(ssWWVBS::pass_tFakeVR) = 1; 
                        }
                      }
                    }

                  }
                }
                else if( bjets->size()>=2 ){
                  // Top-EWK VR
                  if( nonbjets->size() >= 2 ){
                    if( jets->at(0)->pt() > 65*Athena::Units::GeV && jets->at(1)->pt() > 35*Athena::Units::GeV ){
                      m_bools.at(ssWWVBS::pass_tEWKVR) = 1; 
                    }
                  }
                }

          }
        }
        else if (pass_baseline && m_bools.at(ssWWVBS::EXACTLY_THREE_LEPTONS)) {
          // lll inclusive VR
          if (leptons_p4.size() == 3) {
            //std::cout<<"WZVR:  "<< leptons_p4[0].Pt() <<"     "<<leptons_p4[1].Pt()<<"     "<<leptons_p4[2].Pt()<<std::endl;
            // Calculate the invariant mass of the two leading leptons
            TLorentzVector totalP4 = leptons_p4[0] + leptons_p4[1];
            double mll = totalP4.M();

            // Apply selection cuts
            if (mll > 20 * Athena::Units::GeV &&
                leptons_p4[0].Pt() > 27 * Athena::Units::GeV &&
                leptons_p4[1].Pt() > 27 * Athena::Units::GeV &&
                leptons_p4[2].Pt() > 15 * Athena::Units::GeV) {
                  m_bools.at(ssWWVBS::pass_lllVR) = 1;
            }
          }
        }

      //DijetsCR part used for Fake Factor determination
      // Region requirement: exactly one lepton, at least one jet
      // Collect by the prescale SLT 'HLT_mu14' and 'HLT_e12_lhvloose_nod0_L1EM10VH'
      // |Delta phi(l,j)|>2.8, ET_miss_track+ mT(l,met)<50
      else if (m_bools.at(pass_trigger_prescaleSLT) && m_bools.at(ssWWVBS::EXACTLY_ONE_LEPTON)){
              if (electrons->size()==1 && nonbjets->size()>=1){
                ele0=electrons->at(0);
                float mT_lepMET = std::sqrt(2*ele0->pt()*met->met()*(1-std::cos(ele0->phi()-met->phi())));
		float DPhi_lepjet= std::numbers::pi-std::abs(std::numbers::pi-std::abs(ele0->phi()-nonbjets->at(0)->phi()));
                if (ele0->pt() > 27. * Athena::Units::GeV && nonbjets->at(0)->pt() > 25. * Athena::Units::GeV && DPhi_lepjet>2.8 && (mT_lepMET + met_track->met()) < 50. * Athena::Units::GeV && bjets->size()==0 ){
                  m_bools.at(ssWWVBS::pass_DijetsCR)=1;
                }
              }

              else if (muons->size()==1 && nonbjets->size()>=1){
                mu0=muons->at(0);
                float mT_lepMET = std::sqrt(2*mu0->pt()*met->met()*(1-std::cos(mu0->phi()-met->phi())));
		float DPhi_lepjet= std::numbers::pi-std::abs(std::numbers::pi-std::abs(mu0->phi()-nonbjets->at(0)->phi()));
                if (mu0->pt() > 27. * Athena::Units::GeV && nonbjets->at(0)->pt() > 30. * Athena::Units::GeV && DPhi_lepjet>2.8 && (mT_lepMET + met_track->met()) < 50. * Athena::Units::GeV && bjets->size()==0){
                  m_bools.at(ssWWVBS::pass_DijetsCR)=1;
                }
              }
      }
            
      bool pass = false;
      for(const auto& channel : m_channels){
        if(channel == ssWWVBS::SR){
          pass |= m_bools.at(ssWWVBS::pass_SR);
        }
        else if(channel == ssWWVBS::WZCR){
          pass |= m_bools.at(ssWWVBS::pass_WZCR);
        }
        else if(channel == ssWWVBS::misIDCR){
          pass |= m_bools.at(ssWWVBS::pass_misIDCR);
        }
        else if(channel == ssWWVBS::DijetsCR){
          pass |= m_bools.at(ssWWVBS::pass_DijetsCR);
        }
      }
      

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()=="") {

        // Compute total_events
        m_total_events+=1;

        for (const auto &cut : m_inputCutKeys) {
          if (m_ssWWCuts.exists(m_boolnames.at(cut))) {
            m_ssWWCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_ssWWCuts(m_boolnames.at(cut)).passed) {
              m_ssWWCuts(m_boolnames.at(cut)).counter += 1;
            }
          }
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_ssWWCuts.size(); ++i) {
        if (m_ssWWCuts[i].passed)
          consecutive_cuts++;
        else
          break;
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes.
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_ssWWCuts[i].relativeCounter+=1;
      }

      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      }

      if (!m_bypass && !pass_baseline && !pass) continue;
      filter.setPassed(true);
    }
    return StatusCode::SUCCESS;
  }

  StatusCode ssWWSelectorAlg::finalize()
  {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_ssWWCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_ssWWCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_ssWWCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_ssWWCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_ssWWCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }

    return StatusCode::SUCCESS;
  }

  void ssWWSelectorAlg::evaluateTriggerCuts
  (const xAOD::EventInfo *event,
   const xAOD::Electron* ele0, const xAOD::Electron* ele1,
   const xAOD::Muon* mu0, const xAOD::Muon* mu1,
   CutManager& ssWWCuts, const CP::SystematicSet& sys) {

    if (!ssWWCuts.exists("PASS_TRIGGER"))
        return;

    if (ele0 || mu0) evaluateSingleLeptonTrigger(event, ele0, mu0, sys);
    if (ele1 || mu1) evaluateSingleLeptonTrigger(event, ele1, mu1, sys);

    if (m_bools.at(ssWWVBS::pass_trigger_SLT)) m_bools.at(ssWWVBS::PASS_TRIGGER) = true;
  }

  void ssWWSelectorAlg::evaluateSingleLeptonTrigger
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
      trigPassed_SET &= ele->pt() > m_pt_threshold[ssWWVBS::SLT][ssWWVBS::ele];
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
      trigPassed_SMT &= mu->pt() > m_pt_threshold[ssWWVBS::SLT][ssWWVBS::mu];
    }

    m_bools.at(ssWWVBS::pass_trigger_SLT) |= (trigPassed_SET || trigPassed_SMT);
  }

  void ssWWSelectorAlg::evaulateLeptonIDCuts
  (const xAOD::Electron*& ele0, const xAOD::Electron*& ele1,
   const xAOD::Muon*& mu0, const xAOD::Muon*& mu1,
   CutManager& ssWWCuts, const CP::SystematicSet& sys) {

    if (!ssWWCuts.exists("PASS_LEPTON_ID"))
        return;

    // A helper lambda for processing an electron.
    auto processElectron = [&](const xAOD::Electron* ele) -> const xAOD::Electron* {
      if (ele && m_eleWPDecorHandle.get(*ele, sys) == 1 &&
          ele->pt() > 27. * Athena::Units::GeV &&
          (((std::abs(ele->eta()) < 1.37) || (std::abs(ele->eta()) > 1.52)) &&
           (ele->author() == 1))) {
        return ele;
      }
      return nullptr;
    };
    auto processMuon = [&](const xAOD::Muon* mu) -> const xAOD::Muon* {
      if (mu && m_muonWPDecorHandle.get(*mu, sys) == 1 &&
          mu->pt() > 27. * Athena::Units::GeV) {
        return mu;
      }
      return nullptr;
    };
    auto processDiLepton = [&](const xAOD::Electron* ele0, const xAOD::Electron* ele1) -> bool {
      if (ele0 && ele1) {
        if ((std::abs(ele0->eta()) < 1.37) && (std::abs(ele1->eta()) < 1.37)) {
          return true;
        }
        else {
          return false;
        }
      }
      return false;
    };
    
    // Check the lepton ID and pT requirements;
    ele0 = processElectron(ele0);
    ele1 = processElectron(ele1);
    mu0 = processMuon(mu0);
    mu1 = processMuon(mu1);

    if (ele0 && ele1) {
      m_bools.at(ssWWVBS::PASS_LEPTON_ID) = processDiLepton(ele0, ele1);
      if (m_bools.at(ssWWVBS::PASS_LEPTON_ID)) {
        m_bools.at(ssWWVBS::IS_ee) = true;
      }
      else {
        ele0 = nullptr;
        ele1 = nullptr;
      }
    }
    else if (mu0 && mu1) {
      m_bools.at(ssWWVBS::PASS_LEPTON_ID) = true;
      m_bools.at(ssWWVBS::IS_mm) = true;
    }
    else if (ele0 && mu0) {
      m_bools.at(ssWWVBS::PASS_LEPTON_ID) = true;
      if (ele0->pt() >= mu0->pt()) {
        m_bools.at(ssWWVBS::IS_em) = true;
      }
      else {
        m_bools.at(ssWWVBS::IS_me) = true;
      }
    }
    //Add the single electron condition for DijetsCR
    else if (ele0){
      m_bools.at(ssWWVBS::PASS_LEPTON_ID) = true;
    }
    //Add the single muon condition for DijetsCR
    else if (mu0){
      m_bools.at(ssWWVBS::PASS_LEPTON_ID) = true;
    }
    // Set the boolean flags for the leptons
    if (ele0) {
      m_ele_selected.set(*ele0, true, sys);
    }
    if (ele1) {
      m_ele_selected.set(*ele1, true, sys);
    }
    if (mu0) {
      m_mu_selected.set(*mu0, true, sys);
    }
    if (mu1) {
      m_mu_selected.set(*mu1, true, sys);
    }
  }

  void ssWWSelectorAlg::evaluateLeptonCuts
  (const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons,
    const xAOD::Electron* ele0, const xAOD::Electron* ele1,
    const xAOD::Muon* mu0, const xAOD::Muon* mu1,
   CutManager& ssWWCuts)
  {
    float mZ = 91 * Athena::Units::GeV;
    double mll = -99;
    bool Two_Same_Sign_Leptons = false;

    if (electrons.size() + muons.size() == 1)
      m_bools.at(ssWWVBS::EXACTLY_ONE_LEPTON) = true;

    if (electrons.size() + muons.size() == 2)
      m_bools.at(ssWWVBS::EXACTLY_TWO_LEPTONS) = true;

    if (electrons.size() + muons.size() == 3)
      m_bools.at(ssWWVBS::EXACTLY_THREE_LEPTONS) = true;

    if ((ele0 != nullptr) + (ele1 != nullptr) + (mu0 != nullptr) + (mu1 != nullptr) >= 3)
    {
      throw std::runtime_error("More than 2 leptons in the event");
    }
    if (ele0 && ele1)
    {
      mll = (ele0->p4() + ele1->p4()).M();
      Two_Same_Sign_Leptons = ele0->charge()*ele1->charge() == 1;
    }
    if (mu0 && mu1)
    {
      mll = (mu0->p4() + mu1->p4()).M();
      Two_Same_Sign_Leptons = mu0->charge()*mu1->charge() == 1;
    }
    if (ele0 && mu0)
    {
      mll = (ele0->p4() + mu0->p4()).M();
      Two_Same_Sign_Leptons = ele0->charge()*mu0->charge() == 1;
    }
    
    if(ssWWCuts.exists("TWO_SAME_CHARGE_LEPTONS")) m_bools.at(ssWWVBS::TWO_SAME_CHARGE_LEPTONS) = Two_Same_Sign_Leptons;
    if(ssWWCuts.exists("DILEPTON_MASS_THRESHOLD")) m_bools.at(ssWWVBS::DILEPTON_MASS_THRESHOLD) = ( mll >  20.*Athena::Units::GeV );
    if(ssWWCuts.exists("DILEPTON_MASS_SIDEBAND_EE")) m_bools.at(ssWWVBS::DILEPTON_MASS_SIDEBAND_EE) = ((ele0 && ele1) && std::abs(mll - mZ) > 15.*Athena::Units::GeV);

  }

  void ssWWSelectorAlg::evaluateMetCuts(const xAOD::MissingET* met, CutManager& ssWWCuts){

    if(ssWWCuts.exists("MET")) m_bools.at(ssWWVBS::MET) = (met->met() >= 30 * Athena::Units::GeV);

  }

  void ssWWSelectorAlg::evaluateJetCuts(const xAOD::JetContainer& jets, CutManager& ssWWCuts)
  {

    /// All jets in the containers should have pT>20GeV. Check minPt of your JetSelectorAlg in the ssWW_config file.

    double mjj = -99;
    float delta_yjj = 0;

    if(ssWWCuts.exists("AT_LEAST_TWO_JETS")) m_bools.at(ssWWVBS::AT_LEAST_TWO_JETS) = (jets.size() >= 2 && jets.at(0)->pt() > 65*Athena::Units::GeV && jets.at(1)->pt() > 35*Athena::Units::GeV);
    
    if (jets.size() >= 2){
      mjj = (jets.at(0)->p4() + jets.at(1)->p4()).M();
      delta_yjj = std::abs(jets.at(0)->rapidity() - jets.at(1)->rapidity());
      if(ssWWCuts.exists("DIJETS_MASS_LOW")) m_bools.at(ssWWVBS::DIJETS_MASS_LOW) = (mjj > 200*Athena::Units::GeV);
      if(ssWWCuts.exists("DIJETS_MASS_HIGH")) m_bools.at(ssWWVBS::DIJETS_MASS_HIGH) = (mjj > 500*Athena::Units::GeV);
      if(ssWWCuts.exists("DIJETS_DELTA_RAPIDITY")) m_bools.at(ssWWVBS::DIJETS_DELTA_RAPIDITY) = (delta_yjj > 2);
    }
 
  }

  void ssWWSelectorAlg::evaluateBJetLeptonCuts
  (const ConstDataVector<xAOD::JetContainer>& bjets,
   const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons,
   CutManager& ssWWCuts)
  {
    if(ssWWCuts.exists("PASS_TWO_LEPTONS")) m_bools.at(ssWWVBS::PASS_TWO_LEPTONS) = (electrons.size() + muons.size() >= 2);
    if(ssWWCuts.exists("PASS_THREE_LEPTONS")) m_bools.at(ssWWVBS::PASS_THREE_LEPTONS) = (electrons.size() + muons.size() >= 3);
    if(ssWWCuts.exists("BJET_VETO")) m_bools.at(ssWWVBS::BJET_VETO) = (bjets.size() == 0);
  }  

  void ssWWSelectorAlg::setThresholds(const xAOD::EventInfo* event,
					const CP::SystematicSet& sys) {
    
    int year = m_year.get(*event, sys);

    // Single-lepton triggers
    if(year==2015)
      m_pt_threshold[ssWWVBS::SLT][ssWWVBS::ele] = 25. * Athena::Units::GeV;
    // 2022 75 bunches
    else if(m_is22_75bunches.get(*event, sys))
      m_pt_threshold[ssWWVBS::SLT][ssWWVBS::ele] = 18. * Athena::Units::GeV;
    else
      m_pt_threshold[ssWWVBS::SLT][ssWWVBS::ele] = 27. * Athena::Units::GeV;

    if(year==2015)
      m_pt_threshold[ssWWVBS::SLT][ssWWVBS::mu] = 21. * Athena::Units::GeV;
    else if(year>=2016 && year<=2018)
      m_pt_threshold[ssWWVBS::SLT][ssWWVBS::mu] = 27. * Athena::Units::GeV;
    else
      m_pt_threshold[ssWWVBS::SLT][ssWWVBS::mu] = 25. * Athena::Units::GeV;

  }

  StatusCode ssWWSelectorAlg::initialiseCutflow(){

    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_ssWWCuts.CheckInputCutList(m_inputCutList, boolnameslist);

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
      m_ssWWCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_ssWWCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of ssWW VBS cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of ssWW VBS cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of ssWW VBS cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }

  //Only give this prescale SLT to one lepton events
    void ssWWSelectorAlg::evaluatePrescaleTriggerCuts(const xAOD::EventInfo *event, const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons, const CP::SystematicSet& sys){
      std::vector<std::string> prescale_trigger_paths;
      int year=m_year.get(*event,sys);
      bool Pass_Prescale_Trig = false;
      if (year==2015 || year==2016){
          if (electrons.size()==1 && muons.size()==0 ){
              const xAOD::Electron* ele_preTrig=nullptr;
              ele_preTrig=electrons.at(0);
              prescale_trigger_paths={
              "HLT_e12_lhvloose_nod0_L1EM10VH",
              };
              for (const auto& prescale_trigger : prescale_trigger_paths){
                  bool pass=m_triggerdecos.at("trigPassed_"+prescale_trigger).get(*event,sys);
                  if (pass){
                      bool match = m_matchingTool->match(*ele_preTrig,prescale_trigger);
                      Pass_Prescale_Trig |= match;
                  }
              }
              //Here could add a threshold cut for the lepton pass the Prescale SLT
              //Pass_Prescale_Trig &= (ele_preTrig->pt() > 12. * Athena::Units::GeV; )
          }

          else if(electrons.size()==0 && muons.size()==1){
              const xAOD::Muon* muon_preTrig=nullptr;
              muon_preTrig=muons.at(0);
              prescale_trigger_paths={
              "HLT_mu14",
              };
              for (const auto& prescale_trigger : prescale_trigger_paths ){
                  bool pass=m_triggerdecos.at("trigPassed_"+prescale_trigger).get(*event,sys);
                  if (pass){
                      bool match = m_matchingTool->match(*muon_preTrig,prescale_trigger);
                      Pass_Prescale_Trig |= match;
                  }
              }
              //Here could add a threshold cut for the lepton pass the Prescale SLT
              //Pass_Prescale_Trig &= (muon_preTrig->pt() > 14. * Athena::Units::GeV; )
          }
      }
      m_bools.at(ssWWVBS::pass_trigger_prescaleSLT)=Pass_Prescale_Trig;
  }

}

