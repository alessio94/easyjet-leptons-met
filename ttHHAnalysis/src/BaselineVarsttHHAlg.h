/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef TTHHANALYSIS_FINALVARSTTHHALG
#define TTHHANALYSIS_FINALVARSTTHHALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AthContainers/ConstDataVector.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>

namespace ttHH
{

  /// \brief An algorithm for counting containers
  class BaselineVarsttHHAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsttHHAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any
    
private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
    m_bjetHandle{ this, "bjets", "",   "BJet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "jets", "",   "Jet container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "",   "Muon container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "",   "Electron container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_HZPairsHandle{ this, "HZPairs", "",   "Container containing jets from HZ pairing to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_ZZPairsHandle{ this, "ZZPairs", "",   "Container containing jets from ZZ pairing to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    bool m_isMC;
    bool m_nLeptons;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float> > m_Fbranches;
    std::vector<std::string> m_Fvarnames{
      // b-jet kinematics
      "Jet_pt_b1", "Jet_eta_b1", "Jet_phi_b1", "Jet_E_b1",
      "Jet_pt_b2", "Jet_eta_b2", "Jet_phi_b2", "Jet_E_b2", 
      "Jet_pt_b3", "Jet_eta_b3", "Jet_phi_b3", "Jet_E_b3", 
      "Jet_pt_b4", "Jet_eta_b4", "Jet_phi_b4", "Jet_E_b4", 
      "Jet_pt_b5", "Jet_eta_b5", "Jet_phi_b5", "Jet_E_b5",
      "Jet_pt_b6", "Jet_eta_b6", "Jet_phi_b6", "Jet_E_b6",

      //truth information b-jets
      "Jet_truthLabel_b1", "Jet_truthLabel_b2", "Jet_truthLabel_b3",
      "Jet_truthLabel_b4", "Jet_truthLabel_b5", "Jet_truthLabel_b6",

      // additional variables
      "HT",

      //Higgs candidate invariant mass
      "H1_m", "H1_pt", "H1_eta", "H1_phi",
      "H2_m", "H2_pt", "H2_eta", "H2_phi",

      //Invariant mass of jet pair reconstructed using HZ target masses
      "HZ_H_m", "HZ_H_pt", "HZ_H_eta", "HZ_H_phi",
      "HZ_Z_m", "HZ_Z_pt", "HZ_Z_eta", "HZ_Z_phi",

      //Invariant mass of jet pair reconstructed using ZZ target masses
      "ZZ_Z1_m", "ZZ_Z1_pt", "ZZ_Z1_eta", "ZZ_Z1_phi",
      "ZZ_Z2_m", "ZZ_Z2_pt", "ZZ_Z2_eta", "ZZ_Z2_phi",

      //Pair mass and CHI square
      "HH_m", "HH_CHI",
      "HZ_m", "HZ_CHI",
      "ZZ_m", "ZZ_CHI",

      //DeltaR difference
      "Jets_DeltaR12", "Jets_DeltaR34", "Jets_DeltaR56", 
      "Jets_DeltaR1234", "Jets_DeltaR3456", "Jets_DeltaR5612",

      //DeltaPhi difference
      "Jets_DeltaPhi12", "Jets_DeltaPhi34", "Jets_DeltaPhi56",
      "Jets_DeltaPhi1234", "Jets_DeltaPhi3456", "Jets_DeltaPhi5612",

      //DeltaEta difference
      "Jets_DeltaEta12", "Jets_DeltaEta34", "Jets_DeltaEta56",
      "Jets_DeltaEta1234", "Jets_DeltaEta3456", "Jets_DeltaEta5612",

      //mean, max and min DeltaR and DeltaPhi
      "Jets_DeltaRMax", "Jets_DeltaRMin", "Jets_DeltaRMean",
      "Jets_DeltaEtaMax", "Jets_DeltaEtaMin", "Jets_DeltaEtaMean",

      //leptons
      "Leading_Electron_pt", "Leading_Electron_eta", 
      "Leading_Electron_phi", "Leading_Electron_E",
      "Subleading_Electron_pt", "Subleading_Electron_eta", 
      "Subleading_Electron_phi", "Subleading_Electron_E",
      "Leading_Muon_pt", "Leading_Muon_eta", 
      "Leading_Muon_phi", "Leading_Muon_E",
      "Subleading_Muon_pt", "Subleading_Muon_eta", 
      "Subleading_Muon_phi", "Subleading_Muon_E",
      "ee_m", "ee_pt", "ee_dR", "ee_eta", "ee_phi", 
      "mumu_m", "mumu_pt", "mumu_dR", "mumu_eta", "mumu_phi",
      "emu_m", "emu_pt", "emu_dR", "emu_eta", "emu_phi"
    };

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int> > m_Ibranches;
    std::vector<std::string> m_Ivarnames{
      //lepton info
      "n_leptons",
      //jets info
      "nJets", "nBJets",
    };

    const float m_targetMassH = 125e3; // Higgs target mass to be used in chi square calculation
    const float m_targetMassZ = 91.2e3; // Z boson target mass to be used in chi square calculation
    const float m_massResolution = 20.0e3; // Mass resolution used in chi square calculation

    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> getPairKinematics(const xAOD::JetContainer& jetPairs);
    std::tuple<double, double, double> calculateVectorStats(const std::vector<double>& inputVector);
    float computeChiSquare(float observedMass1, float observedMass2, float targetMass1, float targetMass2, float massResolution);
  };
}

#endif
