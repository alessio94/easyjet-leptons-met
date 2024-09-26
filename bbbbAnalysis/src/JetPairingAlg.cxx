/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "JetPairingAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>
#include "FourMomUtils/xAODP4Helpers.h"
#include "PathResolver/PathResolver.h"
#include "TFile.h"
#include <vector>
#include <cmath>

namespace HH4B
{
  JetPairingAlg ::JetPairingAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("kinematicGroup", m_bdt_kinematicGroup);
  }

  StatusCode JetPairingAlg ::initialize()
  {
    ATH_CHECK(m_inJetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_outJetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventInfoKey.initialize());

    // Convert string to enum
    // Pairing strategy
    m_pairing_strategy = stringtoPairingStrategy(m_pairingStrategy);

    // Load BDT models
    std::vector<std::string> trainingFiles = {"Odd", "Even"};
    for (std::string &file: trainingFiles){
      std::unique_ptr<MVAUtils::BDT> bdt;
      loadJetPairingBDT(m_bdt_kinematicGroup, file, bdt);
      m_pairing_bdts.push_back(std::move(bdt));
    }
    if (m_pairing_bdts.size()!=2){
      ATH_MSG_ERROR("2 BDTs are required: 1 for odd and the other for even events");
      return StatusCode::FAILURE;
    }

    ATH_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode JetPairingAlg ::execute()
  {
    // event info container used for selecting xml file to read for BDT pairing
    SG::ReadHandle<xAOD::EventInfo> evtInfoContainer(
        m_eventInfoKey);
    ATH_CHECK(evtInfoContainer.isValid());
    uint64_t event_number = evtInfoContainer->eventNumber();
    // Loop over all systs
    for (const auto &sys : m_systematicsList.systematicsVector())
    {
      // Retrieve inputs
      const xAOD::JetContainer *inContainer = nullptr;
      ATH_CHECK(m_inJetHandle.retrieve(inContainer, sys));

      // fill workContainer with "views" of the inContainer
      // see TJ's tutorial for this
      auto workContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          inContainer->begin(), inContainer->end(), SG::VIEW_ELEMENTS);

      // this assumes that container is pt sorted (use the JetSelectorAlg for
      // this) and checks if we have at least 4 jets otherwise exit this alg
      if (m_pairing_strategy == HH4B::PairingStrategy::minDeltaR && workContainer->size() >= 4)
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
      else if(m_pairing_strategy == HH4B::PairingStrategy::BDT && workContainer->size() >= 4){
        // Calculate the variables that will be input to the BDT and get the BDT scores for each pairing (assign BDT score by applying the weights according to training group (compressed or noncompressed) and event number (odd or even))
        std::vector<JetSet> jetSets(3);
        std::vector<std::array<int, 4>> indexMappings = {
          {0, 1, 2, 3},  // For pairing index = 0
          {0, 2, 1, 3},  // For pairing index = 1
          {0, 3, 1, 2}   // For pairing index = 2
        };
        std::vector<std::array<int, 4>> indexInverseMappings = {
          {2, 3, 0, 1},  // For pairing index = 0 and m_bb1 > m_bb2
          {1, 3, 0, 2},  // For pairing index = 1 and m_bb1 > m_bb2
          {1, 2, 0, 3}   // For pairing index = 2 and m_bb1 > m_bb2
        };

        for(size_t i=0; i<3; ++i){
          JetSet& jetSet = jetSets[i];
          std::array<int, 4> mapping; 
          if (i < indexMappings.size()) mapping = indexMappings[i];
          else {
            ATH_MSG_ERROR("Invalid index for JetSet i = " << i);
            continue;
          }

          xAOD::JetFourMom_t tlv1 = (*workContainer)[mapping[0]]->jetP4();
          xAOD::JetFourMom_t tlv2 = (*workContainer)[mapping[1]]->jetP4();
          xAOD::JetFourMom_t tlv3 = (*workContainer)[mapping[2]]->jetP4();
          xAOD::JetFourMom_t tlv4 = (*workContainer)[mapping[3]]->jetP4();

          jetSet.dr1 = xAOD::P4Helpers::deltaR((*workContainer)[mapping[0]],(*workContainer)[mapping[1]]);
          jetSet.dr2 = xAOD::P4Helpers::deltaR((*workContainer)[mapping[2]],(*workContainer)[mapping[3]]);
          jetSet.deta1 = std::abs(tlv1.Eta() - tlv2.Eta());
          jetSet.deta2 = std::abs(tlv3.Eta() - tlv4.Eta());
          jetSet.dphi1 = std::abs((workContainer->at(mapping[0])->p4()).DeltaPhi(workContainer->at(mapping[1])->p4()));
          jetSet.dphi2 = std::abs((workContainer->at(mapping[2])->p4()).DeltaPhi(workContainer->at(mapping[3])->p4()));
          jetSet.jet_system_pt_1 = (tlv1+tlv2).Pt();
          jetSet.jet_system_pt_2 = (tlv3+tlv4).Pt();
          jetSet.jet_system_mass_1 = (tlv1+tlv2).M();
          jetSet.jet_system_mass_2 = (tlv3+tlv4).M();
          jetSet.systems_mass_difference = std::abs((tlv1+tlv2).M()-(tlv3+tlv4).M());
          jetSet.m4j = ((tlv1+tlv2+tlv3+tlv4).M());
          std::vector<float> vars(10, 0.0f);
          vars[HH4B::pairingBDTinputs::dEta1] = jetSet.deta1;
          vars[HH4B::pairingBDTinputs::dPhi1] = jetSet.dphi1;
          vars[HH4B::pairingBDTinputs::dR1] = jetSet.dr1;
          vars[HH4B::pairingBDTinputs::dEta2] = jetSet.deta2;
          vars[HH4B::pairingBDTinputs::dPhi2] = jetSet.dphi2;
          vars[HH4B::pairingBDTinputs::dR2] = jetSet.dr2;
          vars[HH4B::pairingBDTinputs::recoM4j] = jetSet.m4j;
          vars[HH4B::pairingBDTinputs::recoPtbb1] = jetSet.jet_system_pt_1;
          vars[HH4B::pairingBDTinputs::recoPtbb2] = jetSet.jet_system_pt_2;
          vars[HH4B::pairingBDTinputs::recoSHmassdiff] = jetSet.systems_mass_difference;
          if(event_number%2==0) jetSet.BDT_score = m_pairing_bdts.at(HH4B::pairingBDT::evenEvents)->GetClassification(vars);
          else jetSet.BDT_score = m_pairing_bdts.at(HH4B::pairingBDT::oddEvents)->GetClassification(vars);
          jetSets.push_back(jetSet);
        }

        // Select the pairing with the highest BDT score as the best pairing and add its index to the ntuples
        int myindex;
        int index;
        if(jetSets[0].BDT_score > jetSets[1].BDT_score)
          myindex = 0;
        else
          myindex = 1;

        if(jetSets[myindex].BDT_score > jetSets[2].BDT_score)
          index = myindex;
        else
          index = 2;

        // Sort them for their closeness and invariant mass (more massive scalar gets assigned as h1)
        const auto& mappingSelPairing = (jetSets[index].dr1>jetSets[index].dr2) ? indexMappings[index] : indexInverseMappings[index];
        const xAOD::Jet *temp1 = (*workContainer)[mappingSelPairing[0]];
        const xAOD::Jet *temp2 = (*workContainer)[mappingSelPairing[1]];
        const xAOD::Jet *temp3 = (*workContainer)[mappingSelPairing[2]];
        const xAOD::Jet *temp4 = (*workContainer)[mappingSelPairing[3]];

        // no swap method on ConstDataVector, so by hand
        (*workContainer)[0] = temp1;
        (*workContainer)[1] = temp2;
        (*workContainer)[2] = temp3;
        (*workContainer)[3] = temp4;

        // keep only the higgs candidate ones to avoid confusion
        workContainer->erase(workContainer->begin() + 4, workContainer->end());
      }
      ATH_CHECK(m_outJetHandle.record(std::move(workContainer), sys));
    }
    return StatusCode::SUCCESS;
  }

  PairingStrategy JetPairingAlg::stringtoPairingStrategy(const std::string& pairing_strategy_str) {
    if(pairing_strategy_str=="minDeltaR"){
      return HH4B::PairingStrategy::minDeltaR;
    }
    else if(pairing_strategy_str=="BDT"){
      return HH4B::PairingStrategy::BDT;
    }
    else {
      ATH_MSG_ERROR("Invalid vbfjets method string");
      return HH4B::PairingStrategy::invalid;
    }
  }

  void JetPairingAlg::loadJetPairingBDT(const std::string &kinematicBDTTrainingGroup, std::string &trainingFile, std::unique_ptr<MVAUtils::BDT> &bdt) {
    std::string bdt_root_file_path = PathResolverFindCalibFile("bbbbAnalysis/sh4b_resolved_bdtPairing_"+kinematicBDTTrainingGroup+"Group_for"+trainingFile+"Events.weights.root");
    std::unique_ptr<TFile> f = std::make_unique<TFile>(bdt_root_file_path.c_str(), "READ");
    if (!f || f->IsZombie()) {
      ATH_MSG_ERROR("Cannot open file \"" << bdt_root_file_path << "\" or the file is in a bad state.");
      return;
    }
    TTree *tree = nullptr;
    f->GetObject("BDT", tree);
    bdt = std::make_unique<MVAUtils::BDT>(tree);
    f->Close();
  }
}
