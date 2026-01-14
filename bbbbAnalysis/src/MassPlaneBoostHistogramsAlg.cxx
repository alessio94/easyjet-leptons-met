/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Evil Teng Jian Khoo

#include "MassPlaneBoostHistogramsAlg.h"
#include "MassPlaneBoostHistograms.h"

#include <StoreGate/ReadDecorHandle.h>

#include "H5Cpp.h"

namespace HH4B
{
  MassPlaneBoostHistogramsAlg::MassPlaneBoostHistogramsAlg(
    const std::string &name,
    ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {
    declareProperty("truthMatchingPrefix", m_config.truth_matching_prefix);
    declareProperty("matchingMethods", m_mmv);
  }
  MassPlaneBoostHistogramsAlg::~MassPlaneBoostHistogramsAlg() = default;

  StatusCode MassPlaneBoostHistogramsAlg::initialize()
  {
    // gaudi hack
    m_config.matching_methods.insert(m_mmv.begin(), m_mmv.end());

    ATH_MSG_DEBUG("initialize");
    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_jetsIn.initialize(m_systematicsList));
    ATH_CHECK(m_output_svc.retrieve());

    ANA_CHECK (m_systematicsList.initialize());

    for (const auto& sys: m_systematicsList.systematicsVector()) {
      ATH_MSG_DEBUG("booking " << sys.name());
      m_jet_histograms.emplace_back(
        sys,std::make_unique<bhist::MassPlaneHists>(m_config));
    }

    return StatusCode::SUCCESS;
  }

  StatusCode MassPlaneBoostHistogramsAlg::execute(const EventContext& ctx) const
  {

    SG::ReadDecorHandle<xAOD::EventInfo, float> eventWeight(
      m_eventInfoKey, ctx);
    ATH_CHECK(eventWeight.isValid());
    float weight = eventWeight(*eventWeight);

    for (auto& [sys, hists]: m_jet_histograms) {
      ATH_MSG_VERBOSE("filling " << sys.name());
      const xAOD::JetContainer* jets = nullptr;
      ATH_CHECK(m_jetsIn.retrieve(jets, sys));
      std::vector jvec(jets->begin(), jets->end());
      hists->fill(jvec, weight);
    }

    return StatusCode::SUCCESS;
  }

  H5::Group requireGroup(H5::Group& base, const std::string& name) {
    if (!H5Lexists(base.getLocId(), name.c_str(), H5P_DEFAULT)) {
      base.createGroup(name);
    }
    return base.openGroup(name);
  }

  StatusCode MassPlaneBoostHistogramsAlg::finalize() {
    ATH_MSG_DEBUG("finalizing");
    for (const auto& [sys, hists]: m_jet_histograms) {
      std::string systematic = sys.name().empty() ? "nominal" : sys.name();
      ATH_MSG_DEBUG("saving " << systematic);
      H5::Group* parent = m_output_svc->group();
      H5::Group sysgroup = requireGroup(*parent, systematic);
      H5::Group massgroup = requireGroup(sysgroup, "mass");
      const std::string& pfx = m_config.truth_matching_prefix;
      std::string matchname = pfx.empty() ? "all" : pfx;
      H5::Group matchgroup = requireGroup(massgroup, matchname);
      hists->write(matchgroup);
    }
    return StatusCode::SUCCESS;
  }

}
