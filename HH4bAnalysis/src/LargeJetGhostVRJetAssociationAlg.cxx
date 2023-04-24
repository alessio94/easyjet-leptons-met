/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LargeJetGhostVRJetAssociationAlg.h"

namespace HH4B
{
  LargeJetGhostVRJetAssociationAlg ::LargeJetGhostVRJetAssociationAlg(
      const std::string &name, ISvcLocator *pSvcLocator)
      : AthAlgorithm(name, pSvcLocator)
  {
    declareProperty("workingPoints", m_workingPoints,
                    "the working points to select VR track jets with");
  }

  StatusCode LargeJetGhostVRJetAssociationAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (m_workingPoints.empty())
    {
      ATH_MSG_ERROR("A list of working points needs to be provided");
      return StatusCode::FAILURE;
    }

    for (auto &workingpoint : m_workingPoints)
    {
      m_isBtagAccessors.emplace_back("ftag_select_" + workingpoint);
      m_leadingVRTrackJetBtagDecorators.emplace_back(
          "leadingVRTrackJetsBtag_" + workingpoint);
    }

    ATH_CHECK(m_largeJetInKey.initialize());
    ATH_CHECK(m_eventInfoKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode LargeJetGhostVRJetAssociationAlg ::execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::JetContainer> largeRJets(m_largeJetInKey);
    ATH_CHECK(largeRJets.isValid());
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    ATH_CHECK(recordVRTrackJetGhostAssociation(*largeRJets, *eventInfo));

    return StatusCode::SUCCESS;
  }

