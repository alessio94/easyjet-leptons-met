/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "JetPairingAlgttHH.h"
#include "AthContainers/AuxElement.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "xAODJet/JetContainer.h"
#include <AthContainers/ConstDataVector.h>

namespace ttHH
{
  JetPairingAlgttHH ::JetPairingAlgttHH(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("pairingStrategy", m_pairingStrategy);
  }

  StatusCode JetPairingAlgttHH ::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("        JetPairingAlgttHH        \n");
    ATH_MSG_INFO("*********************************\n");

    // Read syst-aware input/output handles
    ATH_CHECK (m_inHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));


    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode JetPairingAlgttHH ::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrive inputs
      const xAOD::JetContainer *inContainer = nullptr;
      ANA_CHECK (m_inHandle.retrieve (inContainer, sys));    

      // fill workContainer with "views" of the inContainer
      // see TJ's tutorial for this
      auto workContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          inContainer->begin(), inContainer->end(), SG::VIEW_ELEMENTS);

      // this assumes that container is pt sorted (use the JetSelectorAlg for
      // this) and checks if we have at least 4 jets otherwise exit this alg
      if (m_pairingStrategy == "minDeltaR" && workContainer->size() >= 4)
      {
        // calculate dR to the next three leading jets and decorate jet
        const SG::AuxElement::Decorator<float> dRtoLeadingJet_dec(
            "dRtoLeadingJet");
        const SG::AuxElement::ConstAccessor<float> dRtoLeadingJet_acc(
            "dRtoLeadingJet");

        // decorate dR(jet,leading jet) to each jet
        bool firstJet = true;
        for (const xAOD::Jet *jet : *workContainer)
        {
          // more instructive than done with iterators
          if (firstJet)
          {
            dRtoLeadingJet_dec(*jet) = 0;
            firstJet = false;
            continue;
          }
          dRtoLeadingJet_dec(*jet) =
              xAOD::P4Helpers::deltaR(jet, (*workContainer)[0]);
        }

        // now sort them for their closeness
        std::partial_sort(
            workContainer->begin(),     // Iterator from which to start sorting
            workContainer->begin() + 4, // Use begin + N to sort first N
            workContainer->end(),       // Iterator marking the end of the range
            [dRtoLeadingJet_dec, dRtoLeadingJet_acc](
                const xAOD::IParticle *left, const xAOD::IParticle *right)
            { return dRtoLeadingJet_acc(*left) < dRtoLeadingJet_acc(*right); });

        // lets return the pairing of the leading (h1) and subleading (h2) Higgs
        // candidates as four jets in the order:
        // h1_leading_pt_jet
        // h1_subleading_pt_jet
        // h2_leading_pt_jet
        // h2_subleading_pt_jet
        if ((*workContainer)[2]->pt() < (*workContainer)[3]->pt())
        {
          // no swap method on ConstDataVector, so by hand
          const xAOD::Jet *temp = (*workContainer)[2];
          (*workContainer)[2] = (*workContainer)[3];
          (*workContainer)[3] = temp;
        }
        // keep only the higgs candidate ones to avoid confusion
        workContainer->erase(workContainer->begin() + 4, workContainer->end());
      } else if (m_pairingStrategy == "chiSquare" && workContainer->size() >= 4)
      {	    
          auto [hh_bJets, chi_hh] = bJetChiSquarePairing(*workContainer, m_targetMass1, m_targetMass2);

        // TODO: change this!
          workContainer->erase(workContainer->begin() + 4, workContainer->end());
       (*workContainer)[0] = hh_bJets[0];
       (*workContainer)[1] = hh_bJets[1];
       (*workContainer)[2] = hh_bJets[2];
       (*workContainer)[3] = hh_bJets[3];
      }

      // Write to eventstore
      ATH_CHECK(m_outHandle.record(std::move(workContainer), sys));   
    }
    return StatusCode::SUCCESS;
  }

