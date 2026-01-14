/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "BaselineVarsJJAlg.h"
#include "AthContainers/AuxElement.h"
#include "TLorentzVector.h"

namespace VBSVV4q{

    BaselineVarsJJAlg::BaselineVarsJJAlg(const std::string &name,
                                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator){

    }

    StatusCode BaselineVarsJJAlg::initialize(){
      ATH_MSG_INFO("*********************************\n");
      ATH_MSG_INFO("       JJBaselineVarsAlg         \n");
      ATH_MSG_INFO("*********************************\n");

      // Read syst-aware input handles
      ATH_CHECK (m_SmallRJetsHandle.initialize(m_systematicsList));
      ATH_CHECK (m_LargeRJetsHandle.initialize(m_systematicsList));
      ATH_CHECK (m_SigLargeRJetsHandle.initialize(m_systematicsList));
      if( !m_UseVBFRNN ){
        ATH_CHECK (m_vbsjetHandle.initialize(m_systematicsList));
      }
      else {
        ATH_CHECK (m_RNNjetHandle.initialize(m_systematicsList));
      }
      ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

      if (!m_isBtag.empty()) {
        ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_SmallRJetsHandle));
      }

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

      ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_LargeRJetsHandle));
      ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_LargeRJetsHandle));
      ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_LargeRJetsHandle));
      ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_LargeRJetsHandle));

      for(auto jss : m_JSS_list){
        m_JSS.emplace(jss, CP::SysReadDecorHandle<float>(jss, this));
        ATH_CHECK(m_JSS.at(jss).initialize(m_systematicsList, m_LargeRJetsHandle));
      }
      if (m_isMC)
        ATH_CHECK (m_truth_label.initialize(m_systematicsList, m_LargeRJetsHandle));
      
      // boson tagger
      ATH_CHECK (m_discojet.initialize(m_systematicsList, m_LargeRJetsHandle));

      // Intialise syst list (must come after all syst-aware inputs and outputs)
      ATH_CHECK (m_systematicsList.initialize());

      return StatusCode::SUCCESS;
    }

    StatusCode BaselineVarsJJAlg::execute(){
      // Loop over all systs
      for (const auto& sys : m_systematicsList.systematicsVector()){
        // Retrieve inputs
        const xAOD::EventInfo *event = nullptr;
        ANA_CHECK (m_eventHandle.retrieve (event, sys));

        const xAOD::JetContainer *LargeRJets = nullptr;
        ANA_CHECK (m_LargeRJetsHandle.retrieve (LargeRJets, sys));

        const xAOD::JetContainer *SigLargeRJets = nullptr;
        ANA_CHECK (m_SigLargeRJetsHandle.retrieve (SigLargeRJets, sys));

        const xAOD::JetContainer *SmallRJets = nullptr;
        ANA_CHECK (m_SmallRJetsHandle.retrieve (SmallRJets, sys));

        const xAOD::JetContainer *vbsjets = nullptr;
	const xAOD::JetContainer *RNNJets = nullptr;
        if(!m_UseVBFRNN) {
          ANA_CHECK (m_vbsjetHandle.retrieve (vbsjets, sys));
        }
        else {
          ANA_CHECK (m_RNNjetHandle.retrieve (RNNJets, sys));
        }

        for (const std::string &string_var: m_floatVariables) {
          m_Fbranches.at(string_var).set(*event, -99., sys);
        }

        for (const auto& var: m_intVariables) {
          m_Ibranches.at(var).set(*event, -99, sys);
        }
        
        int nSmallRJets = SmallRJets -> size();
        int nLargeRJets = LargeRJets -> size();
        int nCentralJets = 0;
        int nForwardJets = 0;

        bool WPgiven = !m_isBtag.empty();
        auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);

        // number of central jets
        for(const xAOD::Jet* jet : *SmallRJets) {
          // count central jets
          if (std::abs(jet->eta())<2.5) nCentralJets++;
          else nForwardJets++;

          if (WPgiven) {
            if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet);
          }
        }
        int nBJets = bjets->size();

        m_Ibranches.at("nJets").set(*event, nSmallRJets, sys);
        m_Ibranches.at("nLargeRJets").set(*event, nLargeRJets, sys);
        m_Ibranches.at("nBJets").set(*event, nBJets, sys);
        m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
        m_Ibranches.at("nForwardJets").set(*event, nForwardJets, sys);

        // signal jets
        int iii (0);
        for ( auto jet : *SigLargeRJets ){
          iii++;

          // kinematics
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_pT").set(*event, jet->pt(), sys);
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_eta").set(*event, jet->eta(), sys);
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_phi").set(*event, jet->phi(), sys);
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_E").set(*event, jet->e(), sys); 
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_M").set(*event, jet->m(), sys); 

          // xbb tagger
      	  float phbb (-99.), phcc (-99.), pqcd (-99.), ptop (-99.);
          if(m_loadGN2x){
            phbb = m_GN2Xv01_phbb.get(*jet, sys);
            phcc = m_GN2Xv01_phcc.get(*jet, sys);
            pqcd = m_GN2Xv01_pqcd.get(*jet, sys);
            ptop = m_GN2Xv01_ptop.get(*jet, sys);
          }
          float fcc = 0.02;
          float ftop = 0.25;
          float XbbScore= log (phbb / (fcc*phcc + ftop*ptop + pqcd*(1-fcc-ftop)));
          
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_phbb").set(*event, phbb, sys);
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_phcc").set(*event, phcc, sys);
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_pqcd").set(*event, pqcd, sys);
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_ptop").set(*event, ptop, sys);
          m_Fbranches.at("SigJet" + std::to_string(iii) + "_DXbb").set(*event, XbbScore, sys);

          // truth labelling
          if (m_isMC){
            int tlabel (-99);
            tlabel = m_truth_label.get(*jet, sys);
            m_Fbranches.at("SigJet" + std::to_string(iii) + "_TruthLabel").set(*event, tlabel, sys);
          }
          // jss variables
          for(const auto & jss : m_JSS_list){
            float jss_var = m_JSS.at(jss).get(*jet, sys);
            m_Fbranches.at("SigJet" + std::to_string(iii) + "_" + jss).set(*event, jss_var, sys);
          }
          // add the D2
          if ( m_JSS.at("ECF2").get(*jet, sys) > 1e-8 ) {
            float D2 = m_JSS.at("ECF3").get(*jet, sys) * std::pow( m_JSS.at("ECF1").get(*jet, sys), 3.0 ) / std::pow( m_JSS.at("ECF2").get(*jet, sys), 3.0 );
            m_Fbranches.at("SigJet" + std::to_string(iii) + "_D2").set(*event, D2, sys);
          }

          // boson tagger
          float discojet (-99.);
          if(m_loadDisCoJet){
            discojet = m_discojet.get(*jet, sys);
	    m_Fbranches.at("SigJet" + std::to_string(iii) + "_DisCoJet").set(*event, discojet, sys); 
          }

        }

        TLorentzVector JJ, Jet1, Jet2;
        if (SigLargeRJets -> size() >= 2){
          Jet1 = SigLargeRJets -> at(0) -> p4();
          Jet2 = SigLargeRJets -> at(1) -> p4();
          JJ = Jet1 + Jet2;

          m_Fbranches.at("JJ_M").set(*event, JJ.M(), sys);
          m_Fbranches.at("JJ_pT").set(*event, JJ.Pt(), sys);
          m_Fbranches.at("JJ_eta").set(*event, JJ.Eta(), sys);
          m_Fbranches.at("JJ_phi").set(*event, JJ.Phi(), sys);
          
          m_Fbranches.at("JJ_DR").set(*event, Jet1.DeltaR(Jet2), sys);
          m_Fbranches.at("JJ_dphi").set(*event, Jet1.DeltaPhi(Jet2), sys);
          m_Fbranches.at("JJ_deta").set(*event, Jet1.Eta() - Jet2.Eta(), sys);
        }
        
        // kinematics of tagging jets
	if (!m_UseVBFRNN) {
	  TLorentzVector tagjet1, tagjet2, tag_jj;
	  if ( vbsjets->size() >= 2 ){
	    tagjet1 = vbsjets -> at(0) -> p4();
	    tagjet2 = vbsjets -> at(1) -> p4();
	    tag_jj = tagjet1 + tagjet2;
	    
	    m_Fbranches.at("TagJet1_pT").set(*event, tagjet1.Pt(), sys);
	    m_Fbranches.at("TagJet1_eta").set(*event, tagjet1.Eta(), sys);
	    m_Fbranches.at("TagJet1_phi").set(*event, tagjet1.Phi(), sys);
	    m_Fbranches.at("TagJet1_E").set(*event, tagjet1.E(), sys);
	    
	    m_Fbranches.at("TagJet2_pT").set(*event, tagjet2.Pt(), sys);
	    m_Fbranches.at("TagJet2_eta").set(*event, tagjet2.Eta(), sys);
	    m_Fbranches.at("TagJet2_phi").set(*event, tagjet2.Phi(), sys);
	    m_Fbranches.at("TagJet2_E").set(*event, tagjet2.E(), sys);
	    
	    m_Fbranches.at("TagJets_M").set(*event, tag_jj.M(), sys);
	    m_Fbranches.at("TagJets_deta").set(*event, abs(tagjet1.Eta()-tagjet2.Eta()), sys);
	    
	    m_Fbranches.at("TagJets_pT").set(*event, tag_jj.Pt(), sys);
	    m_Fbranches.at("TagJets_eta").set(*event, tag_jj.Eta(), sys);
	    m_Fbranches.at("TagJets_phi").set(*event, tag_jj.Phi(), sys);
	    
	    m_Fbranches.at("TagJets_DR").set(*event, tagjet1.DeltaR(tagjet2), sys);
	    m_Fbranches.at("TagJets_dphi").set(*event, tagjet1.DeltaPhi(tagjet2), sys);
	  }
	}
	//kinematics of RNN jets
        else {
	  for(unsigned int j=0; j<std::min(size_t(2),RNNJets->size()); j++) {
	    std::string prefix = "Jet"+std::to_string(j+1);
	    const xAOD::Jet* RNNJet = RNNJets->at(j);
	    m_Fbranches.at("RNNJets_"+prefix+"_M").set(*event, RNNJet->m(), sys);
	    m_Fbranches.at("RNNJets_"+prefix+"_E").set(*event, RNNJet->e(), sys);
	    m_Fbranches.at("RNNJets_"+prefix+"_pT").set(*event, RNNJet->pt(), sys);
	    m_Fbranches.at("RNNJets_"+prefix+"_eta").set(*event, RNNJet->eta(), sys);
	    m_Fbranches.at("RNNJets_"+prefix+"_phi").set(*event, RNNJet->phi(), sys);
          }
        }
      }      

      return StatusCode::SUCCESS;
    }

}
