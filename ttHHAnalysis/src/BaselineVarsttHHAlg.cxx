/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Giulia Di Gregorio, Luis Falda

#include "BaselineVarsttHHAlg.h"

#include "AthContainers/AuxElement.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <vector>
#include <tuple>
#include <AthenaKernel/Units.h>

#include "PathResolver/PathResolver.h"
#include "TFile.h"
#include "TH1.h"


namespace ttHH
{
  BaselineVarsttHHAlg::BaselineVarsttHHAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  { }
  
  StatusCode BaselineVarsttHHAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       BaselineVarsttHHAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_bjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_pairedJetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }
    if (!m_PCBT.empty()) {
      ATH_CHECK (m_PCBT.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      m_ele_SF = CP::SysReadDecorHandle<float>("effSF_"+m_eleWPName+"_%SYS%", this);
      ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));
      ATH_CHECK (m_ele_truthOrigin.initialize(m_systematicsList, m_electronHandle));
      ATH_CHECK (m_ele_truthType.initialize(m_systematicsList, m_electronHandle));

      m_mu_SF = CP::SysReadDecorHandle<float>("effSF_"+m_muWPName+"_%SYS%", this);
      ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle));
      ATH_CHECK (m_mu_truthOrigin.initialize(m_systematicsList, m_muonHandle));
      ATH_CHECK (m_mu_truthType.initialize(m_systematicsList, m_muonHandle));
    }

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));

    // Intialise syst-aware output decorators

    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &var : m_intVariables){
      ATH_MSG_DEBUG("initializing integer variable: " << var);
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    if (m_isSignal){
      std::string pathEFTfile = PathResolverFindCalibFile("ttHHAnalysis/rw_CttHH_min3_to_3_histo.root");
      ATH_CHECK(loadEFTWeightFile(pathEFTfile));
    }

    ATH_CHECK(m_truthBosonsWithDecayParticlesContainer.initialize(m_isSignal));
    ATH_CHECK(m_truthTopContainer.initialize(m_isSignal));

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsttHHAlg::execute()
  {
    //Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *bjets = nullptr;
      ANA_CHECK (m_bjetHandle.retrieve (bjets, sys));

      auto top1_jet_candidates = std::make_unique<ConstDataVector<xAOD::JetContainer>>(SG::VIEW_ELEMENTS);
      auto top2_jet_candidates = std::make_unique<ConstDataVector<xAOD::JetContainer>>(SG::VIEW_ELEMENTS);

      const xAOD::JetContainer *pairedJets = nullptr;
      ANA_CHECK (m_pairedJetHandle.retrieve (pairedJets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met){
        ATH_MSG_ERROR("Count not retrieve MET");
	return StatusCode::FAILURE;
      }

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }
      
      for (const auto& string_var: m_intVariables) {
        m_Ibranches.at(string_var).set(*event, -99, sys);
      }

      TLorentzVector met_vector;
      met_vector.SetPtEtaPhiE(met->met(), 0, met->phi(), met->met());

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      if (m_isSignal)
      {
        SG::ReadHandle<xAOD::TruthParticleContainer> truthBosonsParticleContainer(m_truthBosonsWithDecayParticlesContainer);
        ATH_CHECK(truthBosonsParticleContainer.isValid());

        SG::ReadHandle<xAOD::TruthParticleContainer> truthTopParticleContainer(m_truthTopContainer);
        ATH_CHECK(truthTopParticleContainer.isValid());

        auto [truth_ttHH_p4, truth_HH_p4, truth_ttbar_p4] = get_ttHH_HH_ttbar_p4(*truthBosonsParticleContainer, *truthTopParticleContainer, true);
        ATH_CHECK(getEFTShapeWeights(truth_ttHH_p4, truth_HH_p4, truth_ttbar_p4));
        ATH_CHECK(getEFTNormWeights());

        m_Fbranches.at("eft_weight_shape_CttHH_min3").set(*event, m_weights_shape_CttHH_min3, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_min2_5").set(*event, m_weights_shape_CttHH_min2_5, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_min2").set(*event, m_weights_shape_CttHH_min2, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_min1_5").set(*event, m_weights_shape_CttHH_min1_5, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_min1").set(*event, m_weights_shape_CttHH_min1, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_min0_5").set(*event, m_weights_shape_CttHH_min0_5, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_0_5").set(*event, m_weights_shape_CttHH_0_5, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_1").set(*event, m_weights_shape_CttHH_1, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_1_5").set(*event, m_weights_shape_CttHH_1_5, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_2").set(*event, m_weights_shape_CttHH_2, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_2_5").set(*event, m_weights_shape_CttHH_2_5, sys);
        m_Fbranches.at("eft_weight_shape_CttHH_3").set(*event, m_weights_shape_CttHH_3, sys);

        m_Fbranches.at("eft_weight_norm_CttHH_min3").set(*event, m_weights_norm_CttHH_min3, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_min2_5").set(*event, m_weights_norm_CttHH_min2_5, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_min2").set(*event, m_weights_norm_CttHH_min2, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_min1_5").set(*event, m_weights_norm_CttHH_min1_5, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_min1").set(*event, m_weights_norm_CttHH_min1, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_min0_5").set(*event, m_weights_norm_CttHH_min0_5, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_0_5").set(*event, m_weights_norm_CttHH_0_5, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_1").set(*event, m_weights_norm_CttHH_1, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_1_5").set(*event, m_weights_norm_CttHH_1_5, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_2").set(*event, m_weights_norm_CttHH_2, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_2_5").set(*event, m_weights_norm_CttHH_2_5, sys);
        m_Fbranches.at("eft_weight_norm_CttHH_3").set(*event, m_weights_norm_CttHH_3, sys);

      }

      TLorentzVector H1(0, 0, 0, 0);
      TLorentzVector H2(0, 0, 0, 0);
      TLorentzVector e1(0.,0.,0.,0.);
      TLorentzVector e2(0.,0.,0.,0.);
      TLorentzVector ee(0.,0.,0.,0.);
      TLorentzVector mu1(0.,0.,0.,0.);
      TLorentzVector mu2(0.,0.,0.,0.);
      TLorentzVector mumu(0.,0.,0.,0.);
      TLorentzVector emu(0.,0.,0.,0.);

      const xAOD::JetContainer paired_jets = *pairedJets;

      double HT = 0; // scalar sum of jet pT
      double HTall = 0; // scalar sum of jet pT and lepton pT
      int nBJets77 = 0; 

      if (pairedJets->size()>=4 && jets->size()>=4 && bjets->size()>=3){
        int jetsCandidateSize = (jets->size()<6) ? jets->size() : 6;
        std::vector<const xAOD::Jet*> JetsCandidate(jetsCandidateSize, nullptr);
      
        if (pairedJets->size()>=6){
          for (std::size_t i=0; i<std::min(pairedJets->size(),(std::size_t)6); i++){
            JetsCandidate[i] = pairedJets->at(i);
          }
        } else{
          for (std::size_t i=0; i<pairedJets->size(); i++){
            JetsCandidate[i] = pairedJets->at(i);
          }
          if (pairedJets->size()==5 && jets->size()>5){
            for (const auto& jet : *jets) {
              bool alreadyInCandidate = false;
              // Check if jet is already in JetsCandidate
              for (const auto& candidate : JetsCandidate) {
                if (jet == candidate) {
                  alreadyInCandidate = true;
                  break;
                }
              }
              if (!alreadyInCandidate) {
                JetsCandidate[5] = jet;
		break;
	      }
	    }
          } else if (pairedJets->size()==4 && jets->size()>4){
	    for (const auto& jet : *jets) {
              bool alreadyInCandidate = false;
              // Check if jet is already in JetsCandidate
              for (const auto& candidate : JetsCandidate) {
                if (jet == candidate) {
                  alreadyInCandidate = true;
                  break;
                }
              }
              if (!alreadyInCandidate) {
		if (!JetsCandidate[4]){
                  JetsCandidate[4] = jet;
		} else if (jets->size()>5){
                  JetsCandidate[5] = jet;
		  break;
                }
              }
            }
          }
        }
      
        for (std::size_t i=0; i<JetsCandidate.size(); i++){

	  if (m_storeJetBranches) {
	    m_Fbranches.at("Jet"+std::to_string(i+1)+"_pt").set(*event, JetsCandidate[i]->p4().Pt(), sys);
            m_Fbranches.at("Jet"+std::to_string(i+1)+"_eta").set(*event, JetsCandidate[i]->p4().Eta(), sys);
            m_Fbranches.at("Jet"+std::to_string(i+1)+"_phi").set(*event, JetsCandidate[i]->p4().Phi(), sys);
            m_Fbranches.at("Jet"+std::to_string(i+1)+"_E").set(*event, JetsCandidate[i]->p4().E(), sys);

            if(!m_PCBT.empty())
              m_Ibranches.at("Jet"+std::to_string(i+1)+"_pcbt").set(*event,m_PCBT.get(*JetsCandidate[i], sys),sys);
	  }

          if (m_PCBT.get(*JetsCandidate[i], sys) > 3) { // [1,2,3,4,5,6] -> [100%,90%,85%,77%,75%,65%] for GN2v01!
            nBJets77++;
          }
        }
      }

      if (pairedJets->size()>=4) {

        // Build the Higgs candidates
        H1 = pairedJets->at(0)->p4() + pairedJets->at(1)->p4();
        H2 = pairedJets->at(2)->p4() + pairedJets->at(3)->p4();

        auto [DeltaR, DeltaPhi, DeltaEta] = getPairKinematics(paired_jets);

        // Jet pairing variables
        m_Fbranches.at("HH_m").set(*event, (H1+H2).M(), sys);
        m_Fbranches.at("HH_CHI").set(*event, computeChiSquare(H1.M(), H2.M(), m_targetMassH, m_targetMassH, m_massResolution), sys);

        // Create a new JetContainer
        xAOD::Jet jj12 = xAOD::Jet();
        jj12 = *paired_jets[0]; // TODO: breaks if jj12 is empty, not sure what it the best approach...
        xAOD::Jet jj34 = xAOD::Jet();
        jj34 = *paired_jets[0];

        jj12.setJetP4(xAOD::JetFourMom_t(H1.Pt(), H1.Eta(), H1.Phi(), H1.M()));
        jj34.setJetP4(xAOD::JetFourMom_t(H2.Pt(), H2.Eta(), H2.Phi(), H2.M()));

        // calculate deltaR, deltaPhi and deltaEta for jj12_jj34, jj34_jj56, jj56_jj12 combinations
        float deltaR_1234 = xAOD::P4Helpers::deltaR(jj12, jj34);
        DeltaR.push_back(deltaR_1234);

        float deltaEta_1234 = xAOD::P4Helpers::deltaEta(jj12, jj34);
        DeltaEta.push_back(deltaEta_1234);

        if (paired_jets.size() > 5)
        {

          // construct 56 jet combination
          xAOD::JetFourMom_t jj56_p4 = paired_jets[4]->jetP4() + paired_jets[5]->jetP4();
          xAOD::Jet jj56 = xAOD::Jet();
          jj56 = *paired_jets[0];
          jj56.setJetP4(jj56_p4);

          float deltaR_5612 = xAOD::P4Helpers::deltaR(jj56, jj12);
          DeltaR.push_back(deltaR_5612);

          float deltaEta_5612 = xAOD::P4Helpers::deltaEta(jj56, jj12);
          DeltaEta.push_back(deltaEta_5612);

          float deltaR_3456 = xAOD::P4Helpers::deltaR(jj34, jj56);
          DeltaR.push_back(deltaR_3456);

          float deltaEta_3456 = xAOD::P4Helpers::deltaEta(jj34, jj56);
          DeltaEta.push_back(deltaEta_3456);
        }   

        // calculate max, min and mean of mass, deltaEta and deltaR
        auto [DeltaRMax, DeltaRMin, DeltaRMean] = calculateVectorStats(DeltaR);

        m_Fbranches.at("Jets_DeltaRMax").set(*event, DeltaRMax, sys);
        m_Fbranches.at("Jets_DeltaRMin").set(*event, DeltaRMin, sys);
        m_Fbranches.at("Jets_DeltaRMean").set(*event, DeltaRMean, sys);
      }

      if (electrons->size() >= 2) {
        // ee
        e1 = electrons->at(0)->p4();
        e2 = electrons->at(1)->p4();
        ee = e1 + e2;
        m_Fbranches.at("ll_m").set(*event, ee.M(), sys);
      }

      if (muons->size() >= 2) {
        // mumu
        mu1 = muons->at(0)->p4();
        mu2 = muons->at(1)->p4();
        mumu = mu1 + mu2;
        m_Fbranches.at("ll_m").set(*event, mumu.M(), sys);
      }

      if (muons->size() >= 1 and electrons->size() >= 1) {
        mu1 = muons->at(0)->p4();
        e1 = electrons->at(0)->p4();
        emu = e1 + mu1;
        m_Fbranches.at("ll_m").set(*event, emu.M(), sys);
      }

      int nJets = jets->size();
      int nLeptons = muons->size() + electrons->size();
      int nJets_ttbar = top1_jet_candidates->size();
      m_Ibranches.at("nJets").set(*event, nJets, sys);
      m_Ibranches.at("nLeptons").set(*event, nLeptons, sys);
      m_Ibranches.at("nBJets85").set(*event, bjets->size(), sys);
      m_Ibranches.at("nBJets77").set(*event, nBJets77, sys);

      // Run top pairing based on chi2
      if (m_runTopness) {

	if (pairedJets->size()>=4) {
          // Build top1 jet candidates
          for (size_t i = 4; i<pairedJets->size(); i++){
            const xAOD::Jet* paired_jet = pairedJets->at(i);
            top1_jet_candidates->push_back(paired_jet);
          }
          for (const auto& jet : *jets) {
            bool isJetInVec = false;
            for (size_t i = 4; i<pairedJets->size(); i++){
              const xAOD::Jet* paired_jet = pairedJets->at(i);
              if (jet == paired_jet){
                isJetInVec = true;
              }
            }
            if (!isJetInVec){
              top1_jet_candidates->push_back(jet);
            }
          }
        }

	std::vector<std::tuple<int, double>> leptonmasses;
        for (unsigned int i = 0; i<muons->size(); i++){
          leptonmasses.emplace_back(i, mu_mass);
        }
        for (unsigned int j = 0; j<electrons->size(); j++){
          leptonmasses.emplace_back(j, e_mass);
        }

	bool top1_had = false;
	bool top2_had = false;
	bool all_had = false;
	bool semi_lep = false;
	bool di_lep = false;
	if (nLeptons == 0 and nJets_ttbar >= 6){ // all hadronic
	  top1_had = true;
	  top2_had = true;
	  all_had = true;
	}
	if (nLeptons == 1 and nJets_ttbar >= 4){ // semi-leptonic
	  top2_had = true;
	  semi_lep = true;
	}
	if (nLeptons == 2 and nJets_ttbar >= 2){ // di-lepton
	  di_lep = true;
	}

	std::vector<unsigned int> top1_jet_locations;
	std::vector<unsigned int> top2_jet_locations;
	std::vector<std::tuple<unsigned int, double>> top1_lepton_locations;
	std::vector<std::tuple<unsigned int, double>> top2_lepton_locations;

	// Build top1 lepton candidates
	std::vector<std::tuple<int, double>> top1_lepton_candidates = leptonmasses;
	double topness1 = -99.;
	if (all_had or semi_lep or di_lep){
	  topness1 = computeChiSquaretops(*top1_jet_candidates, top1_lepton_candidates, met_vector, top1_had, top1_jet_locations, top1_lepton_locations, electrons, muons);
	}
	m_Fbranches.at("topness1").set(*event, topness1, sys);

	for (size_t i = 0; i<top1_jet_candidates->size(); i++) {
	  const xAOD::Jet* jet = top1_jet_candidates->at(i);
	  bool is_jet_from_top1 = false;
	  for (const unsigned int& id : top1_jet_locations) {
	    const xAOD::Jet* top1_jet = top1_jet_candidates->at(id);
	    if (top1_jet == jet) {
	      is_jet_from_top1 = true;
	      break;
	    } 
	  }
	  if (!is_jet_from_top1){
	    top2_jet_candidates->push_back(jet);
	  }
	}

	// Build top2 lepton candidates
	std::vector<std::tuple<int, double>> top2_lepton_candidates;
	if (semi_lep){
	  top2_lepton_candidates = top1_lepton_candidates;
	}
	if (di_lep){
	  for (const auto& leptonmass : leptonmasses){
	    if (leptonmass != top1_lepton_locations[0]){
	      top2_lepton_candidates.push_back(leptonmass);
	    }        
	  }
	}

	double topness2 = -99.;
	if (all_had or semi_lep or di_lep){
	  topness2 = computeChiSquaretops(*top2_jet_candidates, top2_lepton_candidates, met_vector, top2_had, top2_jet_locations, top2_lepton_locations, electrons, muons);
	}
	m_Fbranches.at("topness2").set(*event, topness2, sys);
      }

      //----------------------------------------------------------
      //-- Multileptons

      size_t muonSize = muons->size();

      if (nLeptons == 1){
        //-- Filling Lepton branches
        if (muonSize==1){ // mu
          const xAOD::Muon* muon0 = muons->at(0);
          m_Ibranches.at("total_charge").set(*event, muon0->charge(), sys);
          updateLeptonBranch(event, 1, muon0, 13, m_isMC ? m_mu_SF.get(*muon0, sys) : 1.0 , sys);
	  HTall = muon0->pt();
        } else { // ele 
          const xAOD::Electron* electron0 = electrons->at(0);
          m_Ibranches.at("total_charge").set(*event, electron0->charge(), sys);
          updateLeptonBranch(event, 1, electron0, 11, m_isMC ? m_ele_SF.get(*electron0, sys) : 1.0 , sys);
	  HTall = electron0->pt();
        }

      }
      else if (nLeptons == 2){
        //-- total charge
        int totalCharge = 0;
        for (const auto& muon : *muons) {
          totalCharge += muon->charge();
	  HTall += muon->pt();
	}
        for (const auto& electron : *electrons) {
          totalCharge += electron->charge();
          HTall += electron->pt();
	}

	m_Ibranches.at("total_charge").set(*event, totalCharge, sys);
        m_Ibranches.at("dilept_type").set(*event, muonSize == 2 ? 3 : (muonSize == 1 ? 2 : 1), sys);

        //-- Filling Lepton branches
        if (muonSize==2){ // mumu
          
          const xAOD::Muon* muon0 = muons->at(0);
          const xAOD::Muon* muon1 = muons->at(1);
          updateLeptonBranch(event, 1, muon0, 13, m_isMC ? m_mu_SF.get(*muon0, sys) : 1.0 , sys);
          updateLeptonBranch(event, 2, muon1, 13, m_isMC ? m_mu_SF.get(*muon1, sys) : 1.0 , sys);

        } else if (muonSize==1){ // emu
          
          const xAOD::Muon* muon0 = muons->at(0);
          const xAOD::Electron* electron0 = electrons->at(0);
          if (muon0->pt()>electron0->pt()){
            updateLeptonBranch(event, 1, muon0, 13, m_isMC ? m_mu_SF.get(*muon0, sys) : 1.0 , sys);
            updateLeptonBranch(event, 2, electron0, 11, m_isMC ? m_ele_SF.get(*electron0, sys) : 1.0 , sys);
          } else {
            updateLeptonBranch(event, 2, muon0, 13, m_isMC ? m_mu_SF.get(*muon0, sys) : 1.0 , sys);
            updateLeptonBranch(event, 1, electron0, 11, m_isMC ? m_ele_SF.get(*electron0, sys) : 1.0 , sys);
          }

        } else { //ee
          
          const xAOD::Electron* electron0 = electrons->at(0);
          const xAOD::Electron* electron1 = electrons->at(1);          
          updateLeptonBranch(event, 1, electron0, 11, m_isMC ? m_ele_SF.get(*electron0, sys) : 1.0 , sys);
          updateLeptonBranch(event, 2, electron1, 11, m_isMC ? m_ele_SF.get(*electron1, sys) : 1.0 , sys);
        }
      } else { //not 2l
        m_Ibranches.at("dilept_type").set(*event, 0, sys);
      }
      //-- 3l
      m_Ibranches.at("trilept_type").set(*event, (nLeptons == 3) ? 1 : 0, sys);
      //--

      int sumPCBT = 0; // sum of pcbt scores for one event

      for (const xAOD::Jet *jet : *jets) // Jets here can be every type of jet (No Working point selected)
      {
	// calculate HT
        HT += jet->pt();

	// calculate sumPCBT
	int pcbt = m_PCBT.get(*jet, sys);
	if (pcbt > 0) {
	  sumPCBT += pcbt;
	}
      }

      HTall += HT;

      m_Fbranches.at("HT").set(*event, HT, sys);
      m_Fbranches.at("HTall").set(*event, HTall, sys);
      m_Ibranches.at("sumPCBT").set(*event, sumPCBT, sys);

    }
    return StatusCode::SUCCESS;

  }

  std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> BaselineVarsttHHAlg::getPairKinematics(const xAOD::JetContainer& jetPairs)
  {

    std::vector<double> DeltaR = {xAOD::P4Helpers::deltaR(jetPairs[0], jetPairs[1]), xAOD::P4Helpers::deltaR(jetPairs[2], jetPairs[3])};
    std::vector<double> DeltaPhi = {xAOD::P4Helpers::deltaPhi(jetPairs[0], jetPairs[1]), xAOD::P4Helpers::deltaPhi(jetPairs[2], jetPairs[3])};
    std::vector<double> DeltaEta = {xAOD::P4Helpers::deltaEta(jetPairs[0], jetPairs[1]), xAOD::P4Helpers::deltaEta(jetPairs[2], jetPairs[3])};

    if (jetPairs.size() > 5) {
      DeltaR.push_back(xAOD::P4Helpers::deltaR(jetPairs[4], jetPairs[5]));
      DeltaPhi.push_back(xAOD::P4Helpers::deltaPhi(jetPairs[4], jetPairs[5]));
      DeltaEta.push_back(xAOD::P4Helpers::deltaEta(jetPairs[4], jetPairs[5]));
    }

    return {DeltaR, DeltaPhi, DeltaEta};
  }

  std::tuple<double, double, double> BaselineVarsttHHAlg::calculateVectorStats(const std::vector<double>& inputVector)
  {
    // Function to calculate and store the maximum, minimum, and mean of a vector of floats

    // Initialize variables for max, min, and sum
    double max_value = inputVector[0];
    double min_value = inputVector[0];
    double sum = 0.0;

    // Iterate through the input vector
    for (double value : inputVector) {
        // Update max and min
        max_value = std::max(max_value, value);
        min_value = std::min(min_value, value);

        // Accumulate sum
        sum += value;
    }

    // Calculate mean
    double mean = sum / inputVector.size();

    return {max_value, min_value, mean};
  }

  float BaselineVarsttHHAlg::computeChiSquare(float observedMass1, float observedMass2, float targetMass1, float targetMass2, float massResolution)
  {
    // Function to calculate chi square of jet pairs

    // ratio between target mass and invariant mass of the jet pair
    float r_12 = (targetMass1 - observedMass1);
    float r_34 = (targetMass2 - observedMass2);

    // calculate the CHI squared
    float chi_squared = ( r_12 * r_12 + r_34 * r_34 ) / (massResolution * massResolution);

    return chi_squared;
  }

  double BaselineVarsttHHAlg::computeChiSquaretops
  (const ConstDataVector<xAOD::JetContainer>& jets,
   const std::vector<std::tuple<int, double>> &leptonmasses,
   const TLorentzVector &met,
   bool top_had,
   std::vector<unsigned int> &jet_locations,
   std::vector<std::tuple<unsigned int, double>> &lepton_locations,
   const xAOD::ElectronContainer *electrons,
   const xAOD::MuonContainer *muons) const{
    float minTopness = std::numeric_limits<float>::max();

    if (top_had){ //top decays hadronically
      unsigned int jet1 = 0;
      unsigned int jet2 = 0;
      unsigned int jet3 = 0;
      for (unsigned int j1 = 0; j1<jets.size()-2; j1++){
        for (unsigned int j2 = j1+1; j2<jets.size()-1; j2++){
          for (unsigned int j3 = j2+1; j3<jets.size(); j3++){
            double m_j1j2 = (jets[j1]->p4() + jets[j2]->p4()).M();
            double m_j1j2j3 = (jets[j1]->p4() + jets[j2]->p4() + jets[j3]->p4()).M();
            double topness = std::hypot((m_j1j2-wmass)/wmass, (m_j1j2j3-topmass)/topmass);
            if (topness<minTopness) {
              minTopness = topness;
              jet1 = j1;
              jet2 = j2;
              jet3 = j3;
            }
          }
        }
      }
      jet_locations.push_back(jet1);
      jet_locations.push_back(jet2);
      jet_locations.push_back(jet3);
    }
    else { //top decays leptonically
      unsigned int jet1 = 0;
      unsigned int lep1_id = 0;
      double lep1_mass = 0.;

      for (unsigned int j = 0; j<jets.size(); j++){
        for (const auto& leptonmass : leptonmasses){
          double m_j_lep_met;
          double m_lep_met;
          if (get<1>(leptonmass) == e_mass) {
            m_j_lep_met = ((jets)[j]->p4() + electrons->at(get<0>(leptonmass))->p4() + met).M();
            m_lep_met = (electrons->at(get<0>(leptonmass))->p4() + met).M();
          }
          else {
            m_j_lep_met = ((jets)[j]->p4() + muons->at(get<0>(leptonmass))->p4() + met).M();
            m_lep_met = (muons->at(get<0>(leptonmass))->p4() + met).M();
          }
          double topness = std::hypot((m_lep_met-wmass)/wmass, (m_j_lep_met-topmass)/topmass);
          if (topness<minTopness) {
            minTopness = topness;
            jet1 = j;
            lep1_id = get<0>(leptonmass);
            lep1_mass = get<1>(leptonmass);
          }
        }
      }
      jet_locations.push_back(jet1);
      lepton_locations.emplace_back(lep1_id, lep1_mass);
    }
    return minTopness;
  }

  //-------------------------------------------------------------------------------------------
  // Fill the branches Lepton*_*
  
  template<typename ParticleType>
  void BaselineVarsttHHAlg::updateLeptonBranch(const xAOD::EventInfo *event, int leptonIndex, const ParticleType* particle,  
                                       int lep_pdgid, float lep_sf, 
                                       const CP::SystematicSet& sys) {
  
    // Branch name using lepton index
    std::string prefix = "Lepton" + std::to_string(leptonIndex) + "_";

    TLorentzVector lep = particle->p4();
    
    // Kinematics
    m_Fbranches.at(prefix + "pt").set(*event, lep.Pt(), sys);
    m_Fbranches.at(prefix + "E").set(*event, lep.E(), sys);
    m_Fbranches.at(prefix + "eta").set(*event, lep.Eta(), sys);
    m_Fbranches.at(prefix + "phi").set(*event, lep.Phi(), sys);

    // Easyjet properties
    m_Ibranches.at(prefix + "charge").set(*event, particle->charge(), sys);
    m_Ibranches.at(prefix + "pdgid").set(*event, -1*lep_pdgid*particle->charge(), sys);
    if(m_isMC) m_Fbranches.at(prefix + "effSF").set(*event, lep_sf, sys);

    if (lep_pdgid==13){ 
      m_Ibranches.at(prefix + "isTight").set(*event, m_selected_mu.get(*particle, sys), sys);
    } else if (lep_pdgid==11){
      m_Ibranches.at(prefix + "isTight").set(*event, m_selected_el.get(*particle, sys), sys);
    }

    // Truth
    if (m_isMC) {
      int lep_truthOrigin = std::abs(lep_pdgid)==11 ?
        m_ele_truthOrigin.get(*particle, sys) : m_mu_truthOrigin.get(*particle, sys);
      m_Ibranches.at(prefix + "truthOrigin").set(*event, lep_truthOrigin, sys);
      int lep_truthType = std::abs(lep_pdgid)==11 ?
        m_ele_truthType.get(*particle, sys) : m_mu_truthType.get(*particle, sys);
      m_Ibranches.at(prefix + "truthType").set(*event, lep_truthType, sys);
    
      int lep_isPrompt = 0;
    
      if (lep_pdgid==13){ // simplistic
        if (lep_truthType==6) lep_isPrompt=1; // isolated prompts
      } else if (lep_pdgid==11){
        if (lep_truthType==2) lep_isPrompt=1; // isolated prompts
      }
    
      m_Ibranches.at(prefix + "isPrompt").set(*event, lep_isPrompt, sys);
    }
  }

  StatusCode BaselineVarsttHHAlg::loadEFTWeightFile(const std::string &filePath)
  {

    TFile *f = TFile::Open(filePath.c_str());
    if (!f || f->IsZombie())
    {
      ATH_MSG_ERROR("Cannot open file \""
                    << filePath << "\" or the file is in a bad state.");
      return StatusCode::FAILURE;
    }

    // Retrieve the histogram named "EFTweights"
    m_hist_EFTWeight_shape_CttHH_min3 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_min3"));
    m_hist_EFTWeight_shape_CttHH_min2_5 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_min2_5"));
    m_hist_EFTWeight_shape_CttHH_min2 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_min2"));
    m_hist_EFTWeight_shape_CttHH_min1_5 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_min1_5"));
    m_hist_EFTWeight_shape_CttHH_min1 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_min1"));
    m_hist_EFTWeight_shape_CttHH_min0_5 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_min0_5"));
    m_hist_EFTWeight_shape_CttHH_0_5 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_0_5"));
    m_hist_EFTWeight_shape_CttHH_1 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_1"));
    m_hist_EFTWeight_shape_CttHH_1_5 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_1_5"));
    m_hist_EFTWeight_shape_CttHH_2 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_2"));
    m_hist_EFTWeight_shape_CttHH_2_5 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_2_5"));
    m_hist_EFTWeight_shape_CttHH_3 = dynamic_cast<TH2D *>(f->Get("EFTweights_shape_CttHH_3"));

    m_hist_EFTWeight_norm_CttHH_min3 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_min3"));
    m_hist_EFTWeight_norm_CttHH_min2_5 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_min2_5"));
    m_hist_EFTWeight_norm_CttHH_min2 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_min2"));
    m_hist_EFTWeight_norm_CttHH_min1_5 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_min1_5"));
    m_hist_EFTWeight_norm_CttHH_min1 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_min1"));
    m_hist_EFTWeight_norm_CttHH_min0_5 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_min0_5"));
    m_hist_EFTWeight_norm_CttHH_0_5 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_0_5"));
    m_hist_EFTWeight_norm_CttHH_1 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_1"));
    m_hist_EFTWeight_norm_CttHH_1_5 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_1_5"));
    m_hist_EFTWeight_norm_CttHH_2 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_2"));
    m_hist_EFTWeight_norm_CttHH_2_5 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_2_5"));
    m_hist_EFTWeight_norm_CttHH_3 = dynamic_cast<TH1F *>(f->Get("EFTweights_norm_CttHH_3"));

    f->Clear(); 
    f->Close();
    
    if (!m_hist_EFTWeight_shape_CttHH_min3) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_min3 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_min2_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_min2_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_min2) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_min2 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_min1_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_min1_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_min1) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_min1 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_min0_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_min0_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_0_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_0_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_1) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_1 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_1_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_1_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_2) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_2 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_2_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_2_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_shape_CttHH_3) {
      ATH_MSG_ERROR("m_hist_EFTWeight_shape_CttHH_3 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_min3) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_min3 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_min2_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_min2_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_min2) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_min2 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_min1_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_min1_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_min1) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_min1 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_min0_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_min0_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_0_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_0_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_1) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_1 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_1_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_1_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_2) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_2 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_2_5) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_2_5 not found");
      return StatusCode::FAILURE;
      }
    if (!m_hist_EFTWeight_norm_CttHH_3) {
      ATH_MSG_ERROR("m_hist_EFTWeight_norm_CttHH_3 not found");
      return StatusCode::FAILURE;
      }

    ATH_MSG_INFO("Successfully loaded all 'EFTweights' histograms from file \""
                 << filePath << "\".");
    return StatusCode::SUCCESS;

  }

  TLorentzVector BaselineVarsttHHAlg::get_pair_p4(const xAOD::TruthParticleContainer &truthContainer, int target_pdgId, bool initial)
  {
    // initial (bool), by default true. If set to false, the final truth
    // particles will be used rather than the initial.
    TLorentzVector pair_p4;
    int pair_count = 0;
    int nParents = 0;
    int nChildren = 0;
    int abs_pdgID = 0;

    for (const xAOD::TruthParticle *particle : truthContainer)
    {
      nParents = particle->nParents();
      nChildren = particle->nChildren();
      abs_pdgID = particle->absPdgId();
      if (initial)
      {
        if (abs_pdgID == target_pdgId && !(particle->status() >= 41))
        {
          int parents_with_same_pdgID = 0;
          for (int i = 0; i < nParents; ++i)
          {
            if (particle->parent(i)->pdgId() == particle->pdgId() && nChildren>1)
            {
               parents_with_same_pdgID += 1;
            }
          }
          if(parents_with_same_pdgID>1)
            ATH_MSG_WARNING("Found " << parents_with_same_pdgID << " parents with the same pdgID for truth particle with pdgID = " << particle->pdgId());
          if(parents_with_same_pdgID==0)
          {
            pair_p4 += particle->p4();
            pair_count += 1;
          }

        }
      }
      else if (abs_pdgID == target_pdgId && nChildren>1)
      {
        bool hasSamePdgIdChild = false;
        for (int i = 0; i < nChildren; ++i) {
          if (particle->child(i)->pdgId() == particle->pdgId()) {
            hasSamePdgIdChild = true;
            break;
          }
        }
        if (!hasSamePdgIdChild) {
          pair_p4 += particle->p4();
          pair_count += 1;
        }
      } 
    }
    if (pair_count != 2) {ATH_MSG_WARNING("Found " << pair_count << " truth particles with pdgID = " << target_pdgId);}
    return pair_p4;
  }

  std::tuple<TLorentzVector, TLorentzVector, TLorentzVector> BaselineVarsttHHAlg::get_ttHH_HH_ttbar_p4(const xAOD::TruthParticleContainer &bosonsContainer, const xAOD::TruthParticleContainer &topsContainer, bool initial)
  {
    TLorentzVector ttHH_p4;
    TLorentzVector HH_p4 = get_pair_p4(bosonsContainer, 25, initial);
    TLorentzVector ttbar_p4 = get_pair_p4(topsContainer, 6, initial);
    ttHH_p4 = HH_p4 + ttbar_p4;
    return std::make_tuple(ttHH_p4, HH_p4, ttbar_p4);
  }

  double BaselineVarsttHHAlg::findAndGetEFTWeight(double ttHH_mass, double ttbar_mass, TH2D *hist_EFTWeight_shape)
  {
    int bin_index = hist_EFTWeight_shape->FindBin(ttHH_mass, ttbar_mass);
    return hist_EFTWeight_shape->GetBinContent(bin_index);
  }

  StatusCode BaselineVarsttHHAlg::getEFTShapeWeights(TLorentzVector &ttHH_p4, TLorentzVector &HH_p4, TLorentzVector &ttbar_p4)
  {

    // Get the ttHH invariant mass
    double ttHH_mass = ttHH_p4.M() / Athena::Units::GeV;
    double HH_mass = HH_p4.M() / Athena::Units::GeV;
    double ttbar_mass = ttbar_p4.M() / Athena::Units::GeV;

    ATH_MSG_DEBUG("HH mass: " << HH_mass);
    ATH_MSG_DEBUG("ttbar mass: " << ttbar_mass);
    ATH_MSG_DEBUG("ttHH mass: " << ttHH_mass);

    // TODO : Needs to be generalised!!! -> Histograms should not be each in single pointers
    // Find the bins corresponding to the ttHH invariant mass
    m_weights_shape_CttHH_min3 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_min3);
    m_weights_shape_CttHH_min2_5 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_min2_5);
    m_weights_shape_CttHH_min2 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_min2);
    m_weights_shape_CttHH_min1_5 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_min1_5);
    m_weights_shape_CttHH_min1 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_min1);
    m_weights_shape_CttHH_min0_5 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_min0_5);
    m_weights_shape_CttHH_0_5 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_0_5);
    m_weights_shape_CttHH_1 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_1);
    m_weights_shape_CttHH_1_5 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_1_5);
    m_weights_shape_CttHH_2 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_2);
    m_weights_shape_CttHH_2_5 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_2_5);
    m_weights_shape_CttHH_3 = findAndGetEFTWeight(ttHH_mass, ttbar_mass, m_hist_EFTWeight_shape_CttHH_3);

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsttHHAlg::getEFTNormWeights()
  {
    m_weights_norm_CttHH_min3 = m_hist_EFTWeight_norm_CttHH_min3->GetBinContent(1);
    m_weights_norm_CttHH_min2_5 = m_hist_EFTWeight_norm_CttHH_min2_5->GetBinContent(1);
    m_weights_norm_CttHH_min2 = m_hist_EFTWeight_norm_CttHH_min2->GetBinContent(1);
    m_weights_norm_CttHH_min1_5 = m_hist_EFTWeight_norm_CttHH_min1_5->GetBinContent(1);
    m_weights_norm_CttHH_min1 = m_hist_EFTWeight_norm_CttHH_min1->GetBinContent(1);
    m_weights_norm_CttHH_min0_5 = m_hist_EFTWeight_norm_CttHH_min0_5->GetBinContent(1);
    m_weights_norm_CttHH_0_5 = m_hist_EFTWeight_norm_CttHH_0_5->GetBinContent(1);
    m_weights_norm_CttHH_1 = m_hist_EFTWeight_norm_CttHH_1->GetBinContent(1);
    m_weights_norm_CttHH_1_5 = m_hist_EFTWeight_norm_CttHH_1_5->GetBinContent(1);
    m_weights_norm_CttHH_2 = m_hist_EFTWeight_norm_CttHH_2->GetBinContent(1);
    m_weights_norm_CttHH_2_5 = m_hist_EFTWeight_norm_CttHH_2_5->GetBinContent(1);
    m_weights_norm_CttHH_3 = m_hist_EFTWeight_norm_CttHH_3->GetBinContent(1);

    return StatusCode::SUCCESS;
  }
}