  StatusCode
  LargeJetGhostVRJetAssociationAlg ::recordVRTrackJetGhostAssociation(
      const xAOD::JetContainer &largeRJets,
      const xAOD::EventInfo &eventInfo) const
  {
    ATH_MSG_DEBUG("Saving large-R jets as \"" << m_largeJetInKey.key()
                                              << "\".");

    int maxLeadingGAVRjetsSize = 3;
    char passRelativeDeltaRToVRJetCut = 1;

    for (auto *largejet : largeRJets)
    {
      // get ghost associated VR track jets from untrimmed large R jet
      const xAOD::Jet *untrimmedLargeRJet =
          *m_largeRUntrimmedAccessor(*largejet);
      std::vector<ELPC> ilargeRjet_ghostVRjets =
          m_ghostVRTrackJetsAccessor(*untrimmedLargeRJet);

      // Select good VR track jets that pass pT > 10GeV and |eta| < 2.5
      ilargeRjet_ghostVRjets.erase(
          std::remove_if(
              ilargeRjet_ghostVRjets.begin(), ilargeRjet_ghostVRjets.end(),
              [&](ELPC &vrjet)
              {
                return !(vrjet.isValid() && (*vrjet)->pt() > 10000. &&
                         std::abs((*vrjet)->eta()) < 2.5);
              }),
          ilargeRjet_ghostVRjets.end());

      bool passesDRcut =
          std::find_if(ilargeRjet_ghostVRjets.begin(),
                       ilargeRjet_ghostVRjets.end(),
                       [&](ELPC &vrjet) {
                         return relativeDeltaRToVRJet(**vrjet) >= 1.0;
                       }) != ilargeRjet_ghostVRjets.end();

      if (!passesDRcut)
      {
        ATH_MSG_VERBOSE("VR track jets overlap found, recording to "
                        "EventInfo.passRelativeDeltaRToVRJetCut");
        passRelativeDeltaRToVRJetCut = 0;
      }

      ATH_MSG_VERBOSE("Number of VR jets per large R jet: "
                      << ilargeRjet_ghostVRjets.size());

      m_goodVRTrackJetCountDecorator(*largejet) =
          ilargeRjet_ghostVRjets.size();

      // Sort VR track jets by leading pT
      std::sort(ilargeRjet_ghostVRjets.begin(), ilargeRjet_ghostVRjets.end(),
                [](ELPC &left, ELPC &right)
                { return (*left)->pt() > (*right)->pt(); });

      // loop over the leading VR track jets in the large R jet
      int leadingGAVRjetsSize = ilargeRjet_ghostVRjets.size();
      int minLeadingGAVRjetsSize = leadingGAVRjetsSize > 2
                                       ? maxLeadingGAVRjetsSize
                                       : leadingGAVRjetsSize;

      // hold btag decisions
      std::vector<std::vector<char>> btags(
          m_workingPoints.size(), std::vector<char>(minLeadingGAVRjetsSize));

      // Initialize to some "invalid" values
      std::vector<float> leadingGAVRJetPt(minLeadingGAVRjetsSize);
      std::vector<float> leadingGAVRJetEta(minLeadingGAVRjetsSize);
      std::vector<float> leadingGAVRJetPhi(minLeadingGAVRjetsSize);
      std::vector<float> leadingGAVRJetM(minLeadingGAVRjetsSize);
      std::vector<int> HadronConeExclTruthLabelID(minLeadingGAVRjetsSize);

      float deltaR12{-1};
      float deltaR13{-1};
      float deltaR32{-1};

      for (int i = 0; i < minLeadingGAVRjetsSize; i++)
      {
        const xAOD::Jet *leadingVRJet =
            dynamic_cast<const xAOD::Jet *>(*ilargeRjet_ghostVRjets.at(i));

        // record btagging
        for (size_t j = 0; j < m_workingPoints.size(); j++)
        {
          char btagged = m_isBtagAccessors[j](*leadingVRJet);
          btags[j].at(i) = btagged;
        }

        // compute and recorddeltaR's
        ATH_MSG_VERBOSE("VR jet pt: " << leadingVRJet->pt()
                                      << ", eta: " << leadingVRJet->eta()
                                      << ", phi: " << leadingVRJet->phi()
                                      << ", m: " << leadingVRJet->m());
        leadingGAVRJetPt.at(i) = leadingVRJet->pt();
        leadingGAVRJetEta.at(i) = leadingVRJet->eta();
        leadingGAVRJetPhi.at(i) = leadingVRJet->phi();
        leadingGAVRJetM.at(i) = leadingVRJet->m();
        if (m_isMC) {
          HadronConeExclTruthLabelID.at(i) = m_TruthLabelAccessor(*leadingVRJet);
        }

        if (leadingGAVRjetsSize > 1 && i == 0)
        {
          const xAOD::Jet *secondLeadingVRJet =
              dynamic_cast<const xAOD::Jet *>(*ilargeRjet_ghostVRjets.at(1));

          deltaR12 = ROOT::Math::VectorUtil::DeltaR(
              leadingVRJet->jetP4(), secondLeadingVRJet->jetP4());
          ATH_MSG_VERBOSE("leading VR track jets deltaR12: " << deltaR12);
        }

        if (leadingGAVRjetsSize > 2 && i == 0)
        {
          const xAOD::Jet *thirdLeadingVRJet =
              dynamic_cast<const xAOD::Jet *>(*ilargeRjet_ghostVRjets.at(2));

          deltaR13 = ROOT::Math::VectorUtil::DeltaR(
              leadingVRJet->jetP4(), thirdLeadingVRJet->jetP4());
          ATH_MSG_VERBOSE("leading VR track jets deltaR13: " << deltaR13);
        }

        if (leadingGAVRjetsSize > 2 && i == 1)
        {
          const xAOD::Jet *thirdLeadingVRJet =
              dynamic_cast<const xAOD::Jet *>(*ilargeRjet_ghostVRjets.at(2));

          deltaR32 = ROOT::Math::VectorUtil::DeltaR(thirdLeadingVRJet->jetP4(),
                                                    leadingVRJet->jetP4());
          ATH_MSG_VERBOSE("leading VR track jets deltaR32: " << deltaR32);
        }
      }

      m_leadingVRTrackJetPtDecorator(*largejet) = leadingGAVRJetPt;
      m_leadingVRTrackJetEtaDecorator(*largejet) = leadingGAVRJetEta;
      m_leadingVRTrackJetPhiDecorator(*largejet) = leadingGAVRJetPhi;
      m_leadingVRTrackJetMDecorator(*largejet) = leadingGAVRJetM;
      m_leadingVRTrackJetDeltaR12Decorator(*largejet) = deltaR12;
      m_leadingVRTrackJetDeltaR13Decorator(*largejet) = deltaR13;
      m_leadingVRTrackJetDeltaR32Decorator(*largejet) = deltaR32;
      if (m_isMC) {
        m_HadronConeExclTruthLabelIDDecorator(*largejet) = HadronConeExclTruthLabelID;
      }
      for (size_t j = 0; j < m_workingPoints.size(); j++)
      {
        m_leadingVRTrackJetBtagDecorators[j](*largejet) = btags[j];
      }
    }

    m_passRelativeDeltaRToVRJetCutDecorator(eventInfo) =
        passRelativeDeltaRToVRJetCut;

    return StatusCode::SUCCESS;
  }
}
