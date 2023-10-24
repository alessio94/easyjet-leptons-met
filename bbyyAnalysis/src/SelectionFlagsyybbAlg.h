/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef SELECTIONFLAGSYYBBALG_H
#define SELECTIONFLAGSYYBBALG_H


#include <AthContainers/ConstDataVector.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include "CutManager.h"

class CutManager;

namespace HHBBYY
{

  /// \brief An algorithm for counting containers
  class SelectionFlagsyybbAlg final : public AthHistogramAlgorithm {

    public:
        SelectionFlagsyybbAlg(const std::string &name, ISvcLocator *pSvcLocator);

        /// \brief Initialisation method, for setting up tools and other persistent
        /// configs
        StatusCode initialize() override;
        /// \brief Execute method, for actions to be taken in the event loop
        StatusCode execute() override;
        /// \brief This is the mirror of initialize() and is called after all events are processed.
        StatusCode finalize() override; ///I added this to write the cutflow histogram.

        const std::vector<std::string> m_STANDARD_CUTS{
            "PASS_TRIGGER",
            "TWO_LOOSE_PHOTONS",
            "TWO_TIGHTID_PHOTONS",
            "TWO_ISO_PHOTONS",
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

        static void evaluateTriggerCuts(const xAOD::EventInfo& eventInfo, const std::vector<std::string> &photonTriggers, CutManager& yybbCuts);
        static void evaluatePhotonCuts(const ConstDataVector<xAOD::PhotonContainer>& photons, CutManager& yybbCuts);
        static void evaluateLeptonCuts(const ConstDataVector<xAOD::ElectronContainer>& electrons,
                            const ConstDataVector<xAOD::MuonContainer>& muons, CutManager& yybbCuts);
        static void evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& smallRJets_btag,
                            const ConstDataVector<xAOD::JetContainer>& smallRJets, CutManager& yybbCuts);

private :
        CutManager yybbCuts;

        SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer> >
        m_smallRJets_BTag_ContainerInKey{ this, "smallRJets_BTag_ContainerInKey",
                                "",   "containerName to read" };


        SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer> >
        m_smallRJets_ContainerInKey{ this, "smallRJets_ContainerInKey",
                                "",   "Jet container without WP to read" };

        SG::ReadHandleKey<ConstDataVector<xAOD::PhotonContainer> >
        m_photonContainerInKey{ this, "photonContainerInKey",
                                "",   "containerName to read" };

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
        std::vector<std::string> m_inputCutList{};
        bool m_saveCutFlow;
        std::vector<std::string> m_photonTriggers{};
        long long int m_total_events{0};
        std::string m_triggerCut{};

    };

}

#endif // SELECTIONFLAGSYYBBALG_H
 