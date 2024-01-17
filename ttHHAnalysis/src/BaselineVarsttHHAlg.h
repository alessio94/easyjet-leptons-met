/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_recoSF{"", this};
    CP::SysReadDecorHandle<float> m_ele_idSF{"", this};
    CP::SysReadDecorHandle<float> m_ele_isoSF{"", this};

    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_mu_recoSF{"", this};
    CP::SysReadDecorHandle<float> m_mu_isoSF{"", this};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };
    Gaudi::Property<int> m_nLeptons
      { this, "nLeptons", 0 };

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float> > m_Fbranches;
    std::vector<std::string> m_Fvarnames{
      // b-jet kinematics
      "Jet_b1_pt", "Jet_b1_eta", "Jet_b1_phi", "Jet_b1_E",
      "Jet_b2_pt", "Jet_b2_eta", "Jet_b2_phi", "Jet_b2_E", 
      "Jet_b3_pt", "Jet_b3_eta", "Jet_b3_phi", "Jet_b3_E", 
      "Jet_b4_pt", "Jet_b4_eta", "Jet_b4_phi", "Jet_b4_E", 
      "Jet_b5_pt", "Jet_b5_eta", "Jet_b5_phi", "Jet_b5_E",
      "Jet_b6_pt", "Jet_b6_eta", "Jet_b6_phi", "Jet_b6_E",

      //truth information b-jets
      "Jet_b1_truthLabel", "Jet_b2_truthLabel", "Jet_b3_truthLabel",
      "Jet_b4_truthLabel", "Jet_b5_truthLabel", "Jet_b6_truthLabel",

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
      "Electron1_pt", "Electron1_eta", "Electron1_phi", "Electron1_E",
      "Electron2_pt", "Electron2_eta", "Electron2_phi", "Electron2_E",
      "Muon1_pt", "Muon1_eta", "Muon1_phi", "Muon1_E",
      "Muon2_pt", "Muon2_eta", "Muon2_phi", "Muon2_E",
      "ee_m", "ee_pt", "ee_dR", "ee_eta", "ee_phi", 
      "mumu_m", "mumu_pt", "mumu_dR", "mumu_eta", "mumu_phi",
      "emu_m", "emu_pt", "emu_dR", "emu_eta", "emu_phi"
    };
    std::vector<std::string> m_Fvarnames_MC{
      "Electron1_effSF", "Electron2_effSF",
      "Muon1_effSF", "Muon2_effSF",
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