std::tuple<std::vector<const xAOD::Jet*>, float> JetPairingAlgttHH ::bJetChiSquarePairing(const ConstDataVector<xAOD::JetContainer>& Jets, float target_mass_1, float target_mass_2)
  {
    // This method checks for each bJet combination, and save that combinations that
    // are closer to the target mass based on the CHI^2
  
    float chi_min = std::numeric_limits<float>::infinity();
    std::vector<const xAOD::Jet*> outJets;
    outJets.resize(4);
 
    // Iterate over all combinations of pairs in the jet container
    for(size_t i = 0; i < Jets.size(); i++) {
      for(size_t j = i + 1; j < Jets.size(); j++) {
        for (size_t k = j + 1; k < Jets.size(); ++k) {
          for (size_t l = k + 1; l < Jets.size(); ++l) {
          
            std::vector<std::vector<size_t>> permutations;
            // if the target masses are not the same we also need to test for mX_12 <-> mX_34
            if (target_mass_1 != target_mass_2) {
              permutations = {{i,j,k,l}, {i,l,j,k}, {i,k,j,l}, {k,l,i,j}, {j,k,i,l}, {j,l,i,k}};
            } else {
              permutations = {{i,j,k,l}, {i,l,j,k}, {i,k,j,l}};
            }
          
            for (auto& perm : permutations) {
              //std::cout << "jet pT: " << Jets.at(perm[0])->pt() << " " << Jets.at(perm[1])->pt() <<  " " << Jets.at(perm[2])->pt() << " " << Jets.at(perm[3])->pt() << std::endl;
            
              auto [jet1, jet2, jet3, jet4, chi_squared] = minChiSquared(Jets, perm, target_mass_1, target_mass_2);
            
              // minimize the CHI squared
              if (chi_squared < chi_min) {
                chi_min = chi_squared;
                outJets.at(0) = jet1;
                outJets.at(1) = jet2;
                outJets.at(2) = jet3;
                outJets.at(3) = jet4;
              }
            
              //std::cout << "chi_squared " << chi_squared << std::endl;
            }
	  }
        }
      }
    }

    //std::cout << "chi_min " << chi_min << std::endl;

    // sort the two pairs by pT
    if (outJets[0]->pt() < outJets[1]->pt()) {
      std::swap(outJets[0], outJets[1]);
    }
    if (outJets[2]->pt() < outJets[3]->pt()) {
      std::swap(outJets[2], outJets[3]);
    }

    return {outJets, chi_min};
  }


std::tuple<const xAOD::Jet*, const xAOD::Jet*, const xAOD::Jet*, const xAOD::Jet*, float> JetPairingAlgttHH ::minChiSquared(const ConstDataVector<xAOD::JetContainer>& Jets, const std::vector<size_t>& indexes, float target_mass_1, float target_mass_2) 
  {
    auto jet1 = Jets.at(indexes.at(0));
    auto jet2 = Jets.at(indexes.at(1));
    auto jet3 = Jets.at(indexes.at(2));
    auto jet4 = Jets.at(indexes.at(3));
  
    // construct the jet pair candidates
    xAOD::JetFourMom_t p1 = jet1->jetP4() + jet2->jetP4();
    xAOD::JetFourMom_t p2 = jet3->jetP4() + jet4->jetP4();
  
    // retrive the invariant mass of the jet pair
    float mX_12 = p1.M();
    float mX_34 = p2.M();
  
    //std::cout << "mX_12 " << mX_12 << " mX_34 " << mX_34 << std::endl;
  
    // ratio between target mass and invariant mass of the jet pair
    float r_12 = (1 - mX_12/target_mass_1);
    float r_34 = (1 - mX_34/target_mass_2);
  
    // calculate the CHI squared
    float chi_squared = 10 * ( r_12 * r_12 + r_34 * r_34 );
  
    return {jet1, jet2, jet3, jet4, chi_squared};
  }
}
