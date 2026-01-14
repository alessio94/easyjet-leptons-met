/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsZtautauCalibAlg.h"

namespace XBBCALIB
{
  BaselineVarsZtautauCalibAlg::BaselineVarsZtautauCalibAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  { }

  StatusCode BaselineVarsZtautauCalibAlg::initialize()
  {
    ATH_CHECK (m_lRjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_eventInfoKey.initialize());
    m_yearKey = "EventInfo.dataTakingYear";
    ATH_CHECK(m_yearKey.initialize());
    ATH_CHECK (m_matchingTool.retrieve());

    // Intialise syst-aware output decorators
    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &var : m_intVariables){
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsZtautauCalibAlg::execute(const EventContext& ctx) const
  {
    //Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Initialize variables
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *largeRjets = nullptr;
      ANA_CHECK (m_lRjetHandle.retrieve (largeRjets, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve(photons, sys));

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      // look up for triggers
      SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
      SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> year_handle (m_yearKey);
      ATH_CHECK (eventInfo.isValid());
      int year = year_handle(*eventInfo);
      // TODO: __TauTauCalib__ LargeR jet trigger match information is not stored yet
      // see:  https://its.cern.ch/jira/browse/ATR-26050
      // std::vector<std::string> large_r_jet_triggers;
      // getTriggers(year, "HLT_j", large_r_jet_triggers);
      std::vector<std::string> photon_triggers;
      getTriggers(year, "HLT_g", photon_triggers);

      // Calculate vars
      for (const xAOD::Jet *lRjet : *largeRjets) {

        // check large-r jet vs photon triggers 
        for(const auto& trig : photon_triggers){
          bool match = m_matchingTool->match(*lRjet, trig);
          if (match) m_Ibranches.at("largeR_trigMatch").set(*lRjet, match, sys);
        }

      }

      // Count jets
      m_Ibranches.at("nJets").set(*event, largeRjets->size(), sys);
      m_Ibranches.at("nTaus").set(*event, taus->size(), sys);

      // check photon triggers
      for  (const xAOD::Photon *photon : *photons) {
        for(const auto& trig : photon_triggers){
          bool match = m_matchingTool->match(*photon, trig);
          if (match) m_Ibranches.at("photon_trigMatch").set(*photon, match, sys);
        }
      }

    }
    return StatusCode::SUCCESS;
  }

  void BaselineVarsZtautauCalibAlg::getTriggers(
    int year,
    const std::basic_string<char>& key_word,
    std::vector<std::string>& triggers
  ) const {

    for (auto&& [m_year, trigger_list] : m_triggerlist)
      if (year == std::stoi(m_year))
        for (auto&& trig : trigger_list)
          if (trig.starts_with(key_word))
            triggers.push_back(trig);
  }
}
