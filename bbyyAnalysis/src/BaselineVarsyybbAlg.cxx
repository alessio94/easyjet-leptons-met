/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsyybbAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>

namespace HHBBYY
{
  BaselineVarsyybbAlg::BaselineVarsyybbAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("isMC", m_isMC);
  }

  StatusCode BaselineVarsyybbAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       BaselineVarsyybbAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    for (const std::string &string_var: m_Fvarnames) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_Ivarnames) {
      CP::SysWriteDecorHandle<int> var {string_var+"_%SYS%", this};
      m_Ibranches.emplace(string_var, var);
      ATH_CHECK (m_Ibranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsyybbAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // container we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));

      static const SG::AuxElement::ConstAccessor<int>  HadronConeExclTruthLabelID("HadronConeExclTruthLabelID");

      // initialize
      TLorentzVector H_bb(0.,0.,0.,0.);
      TLorentzVector H_yy(0.,0.,0.,0.);
      TLorentzVector HH(0.,0.,0.,0.);
      TLorentzVector y1(0.,0.,0.,0.);
      TLorentzVector y2(0.,0.,0.,0.);
      TLorentzVector b1(0.,0.,0.,0.);
      TLorentzVector b2(0.,0.,0.,0.);

      int truthLabel_b1 = -99, truthLabel_b2 = -99;
      double dRHH = -99., dRyy = -99., dRbb = -99.;

      for (const std::string &string_var: m_Fvarnames) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }
      
      for (const std::string &string_var: m_Ivarnames) {
        m_Ibranches.at(string_var).set(*event, -99, sys);
      }

      int nCentralJets = 0;
      double HT = 0.; // scalar sum of jet pT

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        // Compute scalar pt sum (Ht) for all the jets in the event |eta|<4.4
        HT += jet->pt();

        // count central jets
        if (std::abs(jet->eta())<2.5) {
          nCentralJets++;
        }

        // check if jet is btagged
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }


      // photon sector
      if (photons->size() >= 1) {
        y1 = photons->at(0)->p4();

        m_Fbranches.at("Leading_Photon_pt").set(*event, y1.Pt(), sys);
        m_Fbranches.at("Leading_Photon_eta").set(*event, y1.Eta(), sys);
        m_Fbranches.at("Leading_Photon_phi").set(*event, y1.Phi(), sys);
        m_Fbranches.at("Leading_Photon_E").set(*event, y1.E(), sys);
      }
      if (photons->size() >= 2) {
        y2 = photons->at(1)->p4();

        // Build the H(yy) candidate
        H_yy = y1 + y2;
        dRyy = (y1).DeltaR(y2);

        m_Fbranches.at("Subleading_Photon_pt").set(*event, y2.Pt(), sys);
        m_Fbranches.at("Subleading_Photon_eta").set(*event, y2.Eta(), sys);
        m_Fbranches.at("Subleading_Photon_phi").set(*event, y2.Phi(), sys);
        m_Fbranches.at("Subleading_Photon_E").set(*event, y2.E(), sys);

        m_Fbranches.at("myy").set(*event, H_yy.M(), sys);
        m_Fbranches.at("pTyy").set(*event, H_yy.Pt(), sys);
        m_Fbranches.at("Etayy").set(*event, H_yy.Eta(), sys);
        m_Fbranches.at("Phiyy").set(*event, H_yy.Phi(), sys);
        m_Fbranches.at("dRyy").set(*event, dRyy, sys);
      }

      // b-jet sector
      if (bjets->size() >= 1) {
        b1 = bjets->at(0)->p4();
        if (m_isMC) truthLabel_b1 = HadronConeExclTruthLabelID(*bjets->at(0));

        m_Fbranches.at("Jet_pt_b1").set(*event, b1.Pt(), sys);
        m_Fbranches.at("Jet_eta_b1").set(*event, b1.Eta(), sys);
        m_Fbranches.at("Jet_phi_b1").set(*event, b1.Phi(), sys);
        m_Fbranches.at("Jet_E_b1").set(*event, b1.E(), sys);
        if (m_isMC) m_Ibranches.at("Jet_truthLabel_b1").set(*event, truthLabel_b1, sys);
      }
      if (bjets->size() >= 2) {
        b2 = bjets->at(1)->p4();
        if (m_isMC) truthLabel_b2 = HadronConeExclTruthLabelID(*bjets->at(1));

        // Build the H(bb) candidate
        H_bb = b1 + b2;
        dRbb = (b1).DeltaR(b2);

        m_Fbranches.at("Jet_pt_b2").set(*event, b2.Pt(), sys);
        m_Fbranches.at("Jet_eta_b2").set(*event, b2.Eta(), sys);
        m_Fbranches.at("Jet_phi_b2").set(*event, b2.Phi(), sys);
        m_Fbranches.at("Jet_E_b2").set(*event, b2.E(), sys);
        if (m_isMC) m_Ibranches.at("Jet_truthLabel_b2").set(*event, truthLabel_b2, sys);

        m_Fbranches.at("mbb").set(*event, H_bb.M(), sys);
        m_Fbranches.at("pTbb").set(*event, H_bb.Pt(), sys);
        m_Fbranches.at("Etabb").set(*event, H_bb.Eta(), sys);
        m_Fbranches.at("Phibb").set(*event, H_bb.Phi(), sys);
        m_Fbranches.at("dRbb").set(*event, dRbb, sys);
      }

      // Build the HH candidate
      if (photons->size() >= 2 && bjets->size() >= 2) {
        HH = H_yy + H_bb;
        dRHH = H_yy.DeltaR(H_bb);

        m_Fbranches.at("pTbbyy").set(*event, HH.Pt(), sys);
        m_Fbranches.at("Etabbyy").set(*event, HH.Eta(), sys);
        m_Fbranches.at("Phibbyy").set(*event, HH.Phi(), sys);
        m_Fbranches.at("dRbbyy").set(*event, dRHH, sys);

        m_Fbranches.at("mbbyy").set(*event, HH.M(), sys);
        m_Fbranches.at("mbbyy_star").set(*event, HH.M()-(H_bb.M()-125e3)-(H_yy.M()-125e3), sys);
      }

      m_Fbranches.at("HT").set(*event, HT, sys);

      m_Ibranches.at("nPhotons").set(*event, photons->size(), sys);
      m_Ibranches.at("nJets").set(*event, jets->size(), sys);
      m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
      m_Ibranches.at("nBJets").set(*event, bjets->size(), sys);

    }

    return StatusCode::SUCCESS;
  }
}
