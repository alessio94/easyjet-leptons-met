/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "BaselineVarsSemiLepAlg.h"
#include "AthContainers/AuxElement.h"
#include "TLorentzVector.h"


namespace VBSHIGGS{
    BaselineVarsSemiLepAlg::BaselineVarsSemiLepAlg(const std::string &name,
                                            ISvcLocator *pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator){

    }

    StatusCode BaselineVarsSemiLepAlg::initialize(){
        ATH_MSG_INFO("*********************************\n");
        ATH_MSG_INFO("       SemiLepBaselineVarsAlg    \n");
        ATH_MSG_INFO("*********************************\n");

        // Read syst-aware input handles
	ATH_CHECK (m_signaljetHandle.initialize(m_systematicsList));
        ATH_CHECK (m_vbsjetHandle.initialize(m_systematicsList));
        ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
        ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
        ATH_CHECK (m_metHandle.initialize(m_systematicsList));
        ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

        if(m_isMC){
            m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
        }
        ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

        if(m_isMC){
            m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
        }
        ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

        if (!m_isBtag.empty()) {
            ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_signaljetHandle));
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

        // Intialise syst list (must come after all syst-aware inputs and outputs)
        ATH_CHECK (m_systematicsList.initialize());
        return StatusCode::SUCCESS;
    }

    StatusCode BaselineVarsSemiLepAlg::execute(){
        // Loop over all systs
        for (const auto& sys : m_systematicsList.systematicsVector()){

            // Retrieve inputs
            const xAOD::EventInfo *event = nullptr;
            ANA_CHECK (m_eventHandle.retrieve (event, sys));

            const xAOD::JetContainer *signaljets = nullptr;
            ANA_CHECK (m_signaljetHandle.retrieve (signaljets, sys));

            const xAOD::JetContainer *vbsjets = nullptr;
            ANA_CHECK (m_vbsjetHandle.retrieve (vbsjets, sys));

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
            for (const std::string &string_var: m_floatVariables) {
                m_Fbranches.at(string_var).set(*event, -99., sys);
            }

            for (const auto& var: m_intVariables) {
                m_Ibranches.at(var).set(*event, -99, sys);
            }

	    int n_jets = signaljets->size() + vbsjets->size();
            int nCentralJets = 0;
            int nForwardJets = 0;

            int n_electrons = electrons->size();
            int n_muons = muons->size();

            // b-jet sector
            bool WPgiven = !m_isBtag.empty();
            auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);

            // number of central jets
            for(const xAOD::Jet* jet : *signaljets) {
              // count central jets
              if (std::abs(jet->eta())<2.5) nCentralJets++;
              else nForwardJets++;

              if (WPgiven) {
                if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet);
              }
            }

	    for(const xAOD::Jet* vbsjet : *vbsjets) {
              if (std::abs(vbsjet->eta())<2.5) nCentralJets++;
              else nForwardJets++;
            }

            int n_bjets = bjets->size();

            m_Ibranches.at("nJets").set(*event, n_jets, sys);
            m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
            m_Ibranches.at("nMuons").set(*event, n_muons, sys);
            m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
            m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
            m_Ibranches.at("nForwardJets").set(*event, nForwardJets, sys);

            // selected leptons ;
            const xAOD::Electron* ele0 = n_electrons>=1 ? electrons->at(0) : nullptr;

            const xAOD::Muon* mu0 = n_muons>=1 ? muons->at(0) : nullptr;

            std::vector<std::pair<const xAOD::IParticle*, int>> leptons;
            if(ele0) leptons.emplace_back(ele0, -11*ele0->charge());
            if(mu0) leptons.emplace_back(mu0, -13*mu0->charge());

            int nLeptons = muons -> size() + electrons -> size();
            m_Ibranches.at("nLeptons").set(*event, nLeptons, sys);

            std::sort(leptons.begin(), leptons.end(),
                [](const std::pair<const xAOD::IParticle*, int>& a,
                   const std::pair<const xAOD::IParticle*, int>& b) {
                  return a.first->pt() > b.first->pt(); });

	    if( !leptons.empty() ){
              TLorentzVector tlv = leptons[0].first->p4();
	      int lep_pdgid = leptons[0].second;
              m_Fbranches.at("Lepton_pt").set(*event,  tlv.Pt(), sys);
              m_Fbranches.at("Lepton_eta").set(*event, tlv.Eta(), sys);
              m_Fbranches.at("Lepton_phi").set(*event, tlv.Phi(), sys);
              m_Fbranches.at("Lepton_E").set(*event, tlv.E(), sys);
	      m_Ibranches.at("Lepton_pdgid").set(*event, lep_pdgid, sys);
            }

            //jet sector
            for (std::size_t i=0; i<std::min(signaljets->size(),(std::size_t)2); i++){
              m_Fbranches.at("Jet"+std::to_string(i+1)+"_pt").set(*event, signaljets->at(i)->pt(), sys);
              m_Fbranches.at("Jet"+std::to_string(i+1)+"_eta").set(*event, signaljets->at(i)->eta(), sys);
              m_Fbranches.at("Jet"+std::to_string(i+1)+"_phi").set(*event, signaljets->at(i)->phi(), sys);
              m_Fbranches.at("Jet"+std::to_string(i+1)+"_E").set(*event, signaljets->at(i)->e(), sys);
            }

            //b-jet sector
            for (std::size_t i=0; i<std::min(bjets->size(),(std::size_t)2); i++){
              m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_pt").set(*event, bjets->at(i)->pt(), sys);
              m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_eta").set(*event, bjets->at(i)->eta(), sys);
              m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_phi").set(*event, bjets->at(i)->phi(), sys);
              m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_E").set(*event, bjets->at(i)->e(), sys);
            }

            if (bjets->size() >=2){
              TLorentzVector bb = bjets->at(0)->p4()+bjets->at(1)->p4();
              m_Fbranches.at("mbb").set(*event, bb.M(), sys);
              m_Fbranches.at("pTbb").set(*event, bb.Pt(), sys);
              m_Fbranches.at("Etabb").set(*event, bb.Eta(), sys);
              m_Fbranches.at("Phibb").set(*event, bb.Phi(), sys);
              m_Fbranches.at("dRbb").set(*event, (bjets->at(0)->p4()).DeltaR(bjets->at(1)->p4()), sys);
              m_Fbranches.at("dPhibb").set(*event, (bjets->at(0)->p4()).DeltaPhi(bjets->at(1)->p4()), sys);
              m_Fbranches.at("dEtabb").set(*event, (bjets->at(0)->eta()) - bjets->at(1)->eta(), sys);
            }

            // kinematics of vbs jets
            if (vbsjets->size() >= 2){
              const xAOD::Jet* vbsJet1 = vbsjets->at(0);
              const xAOD::Jet* vbsJet2 = vbsjets->at(1);

              TLorentzVector vbs_jj = vbsJet1->p4() + vbsJet2->p4();

              m_Fbranches.at("mjj").set(*event, vbs_jj.M(), sys);
              m_Fbranches.at("pTjj").set(*event, vbs_jj.Pt(), sys);
              m_Fbranches.at("Etajj").set(*event, vbs_jj.Eta(), sys);
              m_Fbranches.at("Phijj").set(*event, vbs_jj.Phi(), sys);
              m_Fbranches.at("dRjj").set(*event, (vbsJet1->p4()).DeltaR(vbsJet2->p4()), sys);
              m_Fbranches.at("dEtajj").set(*event, (vbsJet1->eta())-vbsJet2->eta(), sys);
              m_Fbranches.at("dPhijj").set(*event, (vbsJet1->p4()).DeltaPhi(vbsJet2->p4()), sys);

            }
       
       	}
 
 	return StatusCode::SUCCESS;
    } 
}
