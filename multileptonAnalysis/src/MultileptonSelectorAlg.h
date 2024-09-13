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
#include <xAODMuon/MuonContainer.h>
#include <xAODTau/TauJetContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>

#include "SubChannelClassify.h"

namespace MULTILEPTON
{
    enum TriggerChannel
    {
        SLT,
        DLT,
        ASLT,
    };

    enum Var
    {
        ele = 0,
        mu = 1,
        leadingele = 2,
        leadingmu = 3,
        subleadingele = 4,
        subleadingmu = 5,
    };

    enum Booleans
    {
        pass_trigger_SLT,
        pass_trigger_DLT,
        pass_trigger_ASLT,
        PASS_TRIGGER,

        pass_2lss,
        pass_3l,
        pass_bb4l,
        pass_1l2tauhad,
        pass_2l2tauhad,
        pass_2lss1tauhad,
        pass_1l3tauhad,
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

    CP::SysReadHandle<xAOD::TauJetContainer> m_tauHandle{ 
        this, "taus", "hhmlAnalysisTaus_%SYS%", "Tau container to read" };


    CP::SysReadDecorHandle<unsigned int> m_year{
        this, "year", "dataTakingYear",""};

    CP::SysFilterReporterParams m_filterParams{
        this, "Multilepton selection"};


    Gaudi::Property<std::vector<std::string>> m_triggers 
		{ this, "triggerLists", {}, "Name list of trigger" };
		
    ToolHandle<Trig::IMatchingTool> m_matchingTool
		{ this, "trigMatchingTool", "", "Trigger matching tool"};

    std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

    long long int m_total_events{0};
    
    std::unordered_map<MULTILEPTON::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
    std::unordered_map<MULTILEPTON::Booleans, bool> m_bools;
    std::unordered_map<MULTILEPTON::Booleans, std::string> m_boolnames{
        {MULTILEPTON::pass_trigger_SLT, "pass_trigger_SLT"},
        {MULTILEPTON::pass_trigger_DLT, "pass_trigger_DLT"},
        {MULTILEPTON::pass_trigger_ASLT, "pass_trigger_ASLT"},
        {MULTILEPTON::PASS_TRIGGER, "PASS_TRIGGER"},

        {MULTILEPTON::pass_2lss, "pass_2lss"},
        {MULTILEPTON::pass_3l, "pass_3l"},
        {MULTILEPTON::pass_bb4l, "pass_bb4l"},
        {MULTILEPTON::pass_1l2tauhad, "pass_1l2tauhad"},
        {MULTILEPTON::pass_2l2tauhad, "pass_2l2tauhad"},
        {MULTILEPTON::pass_2lss1tauhad, "pass_2lss1tauhad"},
        {MULTILEPTON::pass_1l3tauhad, "pass_1l3tauhad"},
    };

    /// \brief Cutflow Variables
    CutManager m_hhmlCuts;
    Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
    std::vector<MULTILEPTON::Booleans> m_inputCutKeys;
    Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
    double m_total_mcEventWeight{0.0};
    CP::SysReadDecorHandle<float>
		m_generatorWeight{ this, "generatorWeight", "generatorWeight_%SYS%", "MC event weights" };

    std::unordered_map<MULTILEPTON::TriggerChannel, std::unordered_map<MULTILEPTON::Var, float>> m_pt_threshold;

    StatusCode initialiseCutflow();

    void evaluateTriggerCuts(
        const xAOD::EventInfo* event,
        const xAOD::Electron* ele0, const xAOD::Electron* ele1,
        const xAOD::Muon* mu0, const xAOD::Muon* mu1,
        CutManager& hhmlCuts, const CP::SystematicSet& sys);
    
    void evaluateSingleLeptonTrigger(
        const xAOD::EventInfo* event, 
        const xAOD::Electron* ele, const xAOD::Muon* mu,
        const CP::SystematicSet& sys);
    void evaluateDiLeptonTrigger(
        const xAOD::EventInfo* event,
        const xAOD::Electron* ele0, const xAOD::Electron* ele1,
        const xAOD::Muon* mu0, const xAOD::Muon* mu1,
        const CP::SystematicSet& sys);
    void evaluateAsymmetricLeptonTrigger(
        const xAOD::EventInfo* event,
        const xAOD::Electron* ele, const xAOD::Muon* mu,
        const CP::SystematicSet& sys);

    bool evaluate2lssSelection(const SubChannelClassify &classify,
        CutManager& hhmlCuts);

    bool evaluate3lSelection(const SubChannelClassify &classify,
        CutManager& hhmlCuts);

    bool evaluatebb4lSelection(const SubChannelClassify &classify,
        CutManager& hhmlCuts);
    
    bool evaluate1l2tauhadSelection(const SubChannelClassify &classify,
        CutManager& hhmlCuts);
    
    bool evaluate2l2tauhadSelection(const SubChannelClassify &classify,
        CutManager& hhmlCuts);
    
    // NOTE: Ignore 2LSC for the moment
    // bool evaluate2lss1tauhadSelection(
    //     std::vector<std::pair<const xAOD::IParticle*, int>>& leptons,
    //     const xAOD::TauJetContainer *taus,
    //     const xAOD::JetContainer *jets,
    //     const ConstDataVector<xAOD::JetContainer>& bjets,
    //     CutManager& hhmlCuts);

    // TODO: Add 1l3tauhad selection

    void applyChannelSelection(
        const xAOD::ElectronContainer& electrons,
        const xAOD::MuonContainer& muons,
        const xAOD::TauJetContainer& taus,
        const ConstDataVector<xAOD::JetContainer>& bjets,
        CutManager& hhmlCuts);

    void setThresholds(
        const xAOD::EventInfo* event,
        const CP::SystematicSet& sys);
};

} // namespace MULTILEPTON

#endif // MULTILEPTONANALYSIS_MULTILEPTONSELECTORALG
