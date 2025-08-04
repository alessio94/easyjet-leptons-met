/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef MONOJETANALYSIS_MONOJETSELECTORALG
#define MONOJETANALYSIS_MONOJETSELECTORALG

#include <memory>
#include "AnaAlgorithm/AnaAlgorithm.h"
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>


#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODTau/TauJetContainer.h>

#include <EasyjetHub/CutManager.h>

class CutManager;

namespace MONOJET
{

  /// \brief An algorithm for counting containers
  class MonojetSelectorAlg final : public AthHistogramAlgorithm {

    public:
      MonojetSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private :

      const std::vector<std::string> m_STANDARD_CUTS{
        "PASS_TRIGGER",
        "JETCUT",
        "LARGEJETCUT",  
        "METCUT",
        "LEPCUT",
        "LEPTON_VETO",      
        "ELECTRON_VETO",
        "MUON_VETO",
        "TAU_VETO",
        "ALL_CUTS"
      };

      StatusCode initialiseCutflow();

      Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };
      
      Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

      Gaudi::Property<int> m_met_cut
      {this, "met_cut", 0. , "Minimum missing transverse momentum in pre-selection in GeV"};

      /// \brief Setup syst-aware input container handles
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "MonojetAnalysisJets_%SYS%", "Jet container to read" };

      CP::SysReadHandle<xAOD::JetContainer> 
      m_largejetHandle{ this, "largejets", "MonojetAnalysisLargeJets_%SYS%", "Large R Jet container to read"};

      CP::SysReadDecorHandle<char>
      m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

      Gaudi::Property<std::vector<std::string>> m_PCBTnames
        {this, "PCBTDecorList", {}, "Name list of pseudo-continuous b-tagging decorator"};

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "MonojetAnalysisElectrons_%SYS%",   "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "MonojetAnalysisMuons_%SYS%",   "Muon container to read" };

      CP::SysReadHandle<xAOD::TauJetContainer>
      m_tauHandle{ this, "taus", "MonojetAnalysisTaus_%SYS%",   "Tau container to read" };

      CP::SysReadHandle<xAOD::MissingETContainer>
      m_metHandle{ this, "met", "AnalysisMET_%SYS%",   "MET container to read" };

      CP::SysFilterReporterParams m_filterParams {this, "Monojet selection"};

      std::unordered_map<std::string,  SG::ReadDecorHandleKey<xAOD::EventInfo>> m_triggerDecorKeys;

      Gaudi::Property<std::vector<std::string>> m_triggers
      { this, "triggerLists", {}, "Name list of trigger" };

      std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;


      std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;

      CutManager m_MonojetCuts;
      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      long long int m_total_events{0};
      double m_total_mcEventWeight{0.0};
      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};

      void evaluateALLcuts
      (CutManager& MonojetCuts);

      void evaluateMETTriggerCuts
	      (const xAOD::EventInfo* event, 
        const std::vector<std::string> &Triggers, CutManager& MonojetCuts, const CP::SystematicSet& sys);

      void evaluateLeptonVeto
        (const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons,
         const xAOD::TauJetContainer& taus, CutManager& MonojetCuts);
         
      void evaluateElectronVeto
        (const xAOD::ElectronContainer& electrons, CutManager& MonojetCuts);
      void evaluateMuonVeto
        (const xAOD::MuonContainer& muons, CutManager& MonojetCuts);
      void evaluateTauVeto
        (const xAOD::TauJetContainer& taus, CutManager& MonojetCuts);

      void evaluateMETCuts
        (const xAOD::MissingET* met, CutManager& MonojetCuts);

      void evaluateJETCuts
        (const xAOD::JetContainer& jets, CutManager& MonojetCuts);

      void evaluateLARGEJETCuts
        (const xAOD::JetContainer& largeJets, CutManager& MonojetCuts);

      CP::SysReadDecorHandle<float>
        m_generatorWeight{ this, "generatorWeight", "generatorWeight_%SYS%", "MC event weights" };
  };
}

#endif // MONOJETANALYSIS_MONOJETSELECTORALG

