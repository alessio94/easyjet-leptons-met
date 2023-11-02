/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef TTHHANALYSIS_FINALVARSTTHHALG
#define TTHHANALYSIS_FINALVARSTTHHALG

#include <AthContainers/ConstDataVector.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
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

    SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer> >
    m_smallRJets_BTag_ContainerInKey{ this, "smallRJets_BTag_ContainerInKey",
                            "",   "containerName to read" };


    SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer> >
    m_smallRJets_ContainerInKey{ this, "smallRJets_ContainerInKey",
                            "",   "Jet container without WP to read" };


    SG::ReadHandleKey<ConstDataVector<xAOD::MuonContainer> >
    m_muonContainerInKey{ this, "muonContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<ConstDataVector<xAOD::ElectronContainer> >
    m_electronContainerInKey{ this, "electronContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
      this, "EventInfoKey", "EventInfo", "EventInfo container to dump"
    };

    std::unordered_map<std::string, SG::AuxElement::Decorator<float> > m_decos;
    std::vector<std::string> m_vars{
      //lepton info
      "n_leptons",

      // b-jet kinematics
      "Jet_pt_B1", "Jet_eta_B1", "Jet_phi_B1", "Jet_E_B1",
      "Jet_pt_B2", "Jet_eta_B2", "Jet_phi_B2", "Jet_E_B2", 
      "Jet_pt_B3", "Jet_eta_B3", "Jet_phi_B3", "Jet_E_B3", 
      "Jet_pt_B4", "Jet_eta_B4", "Jet_phi_B4", "Jet_E_B4", 
      "Jet_pt_B5", "Jet_eta_B5", "Jet_phi_B5", "Jet_E_B5",
      "Jet_pt_B6", "Jet_eta_B6", "Jet_phi_B6", "Jet_E_B6",


      // additional variables
      "HT",

      //jets info
      "njets", "nBjets",

      //Higgs candidate invariant mass
      "H1_m", "H1_pT", "H1_eta", "H1_phi",
      "H2_m", "H2_pT", "H2_eta", "H2_phi",

      //HH pair variables

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
      "Jets_DeltaEtaMax", "Jets_DeltaEtaMin", "Jets_DeltaEtaMean"
    };

    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> getPairKinematics(const ConstDataVector<xAOD::JetContainer>& jetPairs);
    std::tuple<double, double, double> calculateVectorStats(const std::vector<double>& inputVector);
  };
}

#endif
