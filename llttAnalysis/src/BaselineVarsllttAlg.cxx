/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "BaselineVarsllttAlg.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"

namespace HLLTT
{
  BaselineVarsllttAlg::BaselineVarsllttAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsllttAlg::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_mmc_status.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_types.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    if(m_isMC){
      m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

    if(m_isMC){
      m_tau_effSF = CP::SysReadDecorHandle<float>("tau_effSF_"+m_tauWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_tau_effSF.initialize(m_systematicsList, m_tauHandle, SG::AllowEmpty));

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_tauHandle));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    for (const std::string &var : m_floatVariables){
      ATH_MSG_DEBUG("initializing float variable: " << var);
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK(m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    for (const std::string &var : m_intVariables){
      ATH_MSG_DEBUG("initializing integer variable: " << var);
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsllttAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
	ATH_MSG_ERROR("Could not retrieve MET");
	return StatusCode::FAILURE;	
      }

      for (const auto& var: m_floatVariables) {
          m_Fbranches.at(var).set(*event, -99, sys);
      }

      for (const auto& var: m_intVariables) {
          m_Ibranches.at(var).set(*event, -99, sys);
      }

      // selected leptons ;
      int n_ele = 0;
      int n_muo = 0;
      int n_lep = 0;
      int n_jets = 0;
      int n_bjets = 0; 
      TLorentzVector p4lep[4];
      int lepid[4];
      float lepsf[4]; 

      for(const xAOD::Muon* muon : *muons) {
	if (n_lep==4) break;
        if (m_selected_mu.get(*muon, sys)){
	  if(n_lep<4){
            p4lep[n_lep] = muon->p4();
            lepid[n_lep] = muon->charge()>0?-13: 13;
	    if(m_isMC)lepsf[n_lep] = m_mu_SF.get(*muon, sys);
            ++n_lep;
	    ++n_muo;
	  }
	}
      }

      for(const xAOD::Electron* electron : *electrons) {
        if (m_selected_el.get(*electron, sys)){
	  if(n_lep<4){
	    p4lep[n_lep] = electron->p4();
	    lepid[n_lep] = electron->charge()>0? -11:11;
	    if(m_isMC)lepsf[n_lep] = m_ele_SF.get(*electron, sys);
	    ++n_lep;
	  }
	  ++n_ele;
	}
      }

      // selecting taus
      int n_taus = 0;
      TLorentzVector lead_tau;
      TLorentzVector sublead_tau;
      int lead_tau_id = 0;
      int sublead_tau_id = 0;
      float lead_tau_sf(1.);
      float sublead_tau_sf(1.);

      for(const xAOD::TauJet* tau : *taus) {
        if (m_selected_tau.get(*tau, sys)){
	  ++n_taus; 
	  if (lead_tau_id == 0){
	    lead_tau_id = tau->charge()>0?-15:15;
	    lead_tau = tau->p4();
	    if(m_isMC)lead_tau_sf = m_tau_effSF.get(*tau,sys);
	  }
	  else if(sublead_tau_id == 0){
            sublead_tau_id = tau->charge()>0?-15:15;
	    sublead_tau = tau->p4();
	    if(m_isMC)sublead_tau_sf = m_tau_effSF.get(*tau,sys);
	  }
	}
      }
      //************                                                                                                                                  
      // jet                                                                                                                                          
      //************                                                                                                                                  
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(SG::VIEW_ELEMENTS);
      for (const xAOD::Jet *jet : *jets)
	{
	  n_jets += 1;
	  if (WPgiven && m_isBtag.get(*jet, sys))bjets->push_back(jet);
	}
      n_bjets = bjets->size();
      
      // event level info 
      TLorentzVector p4nu;
      p4nu.SetPtEtaPhiM(met->met(),0.,met->phi(),0);
      int iamu1(-1);
      int iamu2(-1);
      float apt(0);
      float drmin(99.);
      // alternative combination with subleading lepton
      int iamu1x(-1);
      int iamu2x(-1);
      float aptx(0);
      float drminx(99.);
      // find the leading lepton to decide ee or mumu OS 
      // 1 = mumu - 2 = ee - 3 = emu  Positive is SS, negative is OS
      int dil_type(0);
      // isr: index to save event type
      // 0 = OS pair only
      // 1 = OS pair + OS/SS lepton pair
      // 2 = OS pair + OS/SS lepton-tau pair
      // 3 = OS pair + OS/SS tau-tau pair
      int isr(-1);
      // recid=0, default for a->mumu by taking the leading lepton;
      // recid=1 for doing second pass to select the correct muon pair 
      int recid(0); 
      int iatau1(-1);
      int iatau2(-1);
      // ditau_type: Index to save event type
      // 1 = mumu - 2 = ee - 3 = emu
      // 4 = mutau - 5 = etau - 6 = tautau
      // Positive is SS, negative is OS
      int ditau_type(0);
      int osatt(0);      
      float mll(0);
      float ptll(0);
      float drll(0); 
      float dphimetll(0);
      float dratt(0);
      float dphimetatt(0);
      float maa(0);
      float ptaa(0);
      float draa(0);
      float matt(0);
      float ptatt(0);
      int   ditau_index(0);
      float mmc_maa(0);
      float mmc_ptaa(0);
      float mmc_draa(0);
      int mmc_types(-1);   
      
      if( n_lep > 1 ){
	// select leading lepton and close lepton nearby unbiased search 
	
	for(int i = 0; i<n_lep; ++i){
	  if(p4lep[i].Pt()>apt){ 
	    iamu1 = i; 
	    apt = p4lep[i].Pt();
	  }
	}
	// Select OS muon pair with minidr, for higher ma, need to revisit
	// Muons are stored as the first elements in the lepton container
        for(int i = 0; i<n_lep; ++i){
	  if(iamu1 !=i){
	    float dr = p4lep[iamu1].DeltaR(p4lep[i]);
	    if( dr<drmin){
	      iamu2 = i;
	      drmin = dr;
	    }
	  }
        }
	//alternative combination with subleading lepton
	if(n_lep>2){
          for(int i = 0; i<n_lep; ++i){
            if(i!=iamu1&&i!=iamu2&&p4lep[i].Pt()>aptx){
              iamu1x = i;
              aptx = p4lep[i].Pt();
            }
          }
          for(int i = 0; i<n_lep; ++i){
            if(iamu1x !=i){
              float dr = p4lep[iamu1x].DeltaR(p4lep[i]);
              if( dr<drminx){
                iamu2x = i;
                drminx = dr;
              }
            }
          }
	  //selecting with smaller dr
	  if(drmin>1.5||abs(lepid[iamu1])!=13||abs(lepid[iamu2])!=13){
	    ATH_MSG_DEBUG("Atternative pair of leptons selected event: "<<event->eventNumber()<<" default iamu1="<<iamu1<<" iamu2="<<iamu2<<" drmin="<<drmin
			    <<" alternative iamu1x="<<iamu1x<<" iamu2x="<<iamu2x<<" drminx="<<drminx<<" nlep="<<n_lep);
	    if(p4lep[iamu2x].Pt()>aptx){
	      iamu1 = iamu2x;
	      iamu2 = iamu1x;
	    }
	    else{
	      iamu1 = iamu1x;
	      iamu2 = iamu2x;
	    }
            drmin = drminx;
	    recid=1;
          }
        }
        if(iamu2>-1){
	  isr = 0;
	  ATH_MSG_DEBUG(" Reconstructed isr:"<<isr<<" iamu1:"<<iamu1<<" iamu2:"<<iamu2);
	  if(abs(lepid[iamu1])==13&&abs(lepid[iamu2])==13)dil_type=1;
	  else if(abs(lepid[iamu1])==11&&abs(lepid[iamu2])==11)dil_type=2;
	  else if(abs(lepid[iamu1])!=abs(lepid[iamu2]))dil_type=3;
	  if(lepid[iamu1]*lepid[iamu2]<0)dil_type *= -1;
	  TLorentzVector p4amu = p4lep[iamu1]+p4lep[iamu2];
          mll = p4amu.M();
          ptll = p4amu.Pt();
          drll = p4lep[iamu1].DeltaR(p4lep[iamu2]);
	  dphimetll = p4amu.DeltaPhi(p4nu);
	  // dilep + ditau:lep-lep
	  if(n_lep ==4){
	    // Select the a->tautau leptons as keep both OS, SS pair
            iatau1 = -1;
            iatau2 = -1;
            for(int j = 0; j<n_lep; ++j){
              if(j !=iamu1 && j !=iamu2){
                for( int k = j+1; k<n_lep; ++k){
                  if(k !=iamu1 && k!=iamu2){
                    iatau1 = j;
                    iatau2 = k;
                  }
                }
              }
            }
            if(iatau1>-1 && iatau2 >-1){
              isr = 1;
	      ditau_type = 3;
	      if(abs(lepid[iatau1])==13&&abs(lepid[iatau2])==13)
		ditau_type = 1; 
	      else if(abs(lepid[iatau1])==11&&abs(lepid[iatau2])==11)
		ditau_type = 2;
	      ATH_MSG_DEBUG(" Reconstructed isr:"<<isr<<" iatau1:"<<iatau1<<" iatau2:"<<iatau2<<" ditau type:"<<ditau_type);
	      osatt = lepid[iatau1]*lepid[iatau2]>0?ditau_type:-ditau_type;
	      TLorentzVector p4atau = p4lep[iatau1]+p4lep[iatau2];
              matt = p4atau.M();
              ptatt = p4atau.Pt();
              dratt = p4lep[iatau1].DeltaR(p4lep[iatau2]);
	      dphimetatt = p4atau.DeltaPhi(p4nu);
              maa = (p4amu+p4atau).M();
	      ptaa = (p4amu+p4atau).Pt();
              draa = p4amu.DeltaR(p4atau);
	    }
	  }
	  // dilep + ditau lep-had
	  if(n_lep==3 && n_taus ==1){
            iatau1 = -1;
            for(int j = 0; j<n_lep; ++j){
              if(j !=iamu1 && j !=iamu2){
                iatau1 = j;
              }
            }
            if(iatau1>-1){
              isr = 2;
	      iatau2 = 0;
	      ditau_type = abs(lepid[iatau1])==13?4:5;
	      ATH_MSG_DEBUG(" Reconstructed isr:"<<isr<<" iatau1:"<<iatau1<<" iatau2:"<<iatau2<<" ditau type:"<<ditau_type);
              osatt = lepid[iatau1]*lead_tau_id>0?ditau_type:-ditau_type;
	      TLorentzVector p4atau = p4lep[iatau1]+lead_tau;
              matt = p4atau.M();
              ptatt = p4atau.Pt();
              dratt = p4lep[iatau1].DeltaR(lead_tau);
	      dphimetatt = p4atau.DeltaPhi(p4nu);
              maa = (p4amu+p4atau).M();
	      ptaa = (p4amu+p4atau).Pt();
              draa = p4amu.DeltaR(p4atau);
            }
          }
	  // dilep + ditau had-had
	  if(n_lep==2&&n_taus ==2){
            isr = 3;
	    iatau1 = 0; 
	    iatau2 = 1;
	    ATH_MSG_DEBUG(" Reconstructed isr:"<<isr<<" iatau1:"<<iatau1<<" iatau2:"<<iatau2<<" ditau type:"<<"hadhad");
            osatt = lead_tau_id*sublead_tau_id>0?6:-6;	    
	    TLorentzVector p4atau = lead_tau + sublead_tau;
            matt = p4atau.M();
            ptatt = p4atau.Pt();
            dratt = lead_tau.DeltaR(sublead_tau);
	    dphimetatt = p4atau.DeltaPhi(p4nu);
            maa = (p4amu+p4atau).M();
	    ptaa = (p4amu+p4atau).Pt();
            draa = p4amu.DeltaR(p4atau);
          }
        }
      }
      // save stuff here:  
      m_Ibranches.at("isr").set(*event, isr, sys);
      m_Ibranches.at("recid").set(*event, recid, sys);
      m_Ibranches.at("nlep").set(*event, n_lep, sys);      
      m_Ibranches.at("nmuo").set(*event, n_muo, sys);
      m_Ibranches.at("nele").set(*event, n_ele, sys);
      m_Ibranches.at("ntaus").set(*event, n_taus, sys);
      m_Ibranches.at("njets").set(*event, n_jets, sys);
      m_Ibranches.at("nbjets").set(*event, n_bjets, sys);
      m_Ibranches.at("diltype").set(*event, dil_type, sys);
      ATH_MSG_DEBUG(" Saving isr:"<<isr<<" nlep:"<<n_lep<<" nmuo:"<<n_muo<<" nele:"<<n_ele<<" ntaus:"<<n_taus);
      if(isr>0){ 	
	m_Fbranches.at("Lepton1_pt").set(*event, p4lep[iamu1].Pt(), sys);
        m_Fbranches.at("Lepton1_eta").set(*event, p4lep[iamu1].Eta(), sys);
        m_Fbranches.at("Lepton1_phi").set(*event, p4lep[iamu1].Phi(), sys);
	if(m_isMC)m_Fbranches.at("Lepton1_effSF").set(*event, lepsf[iamu1], sys);	
        m_Ibranches.at("Lepton1_pdgid").set(*event, lepid[iamu1], sys);
	
        m_Fbranches.at("Lepton2_pt").set(*event, p4lep[iamu2].Pt(), sys);
        m_Fbranches.at("Lepton2_eta").set(*event, p4lep[iamu2].Eta(), sys);
        m_Fbranches.at("Lepton2_phi").set(*event, p4lep[iamu2].Phi(), sys);
	if(m_isMC)m_Fbranches.at("Lepton2_effSF").set(*event, lepsf[iamu2], sys);
        m_Ibranches.at("Lepton2_pdgid").set(*event, lepid[iamu2], sys);
      
	TLorentzVector p4LeadTau;
	TLorentzVector p4SubleadTau;
	int leadTau_pdgId;
	int subleadTau_pdgId;
	float leadTau_sf(1);
	float subleadTau_sf(1);
	switch(isr){
	case 1:
	  p4LeadTau = p4lep[iatau1];
	  p4SubleadTau = p4lep[iatau2];
	  leadTau_pdgId = lepid[iatau1];
	  subleadTau_pdgId = lepid[iatau2];
	  if(m_isMC){	    
	    leadTau_sf = lepsf[iatau1];
	    subleadTau_sf = lepsf[iatau2];
	  }
	  break;
	case 2:
	  p4LeadTau = p4lep[iatau1];
	  p4SubleadTau = lead_tau;
	  leadTau_pdgId = lepid[iatau1];
	  subleadTau_pdgId = lead_tau_id;
	  if(m_isMC){
	    leadTau_sf = lepsf[iatau1];
	    subleadTau_sf = lead_tau_sf;
	  }
	  break;
	case 3:
	  p4LeadTau = lead_tau;
	  p4SubleadTau = sublead_tau;
	  leadTau_pdgId = lead_tau_id;
	  subleadTau_pdgId = sublead_tau_id;
	  if(m_isMC){
	    leadTau_sf = lead_tau_sf;
	    subleadTau_sf = sublead_tau_sf;
	  }
	  break;
	}
	
	m_Fbranches.at("Tau1_pt").set(*event, p4LeadTau.Pt(), sys);
	m_Fbranches.at("Tau1_eta").set(*event, p4LeadTau.Eta(), sys);
	m_Fbranches.at("Tau1_phi").set(*event, p4LeadTau.Phi(), sys);
	m_Fbranches.at("Tau1_E").set(*event, p4LeadTau.E(), sys);
	if(m_isMC)m_Fbranches.at("Tau1_effSF").set(*event, leadTau_sf, sys);
	m_Ibranches.at("Tau1_pdgid").set(*event, leadTau_pdgId, sys);

        m_Fbranches.at("Tau2_pt").set(*event, p4SubleadTau.Pt(), sys);
        m_Fbranches.at("Tau2_eta").set(*event, p4SubleadTau.Eta(), sys);
        m_Fbranches.at("Tau2_phi").set(*event, p4SubleadTau.Phi(), sys);
        m_Fbranches.at("Tau2_E").set(*event, p4SubleadTau.E(), sys);
	if(m_isMC)m_Fbranches.at("Tau2_effSF").set(*event, subleadTau_sf, sys);
        m_Ibranches.at("Tau2_pdgid").set(*event, subleadTau_pdgId, sys);

	m_Fbranches.at("mll").set(*event, mll, sys);
	m_Fbranches.at("ptll").set(*event, ptll, sys);
	m_Fbranches.at("drll").set(*event, drll, sys);
	m_Fbranches.at("dphimetll").set(*event, dphimetll, sys);
	m_Fbranches.at("matt").set(*event, matt, sys);
	m_Fbranches.at("ptatt").set(*event, ptatt, sys);
	m_Fbranches.at("dratt").set(*event, dratt, sys);
	m_Fbranches.at("maa").set(*event, maa, sys);
	m_Fbranches.at("ptaa").set(*event, ptaa, sys);
	m_Fbranches.at("draa").set(*event, draa, sys);
        m_Fbranches.at("dphimetatt").set(*event, dphimetatt, sys);
	m_Ibranches.at("osatt").set(*event, osatt, sys);
	ditau_index = iatau1 + 10*iatau2 + 100*isr;
	// mmc 
	TLorentzVector mmc_vec(0,0,0,0);
	mmc_vec.SetPtEtaPhiM(m_mmc_pt.get(*event, sys),
			     m_mmc_eta.get(*event, sys),
			     m_mmc_phi.get(*event, sys),
			     m_mmc_m.get(*event, sys));
	mmc_maa = (p4lep[iamu1]+p4lep[iamu2]+mmc_vec).M();
	mmc_ptaa = (p4lep[iamu1]+p4lep[iamu2]+mmc_vec).Pt();
	mmc_draa = (p4lep[iamu1]+p4lep[iamu2]).DeltaR(mmc_vec);
	mmc_types = m_mmc_types.get(*event, sys);
	if(ditau_index != mmc_types){ 
	  ATH_MSG_WARNING("Ditau idexes in MMC do not match with selected: ditau_indexes "
			  << ditau_index<<", mmc_types "<<mmc_types);
	}
	if(mmc_types>0)ATH_MSG_DEBUG(" mmc dump: event "<<event->eventNumber()<<" status "<<m_mmc_status.get(*event, sys)<<" mmc pt "
			<<m_mmc_pt.get(*event, sys)<<" mmc eta "<<m_mmc_eta.get(*event, sys)<<" mmc phi "<<m_mmc_phi.get(*event, sys)
			<<" mmc m "<<m_mmc_m.get(*event, sys)<<" mmc maa "<<mmc_maa<<" mmc ptaa "<<mmc_ptaa<<" mmc draa "<<mmc_draa
			<<" maa "<<maa<<" ptaa "<<ptaa<<" draa "<<draa);
	m_Fbranches.at("mmc_maa").set(*event, mmc_maa, sys);
	m_Fbranches.at("mmc_ptaa").set(*event, mmc_ptaa, sys);
	m_Fbranches.at("mmc_draa").set(*event, mmc_draa, sys);
      }
    }

    return StatusCode::SUCCESS;
  }
}
