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

    return StatusCode::SUCCESS;
  }

  StatusCode LargeJetGhostVRJetAssociationAlg ::execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<JC> largeRJets(m_largeJetInKey);
    ATH_CHECK(largeRJets.isValid());

    ATH_CHECK(recordVRTrackJetGhostAssociation(*largeRJets));

    return StatusCode::SUCCESS;
  }

  StatusCode
  LargeJetGhostVRJetAssociationAlg ::recordVRTrackJetGhostAssociation(
      const JC &largeRJets) const
  {
    ATH_MSG_DEBUG("Saving large-R jets as \"" << m_largeJetInKey.key()
                                              << "\".");

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

      ATH_MSG_VERBOSE("Number of VR jets per large R jet: "
                      << ilargeRjet_ghostVRjets.size());

      m_goodVRTrackJetCountDecorator(*largejet) =
          ilargeRjet_ghostVRjets.size();

      // Sort VR track jets by leading pT
      std::sort(ilargeRjet_ghostVRjets.begin(), ilargeRjet_ghostVRjets.end(),
                [](ELPC &left, ELPC &right)
                { return (*left)->pt() > (*right)->pt(); });

      // Initialize to some "invalid" values
      std::vector<float> leadingGAVRJetPt(3, -1);
      std::vector<float> leadingGAVRJetEta(3, -100);
      std::vector<float> leadingGAVRJetPhi(3, -100);
      std::vector<float> leadingGAVRJetM(3, -1);
      float deltaR12{-1};
      float deltaR13{-1};
      float deltaR32{-1};
      // hold btag decisions
      std::vector<std::vector<char>> btags(m_workingPoints.size(),
                                           std::vector<char>(3, 0));

      // loop over the leading VR track jets in the large R jet
      for (int i = 0; i < 3; i++)
      {
        try
        {
          const xAOD::Jet *leadingVRJet =
              dynamic_cast<const xAOD::Jet *>(*ilargeRjet_ghostVRjets.at(i));

          // record btagging
          for (size_t i = 0; i < m_workingPoints.size(); i++)
          {
            char btagged = m_isBtagAccessors[i](*leadingVRJet);
            btags[i].push_back(btagged);
          }

          // compute and recorddeltaR's
          ATH_MSG_VERBOSE("VR jet pt: " << leadingVRJet->pt()
                                        << ", eta: " << leadingVRJet->eta()
                                        << ", phi: " << leadingVRJet->phi()
                                        << ", m: " << leadingVRJet->m());
          leadingGAVRJetPt.push_back(leadingVRJet->pt());
          leadingGAVRJetEta.push_back(leadingVRJet->eta());
          leadingGAVRJetPhi.push_back(leadingVRJet->phi());
          leadingGAVRJetM.push_back(leadingVRJet->m());

          if (ilargeRjet_ghostVRjets.size() > 1 && i == 0)
          {
            const xAOD::Jet *secondLeadingVRJet =
                dynamic_cast<const xAOD::Jet *>(*ilargeRjet_ghostVRjets.at(1));

            deltaR12 = ROOT::Math::VectorUtil::DeltaR(
                leadingVRJet->jetP4(), secondLeadingVRJet->jetP4());
            ATH_MSG_VERBOSE("leading VR track jets deltaR12: " << deltaR12);
          }

          if (ilargeRjet_ghostVRjets.size() > 2 && i == 0)
          {
            const xAOD::Jet *thirdLeadingVRJet =
                dynamic_cast<const xAOD::Jet *>(*ilargeRjet_ghostVRjets.at(2));

            deltaR13 = ROOT::Math::VectorUtil::DeltaR(
                leadingVRJet->jetP4(), thirdLeadingVRJet->jetP4());
            ATH_MSG_VERBOSE("leading VR track jets deltaR13: " << deltaR13);
          }

          if (ilargeRjet_ghostVRjets.size() > 2 && i == 1)
          {
            const xAOD::Jet *thirdLeadingVRJet =
                dynamic_cast<const xAOD::Jet *>(*ilargeRjet_ghostVRjets.at(2));

            deltaR32 = ROOT::Math::VectorUtil::DeltaR(
                thirdLeadingVRJet->jetP4(), leadingVRJet->jetP4());
            ATH_MSG_VERBOSE("leading VR track jets deltaR32: " << deltaR32);
          }
        }
        catch (const std::out_of_range &oor)
        {
          // the large R jet has less than 3 ghost associated VR jets,
          // just continue and assign the default values
          continue;
        }
      }

      m_leadingVRTrackJetPtDecorator(*largejet) = leadingGAVRJetPt;
      m_leadingVRTrackJetEtaDecorator(*largejet) = leadingGAVRJetEta;
      m_leadingVRTrackJetPhiDecorator(*largejet) = leadingGAVRJetPhi;
      m_leadingVRTrackJetMDecorator(*largejet) = leadingGAVRJetM;
      m_leadingVRTrackJetDeltaR12Decorator(*largejet) = deltaR12;
      m_leadingVRTrackJetDeltaR13Decorator(*largejet) = deltaR13;
      m_leadingVRTrackJetDeltaR32Decorator(*largejet) = deltaR32;
      for (size_t i = 0; i < m_workingPoints.size(); i++)
      {
        m_leadingVRTrackJetBtagDecorators[i](*largejet) = btags[i];
      }
    }

    return StatusCode::SUCCESS;
  }
}
