/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include "AthContainers/AuxElement.h"
#include "BaselineVarsbbttAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

namespace HH4B
{
  BaselineVarsbbttAlg::BaselineVarsbbttAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("bTagWP", m_bTagWP);
  }

  StatusCode BaselineVarsbbttAlg::initialize()
  {
    ATH_CHECK(m_smallRContainerInKey.initialize());
    ATH_CHECK(m_muonContainerInKey.initialize());
    ATH_CHECK(m_electronContainerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());

    // make decorators
    for (const std::string &var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
      SG::AuxElement::Decorator<float> deco(deco_var);
      m_decos.emplace(deco_var, deco);
    };
    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsbbttAlg::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    // set defaults

    for (const std::string &var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
      m_decos.at(deco_var)(*eventInfo) = -99.;
    };

    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRjets(
        m_smallRContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::MuonContainer> > muons_(
        m_muonContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::ElectronContainer> > electrons_(
        m_electronContainerInKey);

    ATH_CHECK(smallRjets.isValid());
    ATH_CHECK(muons_.isValid());
    ATH_CHECK(electrons_.isValid());

    ConstDataVector<xAOD::JetContainer> jets = *smallRjets;
    ConstDataVector<xAOD::MuonContainer> muons = *muons_;
    ConstDataVector<xAOD::ElectronContainer> electrons = *electrons_;

    if (muons.size() >= 1)
    {
      // Leading muon
      m_decos.at("Leading_Muon_pt_" + m_bTagWP)(*eventInfo) =
          muons[0]->pt();
      m_decos.at("Leading_Muon_eta_" + m_bTagWP)(*eventInfo) =
          muons[0]->eta();
    }

    if (electrons.size() >= 1)
    {
      // Leading muon
      m_decos.at("Leading_Muon_pt_" + m_bTagWP)(*eventInfo) =
          electrons[0]->pt();
      m_decos.at("Leading_Muon_eta_" + m_bTagWP)(*eventInfo) =
          electrons[0]->eta();
    }

    return StatusCode::SUCCESS;
  }
}
