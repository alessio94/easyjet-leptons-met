/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VariableDumperAlg.h
//
// This is an algorithm that will dump variables into a tree.
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_VARIABLEDUMPERALG
#define HH4BANALYSIS_VARIABLEDUMPERALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <AthContainers/AuxElement.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>

// Class definition

namespace HH4B
{

  /// \brief An algorithm for dumping variables
  class VariableDumperAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    VariableDumperAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};
    CP::SysListHandle m_systematicsList{this};

    // Member variables for configuration
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{
        this, "ElectronsKey", "AnalysisElectrons_%SYS%",
        "the electron collection to run on"};

    CP::SysReadHandle<xAOD::PhotonContainer> m_photonHandle{
        this, "PhotonsKey", "AnalysisPhotons_%SYS%",
        "the photon collection to run on"};

    CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{
        this, "MuonsKey", "AnalysisMuons_%SYS%",
        "the muon collection to run on"};

    CP::SysReadHandle<xAOD::JetContainer> m_jetsmallRHandle{
        this, "SmallJetKey", "AnalysisJets_%SYS%",
        "the small-R jet collection to run on"};

    CP::SysReadHandle<xAOD::JetContainer> m_jetlargeRHandle{
        this, "LargeJetKey", "AnalysisLargeJets_%SYS%",
        "the large-R jet collection to run on"};

    CP::SysReadHandle<xAOD::JetContainer> m_VRtrackjetHandle{
        this, "VRtrackjets", "VRTrackJets_%SYS%",
        "the VR track jet collection to run on"};
  };
}

#endif
