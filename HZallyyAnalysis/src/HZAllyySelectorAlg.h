/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef HZALLYYANALYSIS_HZALLYYSELECTORALG
#define HZALLYYANALYSIS_HZALLYYSELECTORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>

namespace HZALLYY
{
  enum TriggerChannel
    {
      SLT,
      DLT,
      ASLT1_em,
      ASLT1_me,
      ASLT2,
    };

  enum Var {
    ele = 0,
    mu = 1,
    leadingele = 2,
    leadingmu = 3,
    subleadingele = 4,
    subleadingmu = 5,
  };

  enum Booleans
    {
      Pass_ll,
      IS_SF,
      IS_ee,
      IS_mm,
      IS_em,
      pass_trigger_SLT,
      pass_trigger_DLT,
      pass_trigger_ASLT1_em,
      pass_trigger_ASLT1_me,
      pass_trigger_ASLT2,
      EXACTLY_TWO_LEPTONS,
      PASS_TRIGGER,
      TWO_OPPOSITE_CHARGE_LEPTONS,
      ATLEAST_TWO_PHOTONS
    };

  /// \brief An algorithm for counting containers
  class HZAllyySelectorAlg final : public EL::AnaAlgorithm {
    
  public:
    HZAllyySelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
    
    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// \brief This is the mirror of initialize() and is called after all events are processed.
    StatusCode finalize() override; ///I added this to write the cutflow histogram.

    const std::vector<std::string> m_STANDARD_CUTS{
      "EXACTLY_TWO_LEPTONS",    
        "PASS_TRIGGER",
        "TWO_OPPOSITE_CHARGE_LEPTONS",
	"ATLEAST_TWO_PHOTONS"
	};


  private :
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadHandle<xAOD::PhotonContainer>
      m_photonHandle{ this, "photons", "llyyAnalysisPhotons_%SYS%", "Photons container to read" };
       
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "llyyAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "llyyAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadDecorHandle<unsigned int> m_year
      {this, "year", "dataTakingYear", ""};
    double m_total_mcEventWeight{0.0};
    CP::SysReadDecorHandle<float> m_generatorWeight
      { this, "generatorWeight", "generatorWeight_%SYS%", "MC event weights" };
    CP::SysReadDecorHandle<bool> m_is17_periodB5_B8
      {this, "is2017_periodB5_B8", "is2017_periodB5_B8", ""};
    CP::SysReadDecorHandle<bool> m_is22_75bunches
      {this, "is2022_75bunches", "is2022_75bunches", ""};
    CP::SysReadDecorHandle<bool> m_is23_75bunches
      {this, "is2023_75bunches", "is2023_75bunches", ""};
    CP::SysReadDecorHandle<bool> m_is23_400bunches
      {this, "is2023_400bunches", "is2023_400bunches", ""};
      
    CP::SysFilterReporterParams m_filterParams {this, "HZAllyy selection"};

    std::unordered_map<HZALLYY::TriggerChannel, std::string> m_triggerChannels = 
      {
        {HZALLYY::SLT, "SLT"},
        {HZALLYY::DLT, "DLT"},
        {HZALLYY::ASLT1_em, "ASLT1_em"},
        {HZALLYY::ASLT1_me, "ASLT1_me"},
        {HZALLYY::ASLT2, "ASLT2"},
      };

    Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };

    std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

    ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

    long long int m_total_events{0};

    std::unordered_map<HZALLYY::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
    std::unordered_map<HZALLYY::Booleans, bool> m_bools;
    std::unordered_map<HZALLYY::Booleans, std::string> m_boolnames{
      {HZALLYY::Pass_ll, "Pass_ll"},
      {HZALLYY::IS_SF, "IS_SF"},
      {HZALLYY::IS_ee, "IS_ee"},
      {HZALLYY::IS_mm, "IS_mm"},
      {HZALLYY::IS_em, "IS_em"},
      {HZALLYY::pass_trigger_SLT, "pass_trigger_SLT"},
      {HZALLYY::pass_trigger_DLT, "pass_trigger_DLT"},
      {HZALLYY::pass_trigger_ASLT1_em, "pass_trigger_ASLT1_em"},
      {HZALLYY::pass_trigger_ASLT1_me, "pass_trigger_ASLT1_me"},
      {HZALLYY::pass_trigger_ASLT2, "pass_trigger_ASLT2"},
      {HZALLYY::EXACTLY_TWO_LEPTONS, "EXACTLY_TWO_LEPTONS"},
      {HZALLYY::PASS_TRIGGER, "PASS_TRIGGER"},
      {HZALLYY::TWO_OPPOSITE_CHARGE_LEPTONS, "TWO_OPPOSITE_CHARGE_LEPTONS"},
      {HZALLYY::ATLEAST_TWO_PHOTONS, "ATLEAST_TWO_PHOTONS"},
        
     };

    CutManager m_llyyCuts;
    Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
    std::vector<HZALLYY::Booleans> m_inputCutKeys;
    Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
    CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};

    std::unordered_map<HZALLYY::TriggerChannel, std::unordered_map<HZALLYY::Var, float>> m_pt_threshold;

    void evaluatePhotonCuts
      (const xAOD::PhotonContainer& photons, CutManager& llyyCuts);
      
    void evaluateTriggerCuts
      (const xAOD::EventInfo* event,
       const xAOD::Electron* ele0, const xAOD::Electron* ele1,
       const xAOD::Muon* mu0, const xAOD::Muon* mu1,
       CutManager& llyyCuts, const CP::SystematicSet& sys);
    void evaluateSingleLeptonTrigger
      (const xAOD::EventInfo* event, 
       const xAOD::Electron* ele, const xAOD::Muon* mu,
       const CP::SystematicSet& sys);
    void evaluateDiLeptonTrigger
      (const xAOD::EventInfo* event,
       const xAOD::Electron* ele0, const xAOD::Electron* ele1,
       const xAOD::Muon* mu0, const xAOD::Muon* mu1,
       const CP::SystematicSet& sys);
    void evaluateAsymmetricLeptonTrigger
      (const xAOD::EventInfo* event,
       const xAOD::Electron* ele, const xAOD::Muon* mu,
       const CP::SystematicSet& sys);

    void evaluateLeptonCuts(const xAOD::ElectronContainer& electrons,
			    const xAOD::MuonContainer& muons, CutManager& llyyCuts);
     
    void setThresholds(const xAOD::EventInfo* event,
		       const CP::SystematicSet& sys);
  };

}

#endif // HZALLYYANALYSIS_HZALLYYSELECTORALG

