/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Giulia Di Gregorio, Luis Falda

#include "AthContainers/AuxElement.h"
#include "BaselineVarsttHHAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>

namespace ttHH
{
  BaselineVarsttHHAlg::BaselineVarsttHHAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  { }


  template<typename ParticleType>
  std::pair<int, int> BaselineVarsttHHAlg::truthOrigin(const ParticleType* particle) {
    static const SG::AuxElement::ConstAccessor<int> lepttruthOrigin("truthOrigin");
    static const SG::AuxElement::ConstAccessor<int> lepttruthType("truthType");
    
    return {lepttruthOrigin(*particle), lepttruthType(*particle)};
  }

  
  StatusCode BaselineVarsttHHAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       BaselineVarsttHHAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_bjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_HZPairsHandle.initialize(m_systematicsList));
    ATH_CHECK (m_ZZPairsHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

    // Intialise syst-aware output decorators

    // Add MC var
    if(m_isMC) m_Fvarnames.insert(m_Fvarnames.end(), m_Fvarnames_MC.begin(), m_Fvarnames_MC.end());

    for (const std::string &string_var: m_Fvarnames) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_Ivarnames) {
      CP::SysWriteDecorHandle<int> var {string_var+"_%SYS%", this};
      m_Ibranches.emplace(string_var, var);
      ATH_CHECK (m_Ibranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

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

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::JetContainer *HZPairs = nullptr;
      ANA_CHECK (m_HZPairsHandle.retrieve (HZPairs, sys));

      const xAOD::JetContainer *ZZPairs = nullptr;
      ANA_CHECK (m_ZZPairsHandle.retrieve (ZZPairs, sys));

      static const SG::AuxElement::ConstAccessor<int>  HadronConeExclTruthLabelID("HadronConeExclTruthLabelID");

      TLorentzVector H1(0, 0, 0, 0);
      TLorentzVector H2(0, 0, 0, 0);
      TLorentzVector HZ_H(0, 0, 0, 0);
      TLorentzVector HZ_Z(0, 0, 0, 0);
      TLorentzVector ZZ_Z1(0, 0, 0, 0);
      TLorentzVector ZZ_Z2(0, 0, 0, 0);
      TLorentzVector e1(0.,0.,0.,0.);
      TLorentzVector e2(0.,0.,0.,0.);
      TLorentzVector ee(0.,0.,0.,0.);
      TLorentzVector mu1(0.,0.,0.,0.);
      TLorentzVector mu2(0.,0.,0.,0.);
      TLorentzVector mumu(0.,0.,0.,0.);
      TLorentzVector emu(0.,0.,0.,0.);
      
      //auto btag_jets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      //auto btag_jets = *bjets;
      const xAOD::JetContainer btag_jets = *bjets;

      double HT = 0; // scalar sum of jet pT
      int truthLabel = -99;

      for (const std::string &string_var: m_Fvarnames) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }
      
      for (const std::string &string_var: m_Ivarnames) {
        m_Ibranches.at(string_var).set(*event, -99, sys);
      }

      if (bjets->size()>=4) {
        // fill all b-jet kinematics for at least 6 b-jets
        for (std::size_t i=0; i<std::min(bjets->size(),(std::size_t)6); i++){	

          if (m_isMC) truthLabel = HadronConeExclTruthLabelID(*bjets->at(i));

          m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_pt").set(*event, bjets->at(i)->p4().Pt(), sys);
          m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_eta").set(*event, bjets->at(i)->p4().Eta(), sys);
          m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_phi").set(*event, bjets->at(i)->p4().Phi(), sys);
          m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_E").set(*event, bjets->at(i)->p4().E(), sys);

          m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_truthLabel").set(*event, truthLabel, sys);
        }
 
        // Build the Higgs candidates
        H1 = bjets->at(0)->p4() + bjets->at(1)->p4();
        H2 = bjets->at(2)->p4() + bjets->at(3)->p4();

        // TODO: should it be leading and subleading? 
        HZ_H = HZPairs->at(0)->p4() + HZPairs->at(1)->p4();
        HZ_Z = HZPairs->at(2)->p4() + HZPairs->at(3)->p4();

        ZZ_Z1 = ZZPairs->at(0)->p4() + ZZPairs->at(1)->p4();
        ZZ_Z2 = ZZPairs->at(2)->p4() + ZZPairs->at(3)->p4();

        auto [DeltaR, DeltaPhi, DeltaEta] = getPairKinematics(btag_jets);

        m_Fbranches.at("Jets_DeltaR12").set(*event, DeltaR[0], sys);
        m_Fbranches.at("Jets_DeltaR34").set(*event, DeltaR[1], sys);
        m_Fbranches.at("Jets_DeltaPhi12").set(*event, DeltaPhi[0], sys);
        m_Fbranches.at("Jets_DeltaPhi34").set(*event, DeltaPhi[1], sys);
        m_Fbranches.at("Jets_DeltaEta12").set(*event, DeltaEta[0], sys);
        m_Fbranches.at("Jets_DeltaEta34").set(*event, DeltaEta[1], sys);

        // Jet pairing variables
        m_Fbranches.at("H1_m").set(*event, H1.M(), sys);
        m_Fbranches.at("H1_pt").set(*event, H1.Pt(), sys);
        m_Fbranches.at("H1_eta").set(*event, H1.Eta(), sys);
        m_Fbranches.at("H1_phi").set(*event, H1.Phi(), sys);

        m_Fbranches.at("H2_m").set(*event, H2.M(), sys);
        m_Fbranches.at("H2_pt").set(*event, H2.Pt(), sys);
        m_Fbranches.at("H2_eta").set(*event, H2.Eta(), sys);
        m_Fbranches.at("H2_phi").set(*event, H2.Phi(), sys);

        m_Fbranches.at("HZ_H_m").set(*event, HZ_H.M(), sys);
        m_Fbranches.at("HZ_H_pt").set(*event, HZ_H.Pt(), sys);
        m_Fbranches.at("HZ_H_eta").set(*event, HZ_H.Eta(), sys);
        m_Fbranches.at("HZ_H_phi").set(*event, HZ_H.Phi(), sys);

        m_Fbranches.at("HZ_Z_m").set(*event, HZ_Z.M(), sys);
        m_Fbranches.at("HZ_Z_pt").set(*event, HZ_Z.Pt(), sys);
        m_Fbranches.at("HZ_Z_eta").set(*event, HZ_Z.Eta(), sys);
        m_Fbranches.at("HZ_Z_phi").set(*event, HZ_Z.Phi(), sys);

        m_Fbranches.at("ZZ_Z1_m").set(*event, ZZ_Z1.M(), sys);
        m_Fbranches.at("ZZ_Z1_pt").set(*event, ZZ_Z1.Pt(), sys);
        m_Fbranches.at("ZZ_Z1_eta").set(*event, ZZ_Z1.Eta(), sys);
        m_Fbranches.at("ZZ_Z1_phi").set(*event, ZZ_Z1.Phi(), sys);

        m_Fbranches.at("ZZ_Z2_m").set(*event, ZZ_Z2.M(), sys);
        m_Fbranches.at("ZZ_Z2_pt").set(*event, ZZ_Z2.Pt(), sys);
        m_Fbranches.at("ZZ_Z2_eta").set(*event, ZZ_Z2.Eta(), sys);
        m_Fbranches.at("ZZ_Z2_phi").set(*event, ZZ_Z2.Phi(), sys);

        m_Fbranches.at("HH_m").set(*event, (H1+H2).M(), sys);
        m_Fbranches.at("HZ_m").set(*event, (HZ_H+HZ_Z).M(), sys);
        m_Fbranches.at("ZZ_m").set(*event, (ZZ_Z1+ZZ_Z2).M(), sys);

        m_Fbranches.at("HH_CHI").set(*event, computeChiSquare(H1.M(), H2.M(), m_targetMassH, m_targetMassH, m_massResolution), sys);
        m_Fbranches.at("HZ_CHI").set(*event, computeChiSquare(HZ_H.M(), HZ_Z.M(), m_targetMassH, m_targetMassZ, m_massResolution), sys);
        m_Fbranches.at("ZZ_CHI").set(*event, computeChiSquare(ZZ_Z1.M(), ZZ_Z2.M(), m_targetMassZ, m_targetMassZ, m_massResolution), sys);

        // Create a new JetContainer
        xAOD::Jet jj12 = xAOD::Jet();
        jj12 = *btag_jets[0]; // TODO: breaks if jj12 is empty, not sure what it the best approach...
        xAOD::Jet jj34 = xAOD::Jet();
        jj34 = *btag_jets[0];

        jj12.setJetP4(xAOD::JetFourMom_t(H1.Pt(), H1.Eta(), H1.Phi(), H1.M()));
        jj34.setJetP4(xAOD::JetFourMom_t(H2.Pt(), H2.Eta(), H2.Phi(), H2.M()));

        // calculate deltaR, deltaPhi and deltaEta for jj12_jj34, jj34_jj56, jj56_jj12 combinations
        float deltaR_1234 = xAOD::P4Helpers::deltaR(jj12, jj34);
        DeltaR.push_back(deltaR_1234);
        m_Fbranches.at("Jets_DeltaR1234").set(*event, deltaR_1234, sys);

        float deltaEta_1234 = xAOD::P4Helpers::deltaEta(jj12, jj34);
        DeltaEta.push_back(deltaEta_1234);
        m_Fbranches.at("Jets_DeltaEta1234").set(*event, deltaR_1234, sys);

        float deltaPhi_1234 = xAOD::P4Helpers::deltaPhi(jj12, jj34);
        m_Fbranches.at("Jets_DeltaEta1234").set(*event, deltaPhi_1234, sys);

        if (btag_jets.size() > 5)
        {
          m_Fbranches.at("Jets_DeltaR56").set(*event, DeltaR[2], sys);
          m_Fbranches.at("Jets_DeltaPhi56").set(*event, DeltaPhi[2], sys);
          m_Fbranches.at("Jets_DeltaEta56").set(*event, DeltaEta[2], sys);

          // construct 56 jet combination
          xAOD::JetFourMom_t jj56_p4 = btag_jets[4]->jetP4() + btag_jets[5]->jetP4();
          xAOD::Jet jj56 = xAOD::Jet();
          jj56 = *btag_jets[0];
          jj56.setJetP4(jj56_p4);

          float deltaR_5612 = xAOD::P4Helpers::deltaR(jj56, jj12);
          DeltaR.push_back(deltaR_5612);
          m_Fbranches.at("Jets_DeltaR5612").set(*event, deltaR_5612, sys);

          float deltaEta_5612 = xAOD::P4Helpers::deltaEta(jj56, jj12);
          DeltaEta.push_back(deltaEta_5612);
          m_Fbranches.at("Jets_DeltaEta5612").set(*event, deltaEta_5612, sys);      

          float deltaPhi_5612 = xAOD::P4Helpers::deltaPhi(jj56, jj12);
          m_Fbranches.at("Jets_DeltaPhi5612").set(*event, deltaPhi_5612, sys);

          float deltaR_3456 = xAOD::P4Helpers::deltaR(jj34, jj56);
          DeltaR.push_back(deltaR_3456);
          m_Fbranches.at("Jets_DeltaR3456").set(*event, deltaR_3456, sys);

          float deltaEta_3456 = xAOD::P4Helpers::deltaEta(jj34, jj56);
          DeltaEta.push_back(deltaEta_3456);
          m_Fbranches.at("Jets_DeltaEta3456").set(*event, deltaEta_3456, sys);

          float deltaPhi_3456 = xAOD::P4Helpers::deltaPhi(jj34, jj56);
          m_Fbranches.at("Jets_DeltaPhi3456").set(*event, deltaPhi_3456, sys);
        }   

        // calculate max, min and mean of mass, deltaEta and deltaR
        auto [DeltaRMax, DeltaRMin, DeltaRMean] = calculateVectorStats(DeltaR);
        auto [DeltaEtaMax, DeltaEtaMin, DeltaEtaMean] = calculateVectorStats(DeltaEta);

        m_Fbranches.at("Jets_DeltaRMax").set(*event, DeltaRMax, sys);
        m_Fbranches.at("Jets_DeltaRMin").set(*event, DeltaRMin, sys);
        m_Fbranches.at("Jets_DeltaRMean").set(*event, DeltaRMean, sys);

        m_Fbranches.at("Jets_DeltaEtaMax").set(*event, DeltaEtaMax, sys);
        m_Fbranches.at("Jets_DeltaEtaMin").set(*event, DeltaEtaMin, sys);
        m_Fbranches.at("Jets_DeltaEtaMean").set(*event, DeltaEtaMean, sys);
      }

      // store electron kinematics
      if (electrons->size() >= 1) {
        const xAOD::Electron* ele1 = electrons->at(0);
        e1 = ele1->p4();
        m_Fbranches.at("Electron1_pt").set(*event, e1.Pt(), sys);
        m_Fbranches.at("Electron1_eta").set(*event, e1.Eta(), sys);
        m_Fbranches.at("Electron1_phi").set(*event, e1.Phi(), sys);
        m_Fbranches.at("Electron1_E").set(*event, e1.E(), sys);
        if (m_isMC){
          float ele_SF = m_ele_SF.get(*ele1, sys);
          m_Fbranches.at("Electron1_effSF").set(*event, ele_SF, sys);
	}
      }
      if (electrons->size() >= 2) {
        const xAOD::Electron* ele2 = electrons->at(1);
        e2 = ele2->p4();
        m_Fbranches.at("Electron2_pt").set(*event, e2.Pt(), sys);
        m_Fbranches.at("Electron2_eta").set(*event, e2.Eta(), sys);
        m_Fbranches.at("Electron2_phi").set(*event, e2.Phi(), sys);
        m_Fbranches.at("Electron2_E").set(*event, e2.E(), sys);
        if (m_isMC){
          float ele_SF = m_ele_SF.get(*ele2, sys);
          m_Fbranches.at("Electron2_effSF").set(*event, ele_SF, sys);
        }

        // ee
        e1 = electrons->at(0)->p4();
        ee = e1 + e2;
        m_Fbranches.at("ee_m").set(*event, ee.M(), sys);
        m_Fbranches.at("ee_pt").set(*event, ee.Pt(), sys);
        m_Fbranches.at("ee_eta").set(*event, ee.Eta(), sys);
        m_Fbranches.at("ee_phi").set(*event, ee.Phi(), sys);
        m_Fbranches.at("ee_dR").set(*event, (e1).DeltaR(e2), sys);
      }

      // store muon kinematics
      if (muons->size() >= 1) {
        const xAOD::Muon* muon1 = muons->at(0);
        mu1 = muon1->p4();
        m_Fbranches.at("Muon1_pt").set(*event, mu1.Pt(), sys);
        m_Fbranches.at("Muon1_eta").set(*event, mu1.Eta(), sys);
        m_Fbranches.at("Muon1_phi").set(*event, mu1.Phi(), sys);
        m_Fbranches.at("Muon1_E").set(*event, mu1.E(), sys);
        if (m_isMC){
          float mu_SF = m_mu_SF.get(*muon1, sys);
          m_Fbranches.at("Muon1_effSF").set(*event, mu_SF, sys);
        }
      }
      if (muons->size() >= 2) {
        const xAOD::Muon* muon2 = muons->at(1);
        mu2 = muon2->p4();
        m_Fbranches.at("Muon2_pt").set(*event, mu2.Pt(), sys);
        m_Fbranches.at("Muon2_eta").set(*event, mu2.Eta(), sys);
        m_Fbranches.at("Muon2_phi").set(*event, mu2.Phi(), sys);
        m_Fbranches.at("Muon2_E").set(*event, mu2.E(), sys);
        if (m_isMC){
          float mu_SF = m_mu_SF.get(*muon2, sys);
          m_Fbranches.at("Muon2_effSF").set(*event, mu_SF, sys);
        }

        // mumu
        mu1 = muons->at(0)->p4();
        mumu = mu1 + mu2;
        m_Fbranches.at("mumu_m").set(*event, mumu.M(), sys);
        m_Fbranches.at("mumu_pt").set(*event, mumu.Pt(), sys);
        m_Fbranches.at("mumu_eta").set(*event, mumu.Eta(), sys);
        m_Fbranches.at("mumu_phi").set(*event, mumu.Phi(), sys);
        m_Fbranches.at("mumu_dR").set(*event, (mu1).DeltaR(mu2), sys);
      }

      if (muons->size() >= 1 and electrons->size() >= 1) {
        mu1 = muons->at(0)->p4();
        e1 = electrons->at(0)->p4();
        emu = e1 + mu1;
        m_Fbranches.at("emu_m").set(*event, emu.M(), sys);
        m_Fbranches.at("emu_pt").set(*event, emu.Pt(), sys);
        m_Fbranches.at("emu_eta").set(*event, emu.Eta(), sys);
        m_Fbranches.at("emu_phi").set(*event, emu.Phi(), sys);
        m_Fbranches.at("emu_dR").set(*event, (mu1).DeltaR(e1), sys);
      }

      m_Ibranches.at("nJets").set(*event, jets->size(), sys);
      m_Ibranches.at("nBJets").set(*event, bjets->size(), sys);

      //----------------------------------------------------------
      //-- Multileptons

      size_t muonSize = muons->size();
      size_t electronSize = electrons->size();
      int leptonCount = muonSize + electronSize;
        
      if (leptonCount == 2){

        //-- total charge
        int totalCharge = 0;
        for (const auto& muon : *muons) 
          totalCharge += muon->charge();
        for (const auto& electron : *electrons) 
          totalCharge += electron->charge();
        
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
      m_Ibranches.at("trilept_type").set(*event, (leptonCount == 3) ? 1 : 0, sys);
      //--
      
      for (const xAOD::Jet *jet : *jets) // Jets here can be every type of jet (No Working point selected)
      {
        HT += jet->pt();
      }
      m_Fbranches.at("HT").set(*event, HT, sys);
      m_Ibranches.at("n_leptons").set(*event, muons->size() + electrons->size(), sys);
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


//-------------------------------------------------------------------------------------------
// Fill the branches Lepton*_*
  
template<typename ParticleType>
void BaselineVarsttHHAlg::updateLeptonBranch(const xAOD::EventInfo *event, int leptonIndex, const ParticleType* particle,  
                                       int lep_pdgid, float lep_sf, 
                                       const auto& sys) {

  
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

    // Truth
    auto [lep_truthOrigin, lep_truthType] = truthOrigin(particle);
    m_Ibranches.at(prefix + "truthOrigin").set(*event, lep_truthOrigin, sys);
    m_Ibranches.at(prefix + "truthType").set(*event, lep_truthType, sys);
    int lep_isPrompt = 0;
    
    if (lep_pdgid==13){ // simplistic
      if (lep_truthType==6) lep_isPrompt=1; // isolated prompts
    } else if (lep_pdgid==11){
      if (lep_truthType==2) lep_isPrompt=1; // isolated prompts
    }
    
    m_Ibranches.at(prefix + "isPrompt").set(*event, lep_isPrompt, sys);
    m_Ibranches.at(prefix + "isTight").set(*event, 1, sys);

}


}
