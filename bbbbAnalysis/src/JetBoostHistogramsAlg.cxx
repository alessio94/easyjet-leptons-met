/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Evil Teng Jian Khoo

#include "JetBoostHistogramsAlg.h"
#include "JetBoostHistograms.h"

#include <StoreGate/ReadDecorHandle.h>

#include "H5Cpp.h"

namespace HH4B
{
  JetBoostHistogramsAlg::JetBoostHistogramsAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }
  JetBoostHistogramsAlg::~JetBoostHistogramsAlg() = default;

  StatusCode JetBoostHistogramsAlg::initialize()
  {
    ATH_MSG_DEBUG("initialize");
    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_jetsIn.initialize(m_systematicsList));
    ATH_CHECK(m_output_svc.retrieve());

    ANA_CHECK (m_systematicsList.initialize());

    for (const auto& sys: m_systematicsList.systematicsVector()) {
      ATH_MSG_DEBUG("booking " << sys.name());
      m_jet_histograms.emplace_back(sys,std::make_unique<JetBoostHistograms>());
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetBoostHistogramsAlg::execute(const EventContext& ctx) const
  {

    SG::ReadDecorHandle<xAOD::EventInfo, float> eventWeight(
      m_eventInfoKey, ctx);
    ATH_CHECK(eventWeight.isValid());
    float weight = eventWeight(*eventWeight);

    for (auto& [sys, hists]: m_jet_histograms) {
      ATH_MSG_VERBOSE("filling " << sys.name());
      const xAOD::JetContainer* jets = nullptr;
      ATH_CHECK(m_jetsIn.retrieve(jets, sys));
      for(const Item* jet : *jets) {
        hists->fill(*jet, weight);
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetBoostHistogramsAlg::finalize() {
    ATH_MSG_DEBUG("finalizing");
    for (const auto& [sys, hists]: m_jet_histograms) {
      std::string name = sys.name().empty() ? "nominal" : sys.name();
      ATH_MSG_DEBUG("saving " << name);
      H5::Group sysgroup = m_output_svc->group()->createGroup(name);
      hists->write(sysgroup);
    }
    return StatusCode::SUCCESS;
  }

}
