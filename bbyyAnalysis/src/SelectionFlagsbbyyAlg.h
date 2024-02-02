/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef SELECTIONFLAGSYYBBALG_H
#define SELECTIONFLAGSYYBBALG_H

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AthContainers/ConstDataVector.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <EasyjetHub/CutManager.h>

class CutManager;

namespace HHBBYY
{

  /// \brief An algorithm for counting containers
  class SelectionFlagsbbyyAlg final : public AthHistogramAlgorithm {

    public:
      SelectionFlagsbbyyAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

      const std::vector<std::string> m_STANDARD_CUTS{
          "PASS_TRIGGER",
          "TWO_TIGHTID_ISO_PHOTONS",
          "PASS_RELPT",
          "DIPHOTON_MASS",
          "EXACTLY_ZERO_LEPTONS",
          "AT_LEAST_TWO_JETS",
          "LESS_THAN_SIX_CENTRAL_JETS",
          "AT_LEAST_ONE_B_JET",
          "AT_LEAST_TWO_B_JETS",
          "EXACTLY_ONE_B_JET",
          "EXACTLY_TWO_B_JETS"
      };

      void evaluateTriggerCuts(const xAOD::EventInfo& eventInfo, 
                          const std::vector<std::string> &photonTriggers, CutManager& bbyyCuts);
      void evaluatePhotonCuts(const xAOD::PhotonContainer& photons, CutManager& bbyyCuts);
      void evaluateLeptonCuts(const xAOD::ElectronContainer& electrons,
                          const xAOD::MuonContainer& muons, CutManager& bbyyCuts);
      void evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
                          const xAOD::JetContainer& jets, CutManager& bbyyCuts);

    private :
      // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
      // "someInfo"};

      /// \brief Setup syst-aware input container handles
      CutManager m_bbyyCuts;
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "",   "Jet container to read" };

      CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

      CP::SysReadHandle<xAOD::PhotonContainer>
      m_photonHandle{ this, "photons", "",   "Photons container to read" };

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "",   "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "",   "Muon container to read" };

      std::vector<std::string> m_inputCutList{};

      std::vector<std::string> m_photonTriggers;

      std::unordered_map<std::string,  SG::ReadDecorHandleKey<xAOD::EventInfo>> m_triggerDecorKeys;

      bool m_saveCutFlow;
      long long int m_total_events{0};
      float m_total_mcEventWeight{0.f};
      std::vector<float> eventWeights{0.f};

      Gaudi::Property<bool> m_isMC
        { this, "isMC", false, "Is this simulation?" };

      SG::ReadDecorHandleKey<xAOD::EventInfo> m_mcEventWeightsKey{
        this, "mcEventWeights", "EventInfo.mcEventWeights", "mc event weights"};

      std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;

      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
  };

}

#endif // SELECTIONFLAGSYYBBALG_H
