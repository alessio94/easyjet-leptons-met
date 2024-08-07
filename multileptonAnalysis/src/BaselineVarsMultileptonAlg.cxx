/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsMultileptonAlg.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"
#include "EasyjetHub/MT2_ROOT.h"

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
    ATH_CHECK(m_metHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

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
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_met_sig.initialize(m_systematicsList, m_metHandle));

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

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      for (const auto& var: m_floatVectorVariables) {
        m_FVbranches.at(var).set(*event, {}, sys);
      }

      for (const auto& var: m_charVectorVariables) {
        m_CVbranches.at(var).set(*event, {}, sys);
      }

      int n_electrons = electrons->size();
      int n_muons = muons->size();
      int n_jets = jets->size();
      int nCentralJets = 0;

      // b-jet sector
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        // count central jets
        if (std::abs(jet->eta())<2.5) {
          nCentralJets++;
          if (WPgiven && m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }
      int n_bjets = bjets->size();

      m_Ibranches.at("nJets").set(*event, n_jets, sys);
      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
      m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);


    }
    return StatusCode::SUCCESS;
  }

}


