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
      // hold btag decisions
      std::vector<std::vector<char>> btags(m_workingPoints.size(),
                                           std::vector<char>(3, 0));

      // loop over the leading VR track jets in the large R jet
      for (int i = 0; i < 3; i++)
      {
        try
        {
          auto vrjet = ilargeRjet_ghostVRjets.at(i);
          ATH_MSG_VERBOSE("VR jet pt: " << (*vrjet)->pt()
                                        << ", eta: " << (*vrjet)->eta()
                                        << ", phi: " << (*vrjet)->phi()
                                        << ", m: " << (*vrjet)->m());
          leadingGAVRJetPt.push_back((*vrjet)->pt());
          leadingGAVRJetEta.push_back((*vrjet)->eta());
          leadingGAVRJetPhi.push_back((*vrjet)->phi());
          leadingGAVRJetM.push_back((*vrjet)->m());

          for (size_t i = 0; i < m_workingPoints.size(); i++)
          {
            char btagged = m_isBtagAccessors[i](**vrjet);
            btags[i].push_back(btagged);
          }
        }
        catch (const std::out_of_range &oor)
        {
          // the large R jet has less than 3 ghost associated VR jets,
          // just continue and assign the default values
        }
      }

      m_leadingVRTrackJetPtDecorator(*largejet) = leadingGAVRJetPt;
      m_leadingVRTrackJetEtaDecorator(*largejet) = leadingGAVRJetEta;
      m_leadingVRTrackJetPhiDecorator(*largejet) = leadingGAVRJetPhi;
      m_leadingVRTrackJetMDecorator(*largejet) = leadingGAVRJetM;
      for (size_t i = 0; i < m_workingPoints.size(); i++)
      {
        m_leadingVRTrackJetBtagDecorators[i](*largejet) = btags[i];
      }
    }

    return StatusCode::SUCCESS;
  }
}
