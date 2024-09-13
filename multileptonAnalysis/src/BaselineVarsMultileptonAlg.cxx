/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsMultileptonAlg.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"
#include "EasyjetHub/MT2_ROOT.h"
#include "SubChannelClassify.h"

namespace MULTILEPTON
{
  BaselineVarsMultileptonAlg::BaselineVarsMultileptonAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsMultileptonAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("   BaselineVarsMultileptonAlg    \n");
    ATH_MSG_INFO("*********************************\n");

    // Read syst-aware input handles
    ATH_CHECK(m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK(m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK(m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK(m_metHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));

    // Initialise syst-aware output decorators
    for(const std::string &var : m_floatVariables){
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK(m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for(const std::string &var : m_floatVectorVariables){
      CP::SysWriteDecorHandle<std::vector<float>> whandle{var+"_%SYS%", this};
      m_FVbranches.emplace(var, whandle);
      ATH_CHECK(m_FVbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for(const std::string &var : m_intVariables){
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    for(const std::string & var : m_charVectorVariables){
      CP::SysWriteDecorHandle<std::vector<char>> whandle(var+"_%SYS%", this);
      m_CVbranches.emplace(var, whandle);
      ATH_CHECK(m_CVbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    if (!m_isBtag.empty()){
      ATH_CHECK(m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK(m_met_sig.initialize(m_systematicsList, m_metHandle));

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsMultileptonAlg::execute()
  {
    // Loop over all systematics
    for (const auto& sys : m_systematicsList.systematicsVector()){
      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -999., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -999, sys);
      }

      for (const auto& var: m_floatVectorVariables) {
        m_FVbranches.at(var).set(*event, {}, sys);
      }

      for (const auto& var: m_charVectorVariables) {
        m_CVbranches.at(var).set(*event, {}, sys);
      }

      int n_electrons = electrons->size();
      int n_muons = muons->size();
      int n_taus = taus->size();
      int n_totalJets = jets->size();
      int n_centralJets = 0;

      // seperate jets into b-jets and light jets
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto lightJets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        // count central jets
        if (std::abs(jet->eta())<2.5) {
          n_centralJets++;
          if (WPgiven && m_isBtag.get(*jet, sys)){
            bjets->push_back(jet);
          }
          else {
            lightJets->push_back(jet);
          }
        }
        else {
          lightJets->push_back(jet);
        }
      }
      int n_bjets = bjets->size();
      int n_lightJets = lightJets->size();

      m_Ibranches.at("nTotalJets").set(*event, n_totalJets, sys);
      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nTaus").set(*event, n_taus, sys);
      m_Ibranches.at("nLightJets").set(*event, n_lightJets, sys);
      m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
      m_Ibranches.at("nCentralJets").set(*event, n_centralJets, sys);

      auto classifier = SubChannelClassify(
          muons, electrons, taus, bjets.get()
          );

      m_Ibranches.at("nLeptons").set(
          *event, classifier.getNLeptons(), sys);
      m_Ibranches.at("totalLepCharge").set(
          *event, classifier.getTotalChargeLep(), sys);
      m_Ibranches.at("totalTauCharge").set(
          *event, classifier.getTotalChargeTau(), sys);
      m_Ibranches.at("subChannelID").set(
          *event, static_cast<int>(classifier.getSubChannelId()), sys);
      m_Ibranches.at("subChannelFlavor").set(
          *event, classifier.getSubChannelFlavor(), sys);

      // TODO: compute trigger SF
    }
    return StatusCode::SUCCESS;
  }

}