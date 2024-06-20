/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "JetPairingAlgttHH.h"

#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>
#include "FourMomUtils/xAODP4Helpers.h"

namespace ttHH
{
  JetPairingAlgttHH ::JetPairingAlgttHH(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("pairingStrategyName", m_pairingStrategyName);
  }

  StatusCode JetPairingAlgttHH ::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("        JetPairingAlgttHH        \n");
    ATH_MSG_INFO("*********************************\n");

    // Read syst-aware input/output handles
    ATH_CHECK (m_bjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    if(m_pairingStrategyName == "minDeltaR") m_pairingStrategy = ttHH::MinDeltaR;
    else if (m_pairingStrategyName == "chiSquare") m_pairingStrategy = ttHH::ChiSquare;
    else{
      ATH_MSG_ERROR("Unknown pairing strategy");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetPairingAlgttHH ::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrive inputs
      const xAOD::JetContainer *bjetContainer = nullptr;
      const xAOD::JetContainer *jetContainer = nullptr;
      ANA_CHECK (m_bjetHandle.retrieve (bjetContainer, sys));
      ANA_CHECK (m_jetHandle.retrieve (jetContainer, sys));    

      // fill workContainer with "views" of the jetContainer
      // see TJ's tutorial for this
      auto workContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          bjetContainer->begin(), bjetContainer->end(), SG::VIEW_ELEMENTS);

      // if we do not have 4 bjets we add non b-tagged jets to the pairing
      // which are sorted based on pcbt score
      if (bjetContainer->size() == 3) {
        for(const xAOD::Jet* jet : *jetContainer){
          // fill with first non b-tagged jet
          if (!m_isBtag.get(*jet, sys)) {
            workContainer->push_back(jet);
            break;
          }
        }
      }

      // this assumes that container is pt sorted (use the JetSelectorAlg for
      // this) and checks if we have at least 4 jets otherwise exit this alg
      switch (m_pairingStrategy) {
	case ttHH::MinDeltaR:
	  if (workContainer->size() >= 4)
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
	  }
          break;
        case ttHH::ChiSquare:
          if (workContainer->size() >= 4)
	  {	    

	      // perform b-jet pairing based on chi-square values
	      auto hh_bJets = bJetChiSquarePairing(*workContainer, m_targetMass1, m_targetMass2);

	      // return the pairing of the leading (h1) and subleading (h2) Higgs
	      // candidates as four jets in the order. The other jets in the container
	      // will stay in the back in their original order:
	      // {h1_leading_pt_jet, h1_subleading_pt_jet, h2_leading_pt_jet, h2_subleading_pt_jet, other jets...}
	      int pos = 0;
	      for (const auto& jet : hh_bJets) {
		  // find the iterator pointing to the current jet in workContainer
		  auto it = std::find(workContainer->begin(), workContainer->end(), jet);
		  // rotate the range from pos to the position of the current jet to move it to the 
		  // front but keep hh_bJets order
		  std::rotate(workContainer->begin() + pos, it, it + 1);
		  ++pos;
	      }
	  }
          break;
      }
      // Write to eventstore
      ATH_CHECK(m_outHandle.record(std::move(workContainer), sys));   
    }
    return StatusCode::SUCCESS;
  }

std::vector<const xAOD::Jet*> JetPairingAlgttHH ::bJetChiSquarePairing(const ConstDataVector<xAOD::JetContainer>& Jets, float targetMass1, float targetMass2)
  {
    // this method checks for each bJet combination, and save that combinations that
    // are closer to the target mass based on the CHI^2
  
    float chiMin = std::numeric_limits<float>::infinity();
    std::vector<const xAOD::Jet*> outJets;
    outJets.resize(4);

    // iterate over all combinations of pairs in the jet container
    for(size_t i = 0; i < Jets.size(); i++) {
      for(size_t j = i + 1; j < Jets.size(); j++) {
        for (size_t k = j + 1; k < Jets.size(); ++k) {
          for (size_t l = k + 1; l < Jets.size(); ++l) {
          
            std::vector<std::vector<size_t>> permutations;
            // if the target masses are not the same we also need to test for mX12 <-> mX_34
            if (std::abs(targetMass1-targetMass2)>std::numeric_limits<float>::epsilon()) {
              permutations = {{i,j,k,l}, {i,l,j,k}, {i,k,j,l}, {k,l,i,j}, {j,k,i,l}, {j,l,i,k}};
            } else {
              permutations = {{i,j,k,l}, {i,l,j,k}, {i,k,j,l}};
            }
          
            for (auto& perm : permutations) {

              auto [jet1, jet2, jet3, jet4, chiSquared] = minChiSquared(Jets, perm, targetMass1, targetMass2);
            
              // minimize the CHI squared
              if (chiSquared < chiMin) {
                chiMin = chiSquared;
                outJets.at(0) = jet1;
                outJets.at(1) = jet2;
                outJets.at(2) = jet3;
                outJets.at(3) = jet4;
              }
            }
	  }
        }
      }
    }

    // sort the two pairs by pT
    if (outJets[0]->pt() < outJets[1]->pt()) {
      std::swap(outJets[0], outJets[1]);
    }
    if (outJets[2]->pt() < outJets[3]->pt()) {
      std::swap(outJets[2], outJets[3]);
    }

    return outJets;
  }


std::tuple<const xAOD::Jet*, const xAOD::Jet*, const xAOD::Jet*, const xAOD::Jet*, float> JetPairingAlgttHH ::minChiSquared(const ConstDataVector<xAOD::JetContainer>& Jets, const std::vector<size_t>& indexes, float targetMass1, float targetMass2) 
  {
    auto jet1 = Jets.at(indexes.at(0));
    auto jet2 = Jets.at(indexes.at(1));
    auto jet3 = Jets.at(indexes.at(2));
    auto jet4 = Jets.at(indexes.at(3));
  
    // construct the jet pair candidates
    TLorentzVector p1 = jet1->p4() + jet2->p4();
    TLorentzVector p2 = jet3->p4() + jet4->p4();

    // retrive the invariant mass of the jet pair
    float mX12 = p1.M();
    float mX34 = p2.M();
  
    // ratio between target mass and invariant mass of the jet pair
    float r12 = (targetMass1 - mX12);
    float r34 = (targetMass2 - mX34);

    // calculate the CHI squared
    float chiSquared = r12 * r12 + r34 * r34;

    // If the two target masses are the same sort pair by pT
    if (std::abs(targetMass1-targetMass2)<std::numeric_limits<float>::epsilon() and p1.Pt() < p2.Pt()) {
        return {jet3, jet4, jet1, jet2, chiSquared};
    }
    return {jet1, jet2, jet3, jet4, chiSquared};
  }
}
