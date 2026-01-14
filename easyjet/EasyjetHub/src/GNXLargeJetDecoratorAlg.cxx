/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Giacomo Magni

#include "GNXLargeJetDecoratorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include "xAODJet/JetContainer.h"

namespace Easyjet
{
  GNXLargeJetDecoratorAlg::GNXLargeJetDecoratorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode GNXLargeJetDecoratorAlg::initialize()
  {
    ATH_CHECK (m_jetsInKey.initialize());

    for (auto& [name, key] : m_GN2XTauV00_decorKeys) {
        key =  m_jetsInKey.key() + ".GN2XTauV00_" + name;
        ATH_CHECK(key.initialize());
    }
    m_GN2XTauV00_htt_scoreDecorKey  = m_jetsInKey.key() + ".GN2XTauV00_htt_score";
    ATH_CHECK(m_GN2XTauV00_htt_scoreDecorKey.initialize());

    for (auto& [name, key] : m_GN3XPV01_decorKeys) {
        key =  m_jetsInKey.key() + ".GN3XPV01_" + name;
        ATH_CHECK(key.initialize());
    }
    m_GN3XPV01_htt_scoreDecorKey  = m_jetsInKey.key() + ".GN3XPV01_htt_score";
    ATH_CHECK(m_GN3XPV01_htt_scoreDecorKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode GNXLargeJetDecoratorAlg ::execute(const EventContext& ctx) const
  {

    SG::ReadHandle<xAOD::JetContainer> jets(m_jetsInKey,ctx);
    ATH_CHECK(jets.isValid());

    // build GN2XTauV00 hadles
    std::map<std::string, SG::ReadDecorHandle<xAOD::JetContainer, float>> GN2XTauV00_handles;
    for (const auto& [name, key] : m_GN2XTauV00_decorKeys) {
        GN2XTauV00_handles.emplace(name, SG::ReadDecorHandle<xAOD::JetContainer,float>(key));
    }
    SG::WriteDecorHandle<xAOD::JetContainer, float> GN2XTauV00_score(m_GN2XTauV00_htt_scoreDecorKey);

    // build GN3XPV01 hadles
    std::map<std::string, SG::ReadDecorHandle<xAOD::JetContainer, float>> GN3XPV01_handles;
    for (const auto& [name, key] : m_GN3XPV01_decorKeys) {
        GN3XPV01_handles.emplace(name, SG::ReadDecorHandle<xAOD::JetContainer,float>(key));
    }
    SG::WriteDecorHandle<xAOD::JetContainer, float> GN3XPV01_score(m_GN3XPV01_htt_scoreDecorKey);

    // compute scores
    for(const xAOD::Jet* lRjet: *jets) {

        float GN2XTauV00_htautau_score = compute_GN2XTauV00_htt_score(
          GN2XTauV00_handles.at("phtautauhad")(*lRjet), 
          GN2XTauV00_handles.at("phbb")(*lRjet), 
          GN2XTauV00_handles.at("phcc")(*lRjet),
          GN2XTauV00_handles.at("ptop")(*lRjet),
          GN2XTauV00_handles.at("pqcd")(*lRjet)
        );
        GN2XTauV00_score(*lRjet) = GN2XTauV00_htautau_score;

        float GN3XPV01_htautau_score = compute_GN3XPV01_htt_score(
            GN3XPV01_handles.at("phtautauhad")(*lRjet),
            GN3XPV01_handles.at("phbb")(*lRjet),
            GN3XPV01_handles.at("phcc")(*lRjet),
            GN3XPV01_handles.at("ptop")(*lRjet),
            GN3XPV01_handles.at("pWqq")(*lRjet),
            GN3XPV01_handles.at("pqcdbx")(*lRjet),
            GN3XPV01_handles.at("pqcdbb")(*lRjet),
            GN3XPV01_handles.at("pqcdcx")(*lRjet),
            GN3XPV01_handles.at("pqcdll")(*lRjet)
        );
        GN3XPV01_score(*lRjet) = GN3XPV01_htautau_score;
    }
      
    return StatusCode::SUCCESS;
  }
}

