/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam

#include "MMCDecoratorAlg.h"

using ROOT::Math::PtEtaPhiMVector;
using ROOT::Math::PxPyPzEVector;
using ROOT::Math::VectorUtil::DeltaR;

namespace HLLTT
{
  MMCDecoratorAlg ::MMCDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {
  
  }

  StatusCode MMCDecoratorAlg ::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_pass_baseline_LepLep.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_baseline_LepHad.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_baseline_HadHad.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_tauHandle));
    
    ATH_CHECK (m_selected_el_amm.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu_amm.initialize(m_systematicsList, m_muonHandle));

    // Intialise syst-aware output decorators
    ATH_CHECK(m_mmc_status.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_types.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ANA_CHECK (m_systematicsList.initialize());    

    for ( auto name : m_channel_names){
      if( name == "LepLep") m_channels.push_back(HLLTT::LepLep);
      else if( name == "LepHad") m_channels.push_back(HLLTT::LepHad);
      else if ( name == "HadHad") m_channels.push_back(HLLTT::HadHad);
      else{
        ATH_MSG_ERROR("Unknown channel");
        return StatusCode::FAILURE;
      }
    }

    ATH_CHECK (m_mmcTool.retrieve());

    if (m_method_str == "MLNU3P") 
      m_method = DiTauMassTools::MMCFitMethod::MLNU3P;
    else if (m_method_str == "MAXW") 
      m_method = DiTauMassTools::MMCFitMethod::MAXW;
    else if (m_method_str == "MLM") 
      m_method = DiTauMassTools::MMCFitMethod::MLM;
    else {
      ATH_MSG_ERROR("Unknown MMC method " << m_method_str);
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode MMCDecoratorAlg ::execute()
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

      // Setup MMC inputs and outputs
      const xAOD::IParticle* part1 = nullptr;
      const xAOD::IParticle* part2 = nullptr;
      int status = 0;      
      PtEtaPhiMVector res(0,0,0,0);

      int n_lep(0);
      PxPyPzEVector p4lep[4];
      int lepid[4];
      int iamu1(-1);
      int iamu2(-1);
      int isr(-1);
      int recid(0);
      int iatau1(-1);
      int iatau2(-1);
      float apt(0);
      float drmin(99.);
      // alternative combination with second leading pt
      int iamu1x(-1);
      int iamu2x(-1);
      float aptx(0);
      float drminx(99.);
      int types(0); //iatau1+10*iatau2 +100*isr
      int n_taus(0);
      //
      int nlep(0);

      for(const xAOD::Muon* muon : *muons) {
	m_selected_mu_amm.set(*muon, false, sys);
	if (m_selected_mu.get(*muon, sys)){
	  if(n_lep<4){	    
            p4lep[n_lep] = muon->genvecP4();
	    lepid[n_lep] = muon->charge()>0?-13: 13;
            ++n_lep;
          }
        }
      }

      for(const xAOD::Electron* electron : *electrons) {
	m_selected_el_amm.set(*electron, false, sys);
	if (m_selected_el.get(*electron, sys)){
	  if(n_lep<4){
	    p4lep[n_lep] = electron->genvecP4();
	    lepid[n_lep] = electron->charge()>0? -11:11;
	    ++n_lep;
	  }
	}
      }
      
      for(const xAOD::TauJet* tau : *taus) {
        if (m_selected_tau.get(*tau, sys)){
          ++n_taus;
	}
      }

      if( n_lep > 1 ){
	// select leading lepton and close lepton nearby unbiased search
	for(int i = 0; i<n_lep; ++i){
          if(p4lep[i].Pt()>apt){
            iamu1 = i;
            apt = p4lep[i].Pt();
          }
        }
        for(int i = 0; i<n_lep; ++i){
          if(iamu1 != i){
            float dr = DeltaR(p4lep[iamu1], p4lep[i]);
            if( dr<drmin){
              iamu2 = i;
              drmin = dr;
            }
          }
        }
	//alternarive with second leading pt                                                                                                                                                          
        if(n_lep>2){
	  for(int i = 0; i<n_lep; ++i){
            if(iamu1!=i&&iamu2!=i&&p4lep[i].Pt()>aptx){
              iamu1x = i;
              aptx = p4lep[i].Pt();
            }
          }
          for(int i = 0; i<n_lep; ++i){
            if(iamu1x != i){
              float dr = DeltaR(p4lep[iamu1x], p4lep[i]);
              if( dr<drminx){
                iamu2x = i;
                drminx = dr;
              }
            }
          }
          // selecting with smaller dr
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
	    recid=1;
          }
        }
	if(iamu2>-1){
          isr = 0;
	  ATH_MSG_DEBUG(" Reconstructed isr:"<<isr<<" iamu1:"<<iamu1<<" iamu2:"<<iamu2);
	  int ix(0);
	  for(const xAOD::Muon* muon : *muons) {
	    if (m_selected_mu.get(*muon, sys)){
	      if(ix<4){
		if(ix==iamu1||ix==iamu2)m_selected_mu_amm.set(*muon, true, sys);
		++ix;
	      }
	    }
	  }
	  if(abs(lepid[iamu1])==11||abs(lepid[iamu2])==11){
	    for(const xAOD::Electron* electron : *electrons) {
	      if (m_selected_el.get(*electron, sys)){
		if(ix<4){
		  if(ix==iamu1||ix==iamu2)m_selected_el_amm.set(*electron, true, sys);
		  ++ix;
		}
	      }
	    }
	  }	  
          if(n_lep ==4){
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
            }
          }
	  if(n_lep==3&&n_taus==1){
            iatau1 = -1;
            for(int j = 0; j<n_lep; ++j){
              if(j !=iamu1 && j !=iamu2){
                iatau1 = j;
              }
            }
            if(iatau1>-1){
              isr = 2;
	      iatau2 = 0;
            }
          }
          if(n_lep ==2&&n_taus==2){
            isr = 3;
	    iatau1 = 0;
	    iatau2 = 1;
          }
        }
      }
      
      if(isr==1){ 
	for(const xAOD::Muon* muon : *muons) {
	  if (m_selected_mu.get(*muon, sys)){
	    if(nlep==iatau1)part1 = muon;
	    if(nlep==iatau2){
	      part2 = muon;
	      break;
	    }
	    ++nlep;
	  }
	}
	if(!part1 || !part2){
	  for(const xAOD::Electron* electron : *electrons) {
	    if (m_selected_el.get(*electron, sys)){
	      if(nlep==iatau1)part1 = electron;
	      if(nlep==iatau2){
		part2 = electron;
		break;
	      }
	    ++nlep;
	    }
	  }
	}
      }
      else if(isr==2){ 
	for(const xAOD::Muon* muon : *muons) {
          if (m_selected_mu.get(*muon, sys)){
            if(nlep==iatau1){
	      part1 = muon;
	      break;
	    }
            ++nlep;
          }
        }
	if(!part1){
	  for(const xAOD::Electron* electron : *electrons) {
	    if (m_selected_el.get(*electron, sys)){
	      if(nlep==iatau1){
		part1 = electron;
		break;
	      }
	      ++nlep;
	    }
	  }
	}
	for(const xAOD::TauJet* tau : *taus) {
	  if (m_selected_tau.get(*tau, sys)){
	    if(!part2) {
	      part2 = tau;
	      break;
	    }
	  }
	}
      }
      else if(isr==3){
	for(const xAOD::TauJet* tau : *taus) {
	  if (m_selected_tau.get(*tau, sys)){
	    if(!part1) part1 = tau;
	    else{
	      part2 = tau;
	      break;
	    }
	  }
	}
      }

      // Run MMC if find eligible particle content
      if (part1 && part2) {
	auto code = m_mmcTool->apply(*event, part1, part2, met, jets->size());
	
	if (code != CP::CorrectionCode::Ok) 
	  {
	    ATH_MSG_ERROR("MMC application failed");
	    return StatusCode::FAILURE;	    
	  }
	
	status = m_mmcTool->GetFitStatus(m_method);
	if (status == 1) {
	  res = m_mmcTool->GetResonanceVec(m_method)*1e3;
	}
      }

      // Decorate ouput
      types = isr>0?(iatau1+10*iatau2+100*isr+1000*recid):100*isr;
      ATH_MSG_DEBUG(" MMCDecoratorAlg fits:  event "<<event->eventNumber()<<" mmc m "<< res.M()<<" types "<<types
		    <<" nlep "<<n_lep<<" ntaus "<<n_taus<<" isr "<<isr<<" iatau1 "<<iatau1<<" iatau2 "<<iatau2);
      m_mmc_status.set(*event, status, sys);
      m_mmc_types.set(*event, types, sys);
      m_mmc_pt.set(*event, res.Pt(), sys);
      m_mmc_eta.set(*event, res.Eta(), sys);
      m_mmc_phi.set(*event, res.Phi(), sys);
      m_mmc_m.set(*event, res.M(), sys);            
    }

    return StatusCode::SUCCESS;
  }
}
