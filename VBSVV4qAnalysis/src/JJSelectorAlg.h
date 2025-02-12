/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VBSVV4qANALYSIS_JJSELECTORALG_H
#define VBSVV4qANALYSIS_JJSELECTORALG_H

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <AsgDataHandles/ReadDecorHandleKey.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>


namespace VBSVV4q{

  enum Booleans{
    PASS_TRIGGER,
    PASS_ONE_LARGE_JET,
    //PASS_2LargeRJets_2SmallRJets,
  };

  /// \brief An algorithm for counting containers
  class JJSelectorAlg final : public EL::AnaAlgorithm{

    public:
      JJSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private:
      const std::vector<std::string> m_STANDARD_CUTS{
        "PASS_TRIGGER",
        "PASS_ONE_LARGE_JET",
        //"PASS_2LargeRJets_2SmallRJets",
      };
     
      void vbsjetsSelection(const xAOD::JetContainer * vbsjets);
      void JJSelection(const xAOD::JetContainer *largeJets, const CP::SystematicSet& sys);
      void eventCategorisation();

      StatusCode initialiseCutflow();

      Gaudi::Property<bool> m_bypass{ this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CutManager m_VBSVV4qCuts;

      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer> m_SmallRJetsHandle{ this, "SmallRJets", "", "Jet container to read"};
      CP::SysReadHandle<xAOD::JetContainer> m_LargeRJetsHandle{ this, "LargeRJets", "", "Jet container to read"};

      CP::SysReadDecorHandle<float> m_GN2Xv01_phbb = {this, "phbb", "GN2Xv01_phbb", "GN2Xv01_phbb"};
      CP::SysReadDecorHandle<float> m_GN2Xv01_phcc = {this, "phcc", "GN2Xv01_phcc", "GN2Xv01_phcc"};
      CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd = {this, "pqcd", "GN2Xv01_pqcd", "GN2Xv01_pqcd"};
      CP::SysReadDecorHandle<float> m_GN2Xv01_ptop = {this, "ptop", "GN2Xv01_ptop", "GN2Xv01_ptop"};

      CP::SysReadHandle<xAOD::JetContainer> m_vbsjetHandle{ this, "vbsjets", "", "VBS Jet container to read" };

      CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

      CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{ this, "event", "EventInfo",  "EventInfo container to read" };

      CP::SysReadDecorHandle<bool> m_passTriggerSJT {this, "passTriggerSJT", "pass_trigger_SJT_%SYS%", "events pass any singjet triggers"};

      CP::SysFilterReporterParams m_filterParams {this, "VBSVV4q selection"};

      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      std::vector<VBSVV4q::Booleans> m_inputCutKeys;
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      
      long long int m_total_events{0};

      std::unordered_map<VBSVV4q::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      std::unordered_map<VBSVV4q::Booleans, bool> m_bools;
      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
      std::unordered_map<VBSVV4q::Booleans, std::string> m_boolnames{
        {VBSVV4q::PASS_TRIGGER, "PASS_TRIGGER"},
        {VBSVV4q::PASS_ONE_LARGE_JET, "PASS_ONE_LARGE_JET"},
        //{VBSVV4q::PASS_2LargeRJets_2SmallRJets, "PASS_2LargeRJets_2SmallRJets"},
      };

      CP::SysReadDecorHandle<unsigned int> m_year {this, "year", "dataTakingYear", ""};

      Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };

  };
}
#endif