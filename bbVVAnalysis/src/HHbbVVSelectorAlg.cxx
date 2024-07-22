/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Kira Abeling

#include "HHbbVVSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>

namespace HHBBVV
{
  HHbbVVSelectorAlg ::HHbbVVSelectorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode HHbbVVSelectorAlg ::initialize()
  {

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    m_eleWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_" + m_eleWPName+"_%SYS%", this);
    m_muonWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_muonWPName+"_%SYS%", this);

    ATH_CHECK(m_eleWPDecorHandle.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_muonWPDecorHandle.initialize(m_systematicsList, m_muonHandle));

    ATH_CHECK(m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_selected_mu.initialize(m_systematicsList, m_muonHandle));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    // Intialise syst-aware output decorators
    ATH_CHECK(m_pass_sr.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    for ( auto name : m_channel_names){
      if( name == "1leptonBoosted") m_channels.push_back(HHBBVV::boosted1lep);
      else if ( name == "1leptonSplitboosted") m_channels.push_back(HHBBVV::splitboosted1lep);
      else if( name == "0leptonBoosted") m_channels.push_back(HHBBVV::boosted0lep);
      else if ( name == "0leptonSplitboosted") m_channels.push_back(HHBBVV::splitboosted0lep);
      else{
        ATH_MSG_ERROR("Unknown channel: "
          << name << std::endl
          << "Available are: [\"1leptonBoosted\", \"1leptonSplitboosted\", \"0leptonBoosted\", \"0leptonSplitboosted\"]");
        return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HHbbVVSelectorAlg ::execute()
  {

    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
      	ATH_MSG_ERROR("Could not retrieve MET");
      	return StatusCode::FAILURE;
      }

      // Apply selection

      // flags
      TWO_JETS = false;


      //************
      // lepton
      //************
      int n_leptons = 0;
      int n_looseleptons = 0;
      //example - needs to match bbVV definition
      for (const xAOD::Electron *electron : *electrons)
      {
        bool passElectronWP = m_eleWPDecorHandle.get(*electron, sys);
        m_selected_el.set(*electron, false, sys);
        if (passElectronWP && electron->pt() > 18. * Athena::Units::GeV)
        {
          m_selected_el.set(*electron, true, sys);
          n_leptons += 1;
        }
        else
          n_looseleptons += 1;
      }

      for (const xAOD::Muon *muon : *muons)
      {
        bool passMuonWP = m_muonWPDecorHandle.get(*muon, sys);
        m_selected_mu.set(*muon, false, sys);
        if (passMuonWP && std::abs(muon->eta()) < 2.5 && muon->pt() > 15. * Athena::Units::GeV)
        {
          m_selected_mu.set(*muon, true, sys);
          n_leptons += 1;
        }
        else
          n_looseleptons += 1;
      }


      //************
      // jet
      //************
      int n_jets = 0;
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          SG::VIEW_ELEMENTS);
      //resolved
      for (const xAOD::Jet *jet : *jets)
      {
        if (std::abs(jet->eta()) < 2.5)
        {
          n_jets += 1;
          if (WPgiven)
          {
            if (m_isBtag.get(*jet, sys))
              bjets->push_back(jet);
          }
        }
      }
      if (n_jets >= 2)
      {
        TWO_JETS = true;
        //further selections can go here
      }

      //****************
      // event level info
      //****************
      //for example masses of the system, met, fired triggers etc

      bool pass = true;
      //implement dedicated selection flags here
      // for(const auto& channel : m_channels){
      // 	if(channel == HHBBVV::splitboosted1lep) pass |= ;
      // 	else if(channel == HHBBVV::boosted1lep) pass |= ;
      // }
      if (!m_bypass && !pass) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    }
    
    return StatusCode::SUCCESS;
  }

  StatusCode HHbbVVSelectorAlg::finalize() {
    ANA_CHECK (m_filterParams.finalize ());
    return StatusCode::SUCCESS;
  }

}

