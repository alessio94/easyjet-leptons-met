/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TruthJetSelectorAlg.h"
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace Easyjet
{
  TruthJetSelectorAlg ::TruthJetSelectorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode TruthJetSelectorAlg ::initialize()
  {
    // Read input/output handles
    ATH_CHECK (m_inHandleKey.initialize());
    ATH_CHECK (m_eventHandleKey.initialize());
    ATH_CHECK (m_outHandleKey.initialize());

    // Intialise input/output decorators  
    ATH_CHECK (m_nSelPartKey.initialize());

    ATH_CHECK(m_truthLabelDecorKey.initialize());
    
    for(int i=0; i<m_bjetAmount; i++){ // For b-jets
      std::string index = std::to_string(i+1);
      std::string key = m_inHandleKey.key() + ".istruthbjet" + index;
      SG::WriteDecorHandleKey<xAOD::JetContainer> DecorKey(key);
      m_bleadBranchesKeys.emplace("istruthbjet"+index, std::move(DecorKey));
      ATH_CHECK(m_bleadBranchesKeys.at("istruthbjet"+index).initialize());
    }

    for(int i=0; i<m_cjetAmount; i++){ // For c-jets
      std::string index = std::to_string(i+1);
      std::string key = m_inHandleKey.key() + ".istruthcjet" + index;
      SG::WriteDecorHandleKey<xAOD::JetContainer> DecorKey(key);
      m_cleadBranchesKeys.emplace("istruthcjet"+index, std::move(DecorKey));
      ATH_CHECK(m_cleadBranchesKeys.at("istruthcjet"+index).initialize());
    }

    ATH_CHECK (m_isSelectedJetKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TruthJetSelectorAlg ::execute()
  {
        // Retrieve inputs

    SG::ReadHandle<xAOD::JetContainer> inContainer(m_inHandleKey);
    SG::ReadHandle<xAOD::TruthEventContainer> eventContainer(m_eventHandleKey);
    SG::WriteDecorHandle<xAOD::JetContainer, bool> m_isSelectedJet(m_isSelectedJetKey);
    SG::WriteDecorHandle<xAOD::TruthEventContainer, int> m_nSelPart(m_nSelPartKey);
    SG::WriteHandle<ConstDataVector<xAOD::JetContainer>> m_outHandle(m_outHandleKey);

    
    // // Only construct and use m_truthLabel if the container has a truth label decoration
    std::unique_ptr<SG::ReadDecorHandle<xAOD::JetContainer, int>> m_truthLabel;
    if (m_hasTruthLabel) {
        m_truthLabel = std::make_unique<SG::ReadDecorHandle<xAOD::JetContainer, int>>(m_truthLabelDecorKey);
    }

    const xAOD::TruthEvent *event= eventContainer->at(0);

    // Setup output 
    auto workContainer =
      std::make_unique<ConstDataVector<xAOD::JetContainer> >(
          SG::VIEW_ELEMENTS);
    
    // loop over jets
    for (const xAOD::Jet *jet : *inContainer)
    {

      m_isSelectedJet(*jet) = false; // default to not selected
      // cuts
      if (jet->pt() < m_minPt || std::abs(jet->eta()) > m_maxEta)
        continue;
      // cuts for calibrated large-R jet
      if (m_maxPt > 0 && jet->pt() > m_maxPt)
        continue;
      if (m_maxMass > 0 && jet->m() > m_maxMass)
        continue;
      if (m_minMass > 0 && jet->m() < m_minMass)
        continue;
      
      workContainer->push_back(jet);
      
      m_isSelectedJet(*jet) = true;
    }
    
    int nJets = workContainer->size();
    m_nSelPart(*event) = nJets;
    // if we have more or less than the requested numbers, empty the workcontainer to write
    // defaults/return empty container
    bool over_maximum = m_maximumAmount > 0 && nJets > m_maximumAmount;
    if ( nJets < m_minimumAmount || over_maximum)
    {
      workContainer->clear();
      nJets = 0;
    }
    
    // sort and truncate
    int nKeep;
    if (nJets < m_truncateAtAmount || m_truncateAtAmount == -1) nKeep = nJets; // if we give -1, sort the whole container
    else nKeep = m_truncateAtAmount;
    
    if (m_pTsort) {
      std::partial_sort(
        workContainer->begin(), // Iterator from which to start sorting
        workContainer->begin() + nKeep, // Use begin + N to sort first N
        workContainer->end(), // Iterator marking the end of range to sort
          [&](const xAOD::IParticle *left, const xAOD::IParticle *right) {
            return left->pt() > right->pt();
          });
      // keep only the requested amount
      workContainer->erase(workContainer->begin() + nKeep, workContainer->end());
    }

    //lead/sublead bjet
    if(m_bjetAmount > 0 && m_hasTruthLabel){
      // Create WriteDecorHandles for each key
      std::unordered_map<std::string, SG::WriteDecorHandle<xAOD::JetContainer, bool>> m_bleadBranches;
      for (const auto& kv : m_bleadBranchesKeys) {
          m_bleadBranches.emplace(kv.first, SG::WriteDecorHandle<xAOD::JetContainer, bool>(kv.second));
      }
      int njet = 0;
      for (const xAOD::Jet *jet : *workContainer) {
        int truthLabel = (*m_truthLabel)(*jet);
        if (truthLabel == 5 && std::abs(jet->eta())<2.5){
            njet++;
            m_bleadBranches.at("istruthbjet"+std::to_string(njet))(*jet) = true;
        }
        if ( njet == m_bjetAmount ) break;
      }
    }

    //lead/sublead cjet
    if(m_cjetAmount > 0 && m_hasTruthLabel){
      std::unordered_map<std::string, SG::WriteDecorHandle<xAOD::JetContainer, bool>> m_cleadBranches;
      for (const auto& kv : m_cleadBranchesKeys) {
          m_cleadBranches.emplace(kv.first, SG::WriteDecorHandle<xAOD::JetContainer, bool>(kv.second));
      }
      int njet = 0;
      for (const xAOD::Jet *jet : *workContainer) {
        int truthLabel = (*m_truthLabel)(*jet);
        if (truthLabel == 4 && std::abs(jet->eta())<2.5){
            njet++;
            m_cleadBranches.at("istruthcjet"+std::to_string(njet))(*jet) = true;
        }
        if ( njet == m_cjetAmount ) break;
      }
    }

    // Write to eventstore
    ATH_CHECK(m_outHandle.record(std::move(workContainer)));   
    

    return StatusCode::SUCCESS;
  }
}
