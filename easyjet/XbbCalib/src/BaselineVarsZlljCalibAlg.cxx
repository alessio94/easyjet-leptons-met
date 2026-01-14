/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author

#include "BaselineVarsZlljCalibAlg.h"
#include "FourVectorOutBlock.h"
#include "DefaultOutputs.h"
#include "SystVariableCopier.h"

#include <format>

namespace XBBCALIB
{
  BaselineVarsZlljCalibAlg::BaselineVarsZlljCalibAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  { }
  BaselineVarsZlljCalibAlg::~BaselineVarsZlljCalibAlg() = default;

  StatusCode BaselineVarsZlljCalibAlg::initialize()
  {
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK(m_XbbCalib_electronHandle.initialize(m_systematicsList));
    ATH_CHECK(m_XbbCalib_muonHandle.initialize(m_systematicsList));

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    
    ATH_CHECK(m_nLRJetsHandle.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_nElectronsHandle.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_nMuonsHandle.initialize(m_systematicsList, m_eventHandle));

     // Initialize muons and electron with SF
    if (m_isMC)
    {
      // Leptons
      ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
      ATH_CHECK (m_muonHandle.initialize(m_systematicsList));

      m_ele_SF = CP::SysReadDecorHandle<float>("effSF_" + m_eleWPName + "_%SYS%", this);
      ATH_CHECK(m_ele_SF.initialize(m_systematicsList, m_electronHandle));

      m_mu_SF = CP::SysReadDecorHandle<float>("effSF_" + m_muWPName + "_%SYS%", this);
      ATH_CHECK(m_mu_SF.initialize(m_systematicsList, m_muonHandle));

    }
  
    ATH_CHECK(m_Electron1SFHandle.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_Electron2SFHandle.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_Muon1SFHandle.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_Muon2SFHandle.initialize(m_systematicsList, m_eventHandle));

    m_z_candidate_4vec = std::make_unique<FourVectorOutBlock>(
        this, "Zcand_", m_systematicsList, m_eventHandle);

    // Maybe it is better to only save leding/sub-leadin lep?
    m_e0_candidate_4vec = std::make_unique<FourVectorOutBlock>(
        this, "Electron1_", m_systematicsList, m_eventHandle);
    m_e1_candidate_4vec = std::make_unique<FourVectorOutBlock>(
        this, "Electron2_", m_systematicsList, m_eventHandle);

    m_mu0_candidate_4vec = std::make_unique<FourVectorOutBlock>(
        this, "Muon1_", m_systematicsList, m_eventHandle);
    m_mu1_candidate_4vec = std::make_unique<FourVectorOutBlock>(
        this, "Muon2_", m_systematicsList, m_eventHandle);

    // Initialize other float branches
    // to do: implement new variables initializion with SystVariableCopier 
    for (const auto &var : m_floats_to_copy)
    {
      auto handle = std::make_unique<CP::SysWriteDecorHandle<float>>(
          m_copied_variable_prefix + var + "_%SYS%", this);
      ATH_CHECK(handle->initialize(m_systematicsList, m_eventHandle));
      m_Fbranches.emplace(m_copied_variable_prefix + var, std::move(handle));
    }
    // Initialize other int branches
    for (const auto &var : m_ints_to_copy)
    {
      auto handle = std::make_unique<CP::SysWriteDecorHandle<int>>(
          m_copied_variable_prefix + var + "_%SYS%", this);
      ATH_CHECK(handle->initialize(m_systematicsList, m_eventHandle));
      m_Ibranches.emplace(m_copied_variable_prefix + var, std::move(handle));
    }
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsZlljCalibAlg::execute()
  {
    //Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK(m_XbbCalib_muonHandle.retrieve(muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK(m_XbbCalib_electronHandle.retrieve(electrons, sys));

      for (const std::string &string_var : m_floats_to_copy)
      {
        m_Fbranches.at(m_copied_variable_prefix + string_var)
            ->set(*event, defaults::def_float, sys);
      }

      for (const std::string &string_var : m_ints_to_copy)
      {
        m_Ibranches.at(m_copied_variable_prefix + string_var)
            ->set(*event, defaults::def_int, sys);
      }

      // Calculate vars

      // Count electrons
      size_t n_electrons = electrons->size();
      m_nElectronsHandle.set(*event, n_electrons, sys);
      // Select electrons 
      const xAOD::Electron *ele0 = nullptr;
      const xAOD::Electron *ele1 = nullptr;
      m_e0_candidate_4vec->setDefault(*event, sys);
      m_e1_candidate_4vec->setDefault(*event, sys);
      for (const xAOD::Electron *electron : *electrons)
      {
        if (!ele0)
        {
          ele0 = electron;
          m_e0_candidate_4vec->set(*event, *electron, sys);
          if (m_isMC)
          {
            m_Electron1SFHandle.set(*event, m_ele_SF.get(*electron, sys), sys);
          }
        }
        else
        {
          ele1 = electron;
          m_e1_candidate_4vec->set(*event, *electron, sys);
          if (m_isMC)
          {
            m_Electron2SFHandle.set(*event, m_ele_SF.get(*electron, sys), sys);
          }
          break;
        }
      }

      // Count muons
      size_t n_muons = muons->size();
      m_nMuonsHandle.set(*event, n_muons, sys);
      // Select muons 
      const xAOD::Muon *mu0 = nullptr;
      const xAOD::Muon *mu1 = nullptr;
      m_mu0_candidate_4vec->setDefault(*event, sys);
      m_mu1_candidate_4vec->setDefault(*event, sys);
      for (const xAOD::Muon *muon : *muons)
      {
        if (!mu0)
        {
          mu0 = muon;
          m_mu0_candidate_4vec->set(*event, *muon, sys);
          if (m_isMC)
          {
            m_Muon1SFHandle.set(*event, m_mu_SF.get(*muon, sys), sys);
          }
        }
        else
        {
          mu1 = muon;
          m_mu1_candidate_4vec->set(*event, *muon, sys);
          if (m_isMC)
          {
            m_Muon2SFHandle.set(*event, m_mu_SF.get(*muon, sys), sys);
          }
          break;
        }
      }

      std::vector<const xAOD::Electron *> sel_electron = {ele0, ele1};
      std::vector<const xAOD::Muon *> sel_muon = {mu0, mu1};

      // dilepton kinematics
      TLorentzVector ll;
      TLorentzVector Leading_lep;
      TLorentzVector Subleading_lep;
      if (n_muons > 1 || n_electrons > 1)
      {
        if (n_muons > 1 && n_electrons < 2)
        {
          Leading_lep = sel_muon[0]->p4();
          Subleading_lep = sel_muon[1]->p4();
          ll = Leading_lep + Subleading_lep;
          m_Ibranches.at(m_copied_variable_prefix + "isMuonPair")
              ->set(*event, 1, sys);
        }
        else if (n_muons < 2 && n_electrons > 1)
        {
          Leading_lep = sel_electron[0]->p4();
          Subleading_lep = sel_electron[1]->p4();
          ll = Leading_lep + Subleading_lep;
          m_Ibranches.at(m_copied_variable_prefix + "isMuonPair")
              ->set(*event, 0, sys);
        }
        else if (n_muons > 1 && n_electrons > 1)
        {
          if (((sel_electron[0]->p4() + sel_electron[1]->p4()).Pt()) >
              ((sel_muon[0]->p4() + sel_muon[1]->p4()).Pt()))
          {
            Leading_lep = sel_electron[0]->p4();
            Subleading_lep = sel_electron[1]->p4();
            ll = Leading_lep + Subleading_lep;
            m_Ibranches.at(m_copied_variable_prefix + "isMuonPair")
                ->set(*event, 0, sys);
          }
          else
          {
            Leading_lep = sel_muon[0]->p4();
            Subleading_lep = sel_muon[1]->p4();
            ll = Leading_lep + Subleading_lep;
            m_Ibranches.at(m_copied_variable_prefix + "isMuonPair")
                ->set(*event, 1, sys);
          }
        }
      m_z_candidate_4vec->set(*event, ll, sys);
      m_Fbranches.at(m_copied_variable_prefix + "pt_balance")
            ->set(*event, (Leading_lep.Pt() - Subleading_lep.Pt()) / ll.Pt(), sys);
      m_Fbranches.at(m_copied_variable_prefix + "dR")
            ->set(*event, Leading_lep.DeltaR(Subleading_lep), sys);
      m_Fbranches.at(m_copied_variable_prefix + "dPhi")
            ->set(*event, Leading_lep.DeltaPhi(Subleading_lep), sys);
      m_Fbranches.at(m_copied_variable_prefix + "dEta")
            ->set(*event, Leading_lep.Eta() - Subleading_lep.Eta(), sys);      
      }
      else
      {
        m_z_candidate_4vec->setDefault(*event, sys);
      }
        
      // Count jets
      size_t n_lrjets = lrjets->size();
      m_nLRJetsHandle.set(*event, n_lrjets, sys); 
      
      // Define additional variables
      if (n_lrjets >= 1)
      {
      m_Fbranches.at(m_copied_variable_prefix + "leplep_LR_ptsymmetry")
            ->set(*event, (ll.Pt() - lrjets->at(0)->pt()) / (ll.Pt() + lrjets->at(0)->pt()), sys);
      m_Fbranches.at(m_copied_variable_prefix + "leplep_LR_dEta")
            ->set(*event, abs(ll.Eta() - lrjets->at(0)->eta()), sys);
      } 
    }
    return StatusCode::SUCCESS;
  }

}
