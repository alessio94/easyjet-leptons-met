/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "AthContainers/AuxElement.h"
#include "BaselineVarsttHHAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>

namespace ttHH
{
  BaselineVarsttHHAlg::BaselineVarsttHHAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsttHHAlg::initialize()
  {
    ATH_CHECK(m_smallRJets_BTag_ContainerInKey.initialize());
    ATH_CHECK(m_smallRJets_ContainerInKey.initialize());
    ATH_CHECK(m_muonContainerInKey.initialize());
    ATH_CHECK(m_electronContainerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());

    for (const std::string &var : m_vars)
    {
      std::string deco_var = var; 
      SG::AuxElement::Decorator<float> deco(deco_var);
      m_decos.emplace(deco_var, deco);
    };

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsttHHAlg::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    for (const std::string &var : m_vars)
    {
      std::string deco_var = var; 
      m_decos.at(deco_var)(*eventInfo) = -99.; 
    };


    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRJets_BTag(
       m_smallRJets_BTag_ContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRJets(
       m_smallRJets_ContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::MuonContainer> > muons_(
        m_muonContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::ElectronContainer> > electrons_(
        m_electronContainerInKey);

    ATH_CHECK(smallRJets_BTag.isValid());
    ATH_CHECK(smallRJets.isValid());
    ATH_CHECK(muons_.isValid());
    ATH_CHECK(electrons_.isValid());
    ConstDataVector<xAOD::JetContainer> btag_jets = *smallRJets_BTag;
    
    int n_leptons = (*electrons_).size() + (*muons_).size();
    
    double HT = 0; // scalar sum of jet pT

    
    // Fill electron variables
    /*for (const xAOD::Electron *electron : *electrons_)
    {
      ...
      }*/

    // Fill muon variables
    /*for (const xAOD::Muon *muon : *muons_)
    {
      ...
      }*/

    if (smallRJets_BTag->size()>=4)
    {
      // fill all b-jet kinematics for at least 6 b-jets
      for (std::size_t i=0; i<std::min(smallRJets_BTag->size(),(std::size_t)6); i++){	

        m_decos.at("Jet_pt_B" + std::to_string(i+1))(*eventInfo) = btag_jets[i]->pt();
        m_decos.at("Jet_eta_B"+ std::to_string(i+1))(*eventInfo) = btag_jets[i]->eta();
        m_decos.at("Jet_phi_B"+ std::to_string(i+1))(*eventInfo) = btag_jets[i]->phi();
        m_decos.at("Jet_E_B"+ std::to_string(i+1))(*eventInfo) = btag_jets[i]->e();
      }        

      // construct Higgs Candidates
      xAOD::JetFourMom_t h1 = btag_jets[0]->jetP4() + btag_jets[1]->jetP4();
      xAOD::JetFourMom_t h2 = btag_jets[2]->jetP4() + btag_jets[3]->jetP4();

      // calculate deltaR, deltaPhi and deltaEta for 12, 34, 56 b-jet combinations
      auto [DeltaR, DeltaPhi, DeltaEta] = getPairKinematics(btag_jets); 

      m_decos.at("Jets_DeltaR12")(*eventInfo) = DeltaR[0];
      m_decos.at("Jets_DeltaR34")(*eventInfo) = DeltaR[1];
      m_decos.at("Jets_DeltaPhi12")(*eventInfo) = DeltaPhi[0];
      m_decos.at("Jets_DeltaPhi34")(*eventInfo) = DeltaPhi[1];
      m_decos.at("Jets_DeltaEta12")(*eventInfo) = DeltaEta[0];
      m_decos.at("Jets_DeltaEta34")(*eventInfo) = DeltaEta[1];

      m_decos.at("H1_m")(*eventInfo) = h1.M();
      m_decos.at("H1_pT")(*eventInfo) = h1.Pt();
      m_decos.at("H1_eta")(*eventInfo) = h1.Eta();
      m_decos.at("H1_phi")(*eventInfo) = h1.Phi();

      m_decos.at("H2_m")(*eventInfo) = h2.M();
      m_decos.at("H2_pT")(*eventInfo) = h2.Pt();
      m_decos.at("H2_eta")(*eventInfo) = h2.Eta();
      m_decos.at("H2_phi")(*eventInfo) = h2.Phi();

      //m_decos.at("HH_m")(*eventInfo) = (h1 + h2).M();

       
      // Create a new JetContainer
      xAOD::Jet jj12 = xAOD::Jet();
      jj12 = *btag_jets[0]; // TODO: breaks if jj12 is empty, not sure what it the best approach...
      xAOD::Jet jj34 = xAOD::Jet();
      jj34 = *btag_jets[0];

      jj12.setJetP4(xAOD::JetFourMom_t(h1.Pt(), h1.Eta(), h1.Phi(), h1.M()));
      jj34.setJetP4(xAOD::JetFourMom_t(h2.Pt(), h2.Eta(), h2.Phi(), h2.M()));

      // calculate deltaR, deltaPhi and deltaEta for jj12_jj34, jj34_jj56, jj56_jj12 combinations
      float deltaR_1234 = xAOD::P4Helpers::deltaR(jj12, jj34);
      DeltaR.push_back(deltaR_1234);
      m_decos.at("Jets_DeltaR1234")(*eventInfo) = deltaR_1234;

      float deltaEta_1234 = xAOD::P4Helpers::deltaEta(jj12, jj34);
      DeltaEta.push_back(deltaEta_1234);
      m_decos.at("Jets_DeltaEta1234")(*eventInfo) = deltaEta_1234;

      float deltaPhi_1234 = xAOD::P4Helpers::deltaPhi(jj12, jj34);
      m_decos.at("Jets_DeltaPhi1234")(*eventInfo) = deltaPhi_1234;

      // do the same if we have more than 5 jets
      if (btag_jets.size() > 5)
      {
      	m_decos.at("Jets_DeltaR56")(*eventInfo) = DeltaR[2];
	m_decos.at("Jets_DeltaPhi56")(*eventInfo) = DeltaPhi[2];
	m_decos.at("Jets_DeltaEta56")(*eventInfo) = DeltaEta[2];

        // construct 56 jet combination
        xAOD::JetFourMom_t jj56_p4 = btag_jets[4]->jetP4() + btag_jets[5]->jetP4();
        xAOD::Jet jj56 = xAOD::Jet();
        jj56 = *btag_jets[0];
        jj56.setJetP4(jj56_p4);

        float deltaR_5612 = xAOD::P4Helpers::deltaR(jj56, jj12);
        DeltaR.push_back(deltaR_5612);
        m_decos.at("Jets_DeltaR5612")(*eventInfo) = deltaR_5612;

        float deltaEta_5612 = xAOD::P4Helpers::deltaEta(jj56, jj12);
        DeltaEta.push_back(deltaEta_5612);
        m_decos.at("Jets_DeltaEta5612")(*eventInfo) = deltaEta_5612;      

        float deltaPhi_5612 = xAOD::P4Helpers::deltaPhi(jj56, jj12);
        m_decos.at("Jets_DeltaPhi5612")(*eventInfo) = deltaPhi_5612;

        float deltaR_3456 = xAOD::P4Helpers::deltaR(jj34, jj56);
        DeltaR.push_back(deltaR_3456);
        m_decos.at("Jets_DeltaR3456")(*eventInfo) = deltaR_3456;

        float deltaEta_3456 = xAOD::P4Helpers::deltaEta(jj34, jj56);
        DeltaEta.push_back(deltaEta_3456);
        m_decos.at("Jets_DeltaEta3456")(*eventInfo) = deltaEta_3456;
      
        float deltaPhi_3456 = xAOD::P4Helpers::deltaPhi(jj34, jj56);
        //DeltaPhi.push_back(deltaPhi_3456);
        m_decos.at("Jets_DeltaPhi3456")(*eventInfo) = deltaPhi_3456;
      } 

      // calculate max, min and mean of mass, deltaEta and deltaR
      auto [DeltaRMax, DeltaRMin, DeltaRMean] = calculateVectorStats(DeltaR);
      auto [DeltaEtaMax, DeltaEtaMin, DeltaEtaMean] = calculateVectorStats(DeltaEta);

      m_decos.at("Jets_DeltaRMax")(*eventInfo) = DeltaRMax;
      m_decos.at("Jets_DeltaRMin")(*eventInfo) = DeltaRMin;
      m_decos.at("Jets_DeltaRMean")(*eventInfo) = DeltaRMean;

      m_decos.at("Jets_DeltaEtaMax")(*eventInfo) = DeltaEtaMax;
      m_decos.at("Jets_DeltaEtaMin")(*eventInfo) = DeltaEtaMin;
      m_decos.at("Jets_DeltaEtaMean")(*eventInfo) = DeltaEtaMean;
    }

    m_decos.at("njets")(*eventInfo) = smallRJets->size();
    m_decos.at("nBjets")(*eventInfo) = smallRJets_BTag->size();


    for (const xAOD::Jet *jet : *smallRJets) // Jets here can be every type of jet (No Working point selected)
    {
      HT += jet->pt();
    }
    m_decos.at("HT")(*eventInfo) = HT;

    // Save cutflow booleans
    m_decos.at("n_leptons")(*eventInfo) = n_leptons; 

    return StatusCode::SUCCESS;
  }

  std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> BaselineVarsttHHAlg::getPairKinematics(const ConstDataVector<xAOD::JetContainer>& jetPairs)
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
}
