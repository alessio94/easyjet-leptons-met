/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "AthContainers/AuxElement.h"
#include "BaselineVarsyybbAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>

namespace HH4B
{
  BaselineVarsyybbAlg::BaselineVarsyybbAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("bTagWP", m_bTagWP);
  }

  StatusCode BaselineVarsyybbAlg::initialize()
  {
    ATH_CHECK(m_smallRContainerInKey.initialize());
    ATH_CHECK(m_photonContainerInKey.initialize());
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

  StatusCode BaselineVarsyybbAlg::execute()
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
    SG::ReadHandle<ConstDataVector<xAOD::PhotonContainer> > photons_(
        m_photonContainerInKey);

    ATH_CHECK(smallRjets.isValid());
    ATH_CHECK(photons_.isValid());

    ConstDataVector<xAOD::JetContainer> jets = *smallRjets;
    ConstDataVector<xAOD::PhotonContainer> photons = *photons_;

    // check if we have at least 2 small R jets, and at least 2 photons
    if (jets.size() >= 2 && photons.size() >= 2)
    {

      // jets
      // auto h1 = jets[0]->jetP4() + jets[1]->jetP4();
      // auto h1.M();
      m_decos.at("DeltaR12_" + m_bTagWP)(*eventInfo) =
          xAOD::P4Helpers::deltaR(jets[0], jets[1]);

      // photons
      auto h2 = photons[0]->p4() + photons[1]->p4();

      // Leading photon
      m_decos.at("Leading_Photon_pt_" + m_bTagWP)(*eventInfo) =
          photons[0]->pt();
      m_decos.at("Leading_Photon_eta_" + m_bTagWP)(*eventInfo) =
          photons[0]->eta();
      m_decos.at("Leading_Photon_phi_" + m_bTagWP)(*eventInfo) =
          photons[0]->phi();
      m_decos.at("Leading_Photon_E_" + m_bTagWP)(*eventInfo) = photons[0]->e();

      // Subleading photon
      m_decos.at("Subleading_Photon_pt_" + m_bTagWP)(*eventInfo) =
          photons[1]->pt();
      m_decos.at("Subleading_Photon_eta_" + m_bTagWP)(*eventInfo) =
          photons[1]->eta();
      m_decos.at("Subleading_Photon_phi_" + m_bTagWP)(*eventInfo) =
          photons[1]->phi();
      m_decos.at("Subleading_Photon_E_" + m_bTagWP)(*eventInfo) =
          photons[1]->e();

      // Diphoton system
      m_decos.at("myy_" + m_bTagWP)(*eventInfo) = h2.M(); // mass
    }

    return StatusCode::SUCCESS;
  }
}
