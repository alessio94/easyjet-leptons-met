/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef MULTILEPTONANALYSIS_MULTILEPTONSELECTORALG
#define MULTILEPTONANALYSIS_MULTILEPTONSELECTORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysFilterReporterParams.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEgamma/ElectronContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODMuon/MuonContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>

namespace MULTILEPTON
{
  enum TriggerChannel
  {

  };

  enum Var
  {

  };

  enum Booleans
  {

  };

  /// \brief An algorithm for counting containers
  class MultileptonSelectorAlg final : public EL::AnaAlgorithm
  {

public:
    MultileptonSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// \brief This is the mirror of initialize() and is called after all
    /// events are processed.
    StatusCode
    finalize() override;


private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    Gaudi::Property<bool> m_isMC{this, "isMC", false, "Is this simulation?"};

    Gaudi::Property<bool> m_bypass{
        this, "bypass", false, "Run selector algorithm in pass-through mode"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList{this};

    CP::SysReadHandle<xAOD::JetContainer> m_jetHandle{
        this, "jets", "hhmlAnalysisJets_%SYS%", "Jet container to read"};

    CP::SysReadDecorHandle<char> m_isBtag{
        this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{
        this, "event", "EventInfo", "EventInfo container to read"};

    CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{
        this, "electrons", "hhmlAnalysisElectrons_%SYS%", "Electron container to read"};

    CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{
        this, "muons", "hhmlAnalysisMuons_%SYS%", "Muon container to read"};

    CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle{
        this, "met", "AnalysisMET_%SYS%", "MET container to read"};

    CP::SysReadDecorHandle<unsigned int> m_year{
        this, "year", "dataTakingYear",""};

    CP::SysFilterReporterParams m_filterParams{
        this, "Multilepton selection"};

    long long int m_total_events{0};
  };

} // namespace MULTILEPTON

#endif // MULTILEPTONANALYSIS_MULTILEPTONSELECTORALG
