/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef ZCHARMANALYSIS_ZCHARMTRUTHSELECTORALG
#define ZCHARMANALYSIS_ZCHARMTRUTHSELECTORALG

#include <memory>
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODJet/JetAuxContainer.h>
#include <xAODTruth/TruthParticleContainer.h> 
#include <xAODTruth/TruthParticleAuxContainer.h>
#include <xAODTruth/TruthEvent.h>
#include <xAODTruth/TruthEventContainer.h>
#include <xAODTruth/TruthParticle.h>


#include <EasyjetHub/CutManager.h>
#include "ZCharmEnums.h"


namespace ZCC
{

  /// \brief An algorithm for counting containers
  class ZCharmTruthSelectorAlg final : public EL::AnaAlgorithm {

    public:
      ZCharmTruthSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute(/*const EventContext& ctx*/) override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private :

      const std::vector<std::string> m_BASELINE_CUTS{
        "EXACTLY_TWO_LEPTONS_TRUTH",
        "OPPOSITE_CHARGE_LEPTONS_TRUTH",
        "DILEPTON_MASS_WINDOW_TRUTH",
      };

      Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };


      SG::ReadHandleKey<xAOD::JetContainer>
      m_truthjetsInKey{ this, "truthSmallJetInContainer", "", "Truth jet container to read" };

      SG::ReadHandleKey<xAOD::JetContainer>
      m_truthlargejetInKey{ this, "truthLargeJetInContainer", "", "Truth large R Jet container to read" };

      SG::ReadHandleKey<xAOD::TruthEventContainer>
      m_eventInKey{ this, "event", "TruthEvents", "TruthEvent container to read" };

      SG::ReadHandleKey<xAOD::TruthParticleContainer>
      m_truthelectronInKey{ this, "truthElectronInContainer", "", "Truth electron container to read" };

      SG::ReadHandleKey<xAOD::TruthParticleContainer>
      m_truthmuonInKey{ this, "truthMuonInContainer", "", "Truth muon container to read" };

      std::unordered_map<std::string, SG::ReadDecorHandleKey<xAOD::JetContainer>> m_bjetDecorHandleKeys;

      std::unordered_map<std::string, SG::ReadDecorHandleKey<xAOD::JetContainer>> m_cjetDecorHandleKeys;

      SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetTruthFlavourKey{this, "jetTruthFlavourDecoration", "", "Name of jet truth flavour decoration"};

      long long int m_total_events{0};

      std::unordered_map<ZCC::Booleans, SG::WriteDecorHandleKey<xAOD::TruthEventContainer>> m_BbranchKeys;
      std::unordered_map<ZCC::Booleans, bool> m_bools;
      std::unordered_map<ZCC::Booleans, std::string> m_boolnames{
        {ZCC::IS_ee_TRUTH, "IS_ee_TRUTH"},
        {ZCC::IS_mm_TRUTH, "IS_mm_TRUTH"},
        {ZCC::IS_em_TRUTH, "IS_em_TRUTH"},
        {ZCC::EXACTLY_TWO_LEPTONS_TRUTH, "EXACTLY_TWO_LEPTONS_TRUTH"},
        {ZCC::OPPOSITE_CHARGE_LEPTONS_TRUTH, "OPPOSITE_CHARGE_LEPTONS_TRUTH"},
        {ZCC::DILEPTON_MASS_WINDOW_TRUTH, "DILEPTON_MASS_WINDOW_TRUTH"},
        {ZCC::ONE_B_JETS_TRUTH, "ONE_B_JETS_TRUTH"},
        {ZCC::TWO_B_JETS_TRUTH, "TWO_B_JETS_TRUTH"},
        {ZCC::ONE_C_JETS_TRUTH, "ONE_C_JETS_TRUTH"},
        {ZCC::TWO_C_JETS_TRUTH, "TWO_C_JETS_TRUTH"},
        {ZCC::ONE_LARGE_JET_TRUTH, "ONE_LARGE_JET_TRUTH"},
      };

      CutManager m_ZCharmTruthCuts;
      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "truthcutList", {}};
      std::vector<ZCC::Booleans> m_inputCutKeys;
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      
      
      SG::WriteDecorHandleKey<xAOD::TruthEventContainer> m_passTruthCutsKey{
        this, "PassTruthCutsKey", "TruthEvents.PassTruthCuts", "Decorate event with pass/fail flag"
      };


      // Floating point variables for truth c-jets and Z boson
      Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

      std::unordered_map<std::string, SG::WriteDecorHandleKey<xAOD::TruthEventContainer>>
        m_FbranchesKeys;

      std::unordered_map<std::string, SG::WriteDecorHandle<xAOD::TruthEventContainer, float>> m_Fbranches;

      void evaluateTruthLeptonCuts(const xAOD::TruthEvent& truthevent, const xAOD::TruthParticleContainer& truthElectrons, const xAOD::TruthParticleContainer& truthMuons, CutManager& ZCharmTruthCuts);
      void evaluateTruthBJetCuts(const ConstDataVector<xAOD::JetContainer>& truthBjets, CutManager& ZCharmTruthCuts);
      void evaluateTruthCJetCuts(const ConstDataVector<xAOD::JetContainer>& truthCjets, CutManager& ZCharmTruthCuts);
      void evaluateTruthLargeJetCuts(const xAOD::JetContainer& truthLargeJets);
      void setThresholds(const xAOD::TruthEvent* event);
      StatusCode initialiseCutflow(); 
  };

}

#endif 
