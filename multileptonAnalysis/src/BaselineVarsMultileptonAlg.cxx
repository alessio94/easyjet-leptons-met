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

      // selected leptons
      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;
      const xAOD::Electron* ele2 = nullptr;
      const xAOD::Electron* ele3 = nullptr;

      for(const xAOD::Electron* electron : *electrons) {
        if(!ele0) ele0 = electron;
        else if(!ele1) ele1 = electron;
        else if(!ele2) ele2 = electron;
        else {
          ele3 = electron;
          break;
        }
      }

      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;
      const xAOD::Muon* mu2 = nullptr;
      const xAOD::Muon* mu3 = nullptr;

      for(const xAOD::Muon* muon : *muons) {
        if(!mu0) mu0 = muon;
        else if(!mu1) mu1 = muon;
        else if(!mu2) mu2 = muon;
        else {
          mu3 = muon;
          break;
        }
      }

      std::vector<std::pair<const xAOD::IParticle*, int>> leptons;
      if(ele0) leptons.emplace_back(ele0, -11*ele0->charge());
      if(mu0) leptons.emplace_back(mu0, -13*mu0->charge());
      if(ele1) leptons.emplace_back(ele1, -11*ele1->charge());
      if(mu1) leptons.emplace_back(mu1, -13*mu1->charge());
      if(ele2) leptons.emplace_back(ele2, -11*ele2->charge());
      if(mu2) leptons.emplace_back(mu2, -13*mu2->charge());
      if(ele3) leptons.emplace_back(ele3, -11*ele3->charge());
      if(mu3) leptons.emplace_back(mu3, -13*mu3->charge());

      int nLeptons = muons->size() + electrons->size();
      m_Ibranches.at("nLeptons").set(*event, nLeptons, sys);
      
      // Sort by pT; FIXME: for 3l, os lepton should be the 1st then the ss leptons sorted by pT
      std::sort(leptons.begin(), leptons.end(),
          [](const std::pair<const xAOD::IParticle*, int>& a,
              const std::pair<const xAOD::IParticle*, int>& b) {
            return a.first->pt() > b.first->pt(); });

      // TODO: compute trigger SF
    }
    return StatusCode::SUCCESS;
  }

}