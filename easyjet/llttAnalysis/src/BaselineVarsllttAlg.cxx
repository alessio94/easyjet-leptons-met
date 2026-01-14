/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "BaselineVarsllttAlg.h"
#include <AthContainers/ConstDataVector.h>
#include "TauAnalysisTools/HelperFunctions.h"

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
    ATH_CHECK (m_llttJetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_llttTauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_llttElectronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_llttMuonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_mmc_status.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_types.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    if(m_isMC){
      ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
      m_ele_SF = CP::SysReadDecorHandle<float>("effSF_"+m_eleWPName+"_%SYS%", this);
      ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));

      ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
      m_mu_SF = CP::SysReadDecorHandle<float>("effSF_"+m_muWPName+"_%SYS%", this);
      ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle));

      ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
      m_tau_effSF = CP::SysReadDecorHandle<float>("effSF_"+m_tauWPName+"_%SYS%", this);
      ATH_CHECK (m_tau_effSF.initialize(m_systematicsList, m_tauHandle, SG::AllowEmpty));
    }

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_llttElectronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_llttMuonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_llttTauHandle));
    ATH_CHECK (m_istauID.initialize(m_systematicsList, m_llttTauHandle));

    ATH_CHECK (m_selected_el_amm.initialize(m_systematicsList, m_llttElectronHandle));
    ATH_CHECK (m_selected_mu_amm.initialize(m_systematicsList, m_llttMuonHandle));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_llttJetHandle));
    }
    if (m_isMC){
      ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_llttJetHandle));
      ATH_CHECK (m_ele_truthOrigin.initialize(m_systematicsList, m_llttElectronHandle));
      ATH_CHECK (m_ele_truthType.initialize(m_systematicsList, m_llttElectronHandle));
      ATH_CHECK (m_mu_truthOrigin.initialize(m_systematicsList, m_llttMuonHandle));
      ATH_CHECK (m_mu_truthType.initialize(m_systematicsList, m_llttMuonHandle));
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
      ANA_CHECK (m_llttJetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_llttMuonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_llttElectronHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_llttTauHandle.retrieve (taus, sys));

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
      int leptruthorig[4];
      int leptruthtype[4];
      int iamu1(-1);
      int iamu2(-1);

      for(const xAOD::Muon* muon : *muons) {
	if (n_lep==4) break;
        if (m_selected_mu.get(*muon, sys)){
	  if(n_lep<4){
            p4lep[n_lep] = muon->p4();
            lepid[n_lep] = muon->charge()>0?-13: 13;
	    if(m_isMC){
	      lepsf[n_lep] = m_mu_SF.get(*muon, sys);
	      leptruthorig[n_lep] = m_mu_truthOrigin.get(*muon,sys);
	      leptruthtype[n_lep] = m_mu_truthType.get(*muon,sys);
	    }
	    // selecting which muon is from a->mumu
	    if(m_selected_mu_amm.get(*muon, sys)){
	      if(iamu1==-1)iamu1 = n_lep;
	      else if(iamu2==-1) iamu2 = n_lep;
	    }
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
            if(m_isMC){
              lepsf[n_lep] = m_ele_SF.get(*electron, sys);
	      leptruthorig[n_lep] = m_ele_truthOrigin.get(*electron,sys);
              leptruthtype[n_lep] = m_ele_truthType.get(*electron,sys);
	    }
	    // selecting which muon is from a->mumu
            if(m_selected_el_amm.get(*electron, sys)){
              if(iamu1==-1)iamu1 = n_lep;
              else if(iamu2==-1) iamu2 = n_lep;	
            }
            ++n_lep;
          }
          ++n_ele;
        }
      }

      // selecting taus
      int n_taus = 0;
      const xAOD::TauJet* tau0 = nullptr;
      const xAOD::TauJet* tau1 = nullptr;
      
      for(const xAOD::TauJet* tau : *taus) {
        if (m_selected_tau.get(*tau, sys)){
	  ++n_taus;
	  if (!tau0) tau0 = tau;
	  else if(!tau1) tau1 = tau;
	}
      }
      //************
      // jet
      //************
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(SG::VIEW_ELEMENTS);
      for (const xAOD::Jet* jet : *jets)
	{	  
	  if(std::abs(jet->eta())<2.5){
	    n_jets += 1;
	    if (WPgiven && m_isBtag.get(*jet, sys))
	      bjets->push_back(jet);
	  }
	}
      n_bjets = bjets->size();
      // event level info 
      TLorentzVector p4nu;
      p4nu.SetPtEtaPhiM(met->met(),0.,met->phi(),0);
      // alternative combination with subleading lepton
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
      
      mmc_types = m_mmc_types.get(*event, sys);
      // decode isr and recid from reconstruction types 
      if(mmc_types>1000)recid=1;
      
      if( n_lep > 1 ){
        if(iamu1>-1&&iamu2>-1){
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
	      osatt = (lepid[iatau1]*lepid[iatau2])>0?ditau_type:-ditau_type;
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
	  if(n_lep==3 && n_taus >=1){
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
              osatt = (lepid[iatau1]*(-tau0->charge()))>0?ditau_type:-ditau_type;
	      TLorentzVector p4atau = p4lep[iatau1]+tau0->p4();
              matt = p4atau.M();
              ptatt = p4atau.Pt();
              dratt = p4lep[iatau1].DeltaR(tau0->p4());
	      dphimetatt = p4atau.DeltaPhi(p4nu);
              maa = (p4amu+p4atau).M();
	      ptaa = (p4amu+p4atau).Pt();
              draa = p4amu.DeltaR(p4atau);
            }
          }
	  // dilep + ditau had-had
	  if(n_lep==2&&n_taus >=2){
            isr = 3;
	    iatau1 = 0; 
	    iatau2 = 1;
	    ATH_MSG_DEBUG(" Reconstructed isr:"<<isr<<" iatau1:"<<iatau1<<" iatau2:"<<iatau2<<" ditau type:"<<"hadhad");
            osatt = (tau0->charge()*tau1->charge())>0?6:-6;	    
	    TLorentzVector p4atau = tau0->p4() + tau1->p4();
            matt = p4atau.M();
            ptatt = p4atau.Pt();
            dratt = (tau0->p4()).DeltaR(tau1->p4());
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
	if(m_isMC){
	  m_Ibranches.at("Lepton1_truthOrig").set(*event, leptruthorig[iamu1], sys);
          m_Ibranches.at("Lepton1_truthType").set(*event, leptruthtype[iamu1], sys);
	}	  	
        m_Fbranches.at("Lepton2_pt").set(*event, p4lep[iamu2].Pt(), sys);
        m_Fbranches.at("Lepton2_eta").set(*event, p4lep[iamu2].Eta(), sys);
        m_Fbranches.at("Lepton2_phi").set(*event, p4lep[iamu2].Phi(), sys);
	if(m_isMC)m_Fbranches.at("Lepton2_effSF").set(*event, lepsf[iamu2], sys);
        m_Ibranches.at("Lepton2_pdgid").set(*event, lepid[iamu2], sys);
	if(m_isMC){
          m_Ibranches.at("Lepton2_truthOrig").set(*event, leptruthorig[iamu2], sys);
          m_Ibranches.at("Lepton2_truthType").set(*event, leptruthtype[iamu2], sys);
	}
	std::vector<TLorentzVector> p4Tau;
	std::vector<int> tau_pdgId;
	std::vector<float> tau_sf;
	std::vector<int> tau_truthorig;
        std::vector<int> tau_truthtype;	

	const xAOD::TauJet* Tau1 = nullptr;
	const xAOD::TauJet* Tau2 = nullptr;
	static const SG::AuxElement::ConstAccessor<int> acc_PartonTruthLabelID("PartonTruthLabelID");
	switch(isr){	    
	case 1:
	  for(const int iatau : {iatau1, iatau2}){
	    p4Tau.push_back(p4lep[iatau]);
	    tau_pdgId.push_back(lepid[iatau]);
	    if(m_isMC){	    
	      tau_sf.push_back(lepsf[iatau]);
	      tau_truthorig.push_back(leptruthorig[iatau]);
	      tau_truthtype.push_back(leptruthtype[iatau]);
	    }
	  }
	  break;
	case 2:
	  p4Tau.push_back(p4lep[iatau1]);
	  tau_pdgId.push_back(lepid[iatau1]);
	  if(m_isMC){	    
	    tau_sf.push_back(lepsf[iatau1]);
	    tau_truthorig.push_back(leptruthorig[iatau1]);
	    tau_truthtype.push_back(leptruthtype[iatau1]);
	  }

	  p4Tau.push_back(tau0->p4());
	  tau_pdgId.push_back(tau0->charge()>0?-15:15);
	  if(m_isMC){
	    tau_sf.push_back(m_tau_effSF.get(*tau0,sys));
	    const xAOD::Jet *truthJet = xAOD::TauHelpers::getLink<xAOD::Jet>(tau0, "truthJetLink");
            tau_truthorig.push_back((truthJet) ? acc_PartonTruthLabelID(*truthJet): -99);
	    tau_truthtype.push_back(TauAnalysisTools::getTruthParticleType(*tau0));
	  }
	  Tau2 = tau0;
	  break;
	case 3:
	  for (const auto& tau : {tau0, tau1}){
	    p4Tau.push_back(tau->p4());
	    tau_pdgId.push_back(tau->charge()>0?-15:15);
	    if(m_isMC){
	      tau_sf.push_back(m_tau_effSF.get(*tau,sys));
	      const xAOD::Jet *truthJet = xAOD::TauHelpers::getLink<xAOD::Jet>(tau, "truthJetLink");
              tau_truthorig.push_back((truthJet) ? acc_PartonTruthLabelID(*truthJet): -99);
	      tau_truthtype.push_back(TauAnalysisTools::getTruthParticleType(*tau0));
	    }
	  }
	  Tau1 = tau0;
	  Tau2 = tau1;
	  break;  
	default:
	  ATH_MSG_ERROR("Unknown isr "<<isr);
	  return StatusCode::FAILURE;
	}
	for(unsigned int i=0; i<2; i++){
	  std::string prefix = "Tau"+std::to_string(i+1);
	  m_Fbranches.at(prefix+"_pt").set(*event, p4Tau[i].Pt(), sys);
	  m_Fbranches.at(prefix+"_eta").set(*event, p4Tau[i].Eta(), sys);
	  m_Fbranches.at(prefix+"_phi").set(*event, p4Tau[i].Phi(), sys);
	  m_Fbranches.at(prefix+"_E").set(*event, p4Tau[i].E(), sys);
	  m_Ibranches.at(prefix+"_pdgid").set(*event, tau_pdgId[i], sys);
	  m_Ibranches.at(prefix+"_charge").set(*event, tau_pdgId[i]>0?-1:1, sys);
	  int decayMode=-1;
	  int tau_EleRNN_WP = 0;
	  const xAOD::TauJet* tau = i==0 ? Tau1 : Tau2;
	  if(tau){
	    m_Fbranches.at(prefix+"_RNN").set(*event, tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans), sys);
	    m_Ibranches.at(prefix+"_charge").set(*event, tau->charge(), sys);
	    m_Ibranches.at(prefix+"_nProng").set(*event, tau->nTracks(), sys);
	    tau->panTauDetail(xAOD::TauJetParameters::PanTau_DecayMode, decayMode);
	    m_Ibranches.at(prefix+"_decayMode").set(*event, decayMode, sys);
	    if(tau->isTau(xAOD::TauJetParameters::EleRNNTight)) tau_EleRNN_WP = 3;
	    else if(tau->isTau(xAOD::TauJetParameters::EleRNNMedium)) tau_EleRNN_WP = 2;
	    else if(tau->isTau(xAOD::TauJetParameters::EleRNNLoose)) tau_EleRNN_WP = 1;
	    tau_EleRNN_WP+=m_istauID.get(*tau, sys)*10;
	    m_Ibranches.at(prefix+"_EleRNN_WP").set(*event, tau_EleRNN_WP, sys);
	  }
	  if(m_isMC){
	    m_Fbranches.at(prefix+"_effSF").set(*event, tau_sf[i], sys);
	    m_Ibranches.at(prefix+"_truthOrig").set(*event, tau_truthorig[i], sys);
	    m_Ibranches.at(prefix+"_truthType").set(*event, tau_truthtype[i], sys);
	    if(tau){
	      ATH_MSG_DEBUG("Dump tau truthType: event  "<<event->eventNumber()<<" isr "<<isr<<" Tau"<<i+1<<" pt "<< tau->pt()<<" nProng "<<tau->nTracks()
			  <<" truthType decoded "<<tau_truthtype[i]<<" truthType "<<int(TauAnalysisTools::getTruthParticleType(*tau)));
	    }
	  }
        }
	////////
	if(msgLvl(MSG::DEBUG)&&abs(osatt)==4){ // mu+tau final state
	  ATH_MSG_DEBUG("Dump mu+tau: merged jets event  "<<event->eventNumber()<<" osatt "<<osatt);
	  for(const xAOD::Muon* muon : *muons) {
	    if (m_selected_mu.get(*muon, sys)){
	      TLorentzVector p4x = muon->p4();
	      if(fabs(p4x.Pt()-p4Tau[0].Pt())<0.001){ // matching with pt of muon 
		const xAOD::TrackParticle* ptrk = muon->trackParticle( xAOD::Muon::InnerDetectorTrackParticle );
		if(ptrk){
		  ATH_MSG_DEBUG("Dump mu+tau: muon from tau decay pt "<<p4x.Pt()<<" eta "<<p4x.Eta()<<" phi "<<p4x.Phi()<<" charge "
				<<muon->charge()<<" trk index "<<ptrk->index()<<" pt "<<ptrk->pt()<<" eta "<<ptrk->eta()<<" phi "
				<<ptrk->phi()<<" charge "<<ptrk->charge());
		}
	      }
	    }
	  }
	  const auto& tpLinks = tau0->tauTrackLinks();
	  int nTrkTau = 0;
	  for(const auto& tpLink : tpLinks){
	    if( !tpLink.isValid() ) continue;	    
	    const xAOD::TrackParticle* ptrk =(*tpLink)->track();
	    ATH_MSG_DEBUG("Dump mu+tau: tracks from tau decay nProng "<<tau0->nTracks()<<" pt "<<p4Tau[1].Pt()<<" eta "<<p4Tau[1].Eta()<<
	    		  " phi "<<p4Tau[1].Phi()<<" charge "<<tau0->charge()<<" trk "<<nTrkTau<<" index "<<ptrk->index()<<" pt "<<
	    		  ptrk->pt()<<" eta "<<ptrk->eta()<<" phi "<<ptrk->phi()<<" charge "<<ptrk->charge());
	    ++nTrkTau;
	  }
	}
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
	ditau_index = isr>0?(iatau1 + 10*iatau2 + 100*isr+1000*recid):100*isr;
	mmc_types = m_mmc_types.get(*event, sys);
	if(ditau_index != mmc_types){
          ATH_MSG_WARNING("Ditau idexes in MMC do not match with selected: ditau_indexes "
                          << ditau_index<<", mmc_types "<<mmc_types);
        }
	// do_mmc is set
	if(m_mmc_status.get(*event, sys)>-1){
	  TLorentzVector mmc_vec(0,0,0,0);
	  mmc_vec.SetPtEtaPhiM(m_mmc_pt.get(*event, sys),
			       m_mmc_eta.get(*event, sys),
			       m_mmc_phi.get(*event, sys),
			       m_mmc_m.get(*event, sys));
	  mmc_maa = (p4lep[iamu1]+p4lep[iamu2]+mmc_vec).M();
	  mmc_ptaa = (p4lep[iamu1]+p4lep[iamu2]+mmc_vec).Pt();
	  mmc_draa = (p4lep[iamu1]+p4lep[iamu2]).DeltaR(mmc_vec);
	  if(mmc_types>0)ATH_MSG_DEBUG(" mmc dump: event "<<event->eventNumber()<<" status "<<m_mmc_status.get(*event, sys)<<" mmc pt "
				       <<m_mmc_pt.get(*event, sys)<<" mmc eta "<<m_mmc_eta.get(*event, sys)<<" mmc phi "<<m_mmc_phi.get(*event, sys)
				       <<" mmc m "<<m_mmc_m.get(*event, sys)<<" mmc maa "<<mmc_maa<<" mmc ptaa "<<mmc_ptaa<<" mmc draa "<<mmc_draa
				       <<" maa "<<maa<<" ptaa "<<ptaa<<" draa "<<draa);
	  m_Fbranches.at("mmc_maa").set(*event, mmc_maa, sys);
	  m_Fbranches.at("mmc_ptaa").set(*event, mmc_ptaa, sys);
	  m_Fbranches.at("mmc_draa").set(*event, mmc_draa, sys);
	}
      }
    }

    return StatusCode::SUCCESS;
  }
}
