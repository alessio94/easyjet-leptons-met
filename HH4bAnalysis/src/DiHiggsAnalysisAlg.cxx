/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "DiHiggsAnalysisAlg.h"
#include "DiHiggsAnalysis.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace HH4B
{
  DiHiggsAnalysisAlg ::DiHiggsAnalysisAlg(const std::string &name,
                                          ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("doResolvedAnalysis", m_doResolvedAnalysis);
    declareProperty("doBoostedAnalysis", m_doBoostedAnalysis);
    declareProperty("btag_wps", m_btag_wps);
    declareProperty("vr_btag_wps", m_vr_btag_wps);
    declareProperty("isMC", m_isMC);
  }

  StatusCode DiHiggsAnalysisAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    // get to be used Higgs vars to create the final decorators here so they
    // are created once per alg and not per event
    m_higgsVars = getHiggsVarsNames(m_btag_wps, m_vr_btag_wps);
    for (std::string var : m_higgsVars)
    {
      SG::AuxElement::Decorator<float> HiggsVar_dec(var);
      m_diHiggs_decos.emplace(var, HiggsVar_dec);
    };

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_SmallJetKey.initialize());
    ATH_CHECK(m_LargeJetKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode DiHiggsAnalysisAlg ::execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    // get the xAOD objects
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    SG::ReadHandle<xAOD::JetContainer> antiKt4RecoJets(m_SmallJetKey);
    SG::ReadHandle<xAOD::JetContainer> antiKt10RecoJets(m_LargeJetKey);
    ATH_CHECK(eventInfo.isValid());
    ATH_CHECK(antiKt4RecoJets.isValid());
    ATH_CHECK(antiKt10RecoJets.isValid());

    // create the analysis object
    DiHiggsAnalysis hh4b_analysis;
    // write defaults (-1) into the Higgs variable map
    hh4b_analysis.initHiggsVarsMap(m_higgsVars);

    // which analyses to do
    if (m_doResolvedAnalysis)
    {
      for (std::string wp : m_btag_wps)
      {
        hh4b_analysis.makeResolvedAnalysis(*antiKt4RecoJets, wp, m_isMC);
      }
    }
    if (m_doBoostedAnalysis)
    {
      for (std::string wp : m_vr_btag_wps)
      {
        hh4b_analysis.makeBoostedAnalysis(*antiKt10RecoJets, wp);
      }
    }

    // get higgs vars map (behaves like dict in python)
    std::unordered_map<std::string, float> higgsVarsMap =
        hh4b_analysis.getHiggsVarsMap();
    // loop over decorators
    for (const auto &[var, value] : higgsVarsMap)
    {
      SG::AuxElement::Decorator<float> HiggsVar_dec = m_diHiggs_decos.at(var);
      HiggsVar_dec(*eventInfo) = value;
    };

    return StatusCode::SUCCESS;
  }
}
