
#include "DiHiggsAnalysis.h"
#include "AthenaBaseComps/AthCommonMsg.h"
#include "DiHiggsAnalysisHelpers.h"
#include "Math/GenVector/VectorUtil.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <AsgMessaging/MessageCheck.h>
#include <AthContainers/ConstDataVector.h>
#include <SystematicsHandles/SysReadHandle.h>

// for ANA_MSG_BLAH
using namespace asg::msgUserCode;

namespace HH4B
{
  std::vector<std::string>
  getHiggsVarsNames(std::vector<std::string> &btag_wps,
                    std::vector<std::string> &vr_btag_wps)
  {
    // clang-format off
    std::vector<std::string> resolvedVars =
    {
      "resolved_nCentralJets_",
      "resolved_nBtaggedCentralJets_",
      "resolved_jet1_pt_",
      "resolved_jet2_pt_",
      "resolved_jet3_pt_",
      "resolved_jet4_pt_",
      "resolved_DeltaR12_",
      "resolved_DeltaR13_",
      "resolved_DeltaR14_",
      "resolved_DeltaR23_",
      "resolved_DeltaR24_",
      "resolved_DeltaR34_",
      "resolved_h1_m_",
      "resolved_h2_m_",
      "resolved_h1_dR_jets_",
      "resolved_h2_dR_jets_",
      "resolved_hh_m_",
      "resolved_h1_fromSameInitialParticle_",
      "resolved_h2_fromSameInitialParticle_",
      "resolved_h1_dR_leadingJet_closestB_",
      "resolved_h2_dR_leadingJet_closestB_",
      "resolved_h1_dR_subleadingJet_closestB_",
      "resolved_h2_dR_subleadingJet_closestB_",
    };
    std::vector<std::string> boostedVars =
    {
      "boosted_nLargeJets_",
      "boosted_h1_m_",
      "boosted_h1_jet1_pt_",
      "boosted_h1_jet2_pt_",
      "boosted_h1_dR_jets_",
      "boosted_h2_m_",
      "boosted_h2_jet1_pt_",
      "boosted_h2_jet2_pt_",
      "boosted_h2_dR_jets_",
      "boosted_hh_m_",
      "boosted_h1_nGhostAssocVrJets_",
      "boosted_h1_nBtaggedGhostAssocVrTrackJets_",
      "boosted_h2_nGhostAssocVrJets_",
      "boosted_h2_nBtaggedGhostAssocVrTrackJets_",
    }; // clang-format on

    // attach btagging working points to vars
    std::vector<std::string> vars;

    for (std::string var : resolvedVars)
    {
      for (std::string wp : btag_wps)
      {
        vars.push_back(var + wp);
      }
    }
    for (std::string var : boostedVars)
    {
      for (std::string wp : vr_btag_wps)
      {
        vars.push_back(var + wp);
      }
    }
    return vars;
  }

  void DiHiggsAnalysis::initHiggsVarsMap(std::vector<std::string> &higgsVars)
  {
    float initValue = -1.;
    for (std::string var : higgsVars)
    {
      m_higgsVarsMap.insert_or_assign(var, initValue);
    }
    return;
  }

  void
  DiHiggsAnalysis::makeResolvedAnalysis(const xAOD::JetContainer &smallRjets,
                                        std::string wp, bool isMC)
  {
    // leadding higgs candidate
    HiggsCandidate h1;
    // subleading higgs candidate
    HiggsCandidate h2;

    // this is a shallow copy container
    ConstDataVector<xAOD::JetContainer> bTaggedJets(SG::VIEW_ELEMENTS);
    // accessor for the btagging info on the jet
    static const SG::AuxElement::ConstAccessor<char> isBtag("ftag_select_" +
                                                            wp);
    // use some truth info if we have MC
    ConstDataVector<xAOD::TruthParticleContainer> truthInitialParticles(
        SG::VIEW_ELEMENTS);
    static const SG::AuxElement::Decorator<std::vector<float>> dRtoTruthBs_dec(
        "dRtoTruthBs");
    std::vector<const xAOD::TruthParticle *> truthBs;
    if (isMC)
    {
      truthBs = getTruthBs();
      // fix for now if there are now b's
      if (truthBs.size() < 1)
      {
        isMC = false;
      }
    }

    // remove non-btagged jets of designated wp
    // (fancy std::remove_if from boosted analysis only works with
    // std::vector)
    bool isCentral;
    int nCentralJets = 0;
    int nBtaggedCentralJets = 0;
    for (const xAOD::Jet *jet : smallRjets)
    {
      // count central jets
      isCentral = false;
      if (jet->pt() > 25000. && std::abs(jet->eta()) < 2.5)
      {
        nCentralJets += 1;
        isCentral = true;
      }
      // get deltaR's to truth b's from initial process
      // set default value
      dRtoTruthBs_dec(*jet) = std::vector<float>{-1};
      if (isMC)
      {
        std::vector<float> dRtoTruthBs;
        for (const xAOD::TruthParticle *tp : truthBs)
        {
          // get deltaR to reco jet from each truth b
          float dR =
              ROOT::Math::VectorUtil::DeltaR(tp->genvecP4(), jet->jetP4());
          dRtoTruthBs.push_back(dR);
        }
        // decorate jet
        dRtoTruthBs_dec(*jet) = dRtoTruthBs;
      }

      // the ftag sequence does not tag all of them
      if (!isBtag.isAvailable(*jet))
      {
        ANA_MSG_DEBUG("Skipped non B-tagged jet");
        continue;
      }

      // keep only btagged jets with a designated wp
      if (isBtag(*jet))
      {
        bTaggedJets.push_back(jet);
        // count btagged central jets
        if (isCentral)
        {
          nBtaggedCentralJets += 1;
        }
      }

    } // end of 0.4 jet loop

    // write out counts
    m_higgsVarsMap["resolved_nCentralJets_" + wp] = nCentralJets;
    m_higgsVarsMap["resolved_nBtaggedCentralJets_" + wp] = nBtaggedCentralJets;

    // check if we have at least 4 btagged jets
    if (bTaggedJets.size() < 4)
    {
      return;
    }

    ConstDataVector<xAOD::JetContainer> ptSortedBtaggedJets(
        bTaggedJets.begin(), bTaggedJets.end(), SG::VIEW_ELEMENTS);

    // taken from TJ's tutorial
    // https://gitlab.cern.ch/khoo/athanalysistutorial/-/blob/r22/MySecondAthAnalysis/src/DileptonFinderAlg.cxx
    //
    // with std::partial_sort you only have to sort an N-sized container
    // instead of sorting a ptSortedBtaggedJets.size() container
    std::partial_sort(
        ptSortedBtaggedJets.begin(), // Iterator from which to start sorting
        ptSortedBtaggedJets.begin() + 4, // Use begin + N to sort first N
        ptSortedBtaggedJets.end(), // Iterator marking the end of range to sort
        [](const xAOD::IParticle *left, const xAOD::IParticle *right)
        {
          return left->pt() > right->pt();
        }); // lambda function here just handy, could also be another
            // function that returns bool

    // get the leading b-jet for the leading Higgs candidate
    h1.m_leadingJet = ptSortedBtaggedJets[0];

    // calculate dR to the next three leading jets and decorate jet
    static const SG::AuxElement::Decorator<float> dRtoLeadingJet_dec(
        "dRtoLeadingJet");
    static const SG::AuxElement::ConstAccessor<float> dRtoLeadingJet_acc(
        "dRtoLeadingJet");

    // keep only the four leading ones
    ptSortedBtaggedJets.erase(ptSortedBtaggedJets.begin() + 4,
                              ptSortedBtaggedJets.end());

    // decorate dR(jet,leading jet) to each jet
    bool firstJet = true;
    for (const xAOD::Jet *jet : ptSortedBtaggedJets)
    {
      // more instructive than done with iterators
      if (firstJet)
      {
        dRtoLeadingJet_dec(*jet) = 0;
        firstJet = false;
        continue;
      }
      dRtoLeadingJet_dec(*jet) = ROOT::Math::VectorUtil::DeltaR(
          jet->jetP4(), h1.m_leadingJet->jetP4());
    }

    // now find the closest one
    ConstDataVector<xAOD::JetContainer> dRsortedJets = ptSortedBtaggedJets;
    std::partial_sort(
        dRsortedJets.begin(),     // Iterator from which to start sorting
        dRsortedJets.begin() + 4, // Use begin + N to sort first N
        dRsortedJets.end(), // Iterator marking the end of the range to sort
        [](const xAOD::IParticle *left, const xAOD::IParticle *right)
        { return dRtoLeadingJet_acc(*left) > dRtoLeadingJet_acc(*right); });

    // first one is dr to the leading itself, therefore second one
    h1.m_subleadingJet = dRsortedJets[1];

    // get the jets for h2
    if (dRsortedJets[2]->pt() > dRsortedJets[3]->pt())
    {
      h2.m_leadingJet = dRsortedJets[2];
      h2.m_subleadingJet = dRsortedJets[3];
    }
    else
    {
      h2.m_leadingJet = dRsortedJets[3];
      h2.m_subleadingJet = dRsortedJets[2];
    }

    // calculate higgs candidate four vector and dR between jets in Higgs
    // candidate
    h1.m_fourVector = h1.m_leadingJet->jetP4() + h1.m_subleadingJet->jetP4();
    h1.m_dRjets = dRtoLeadingJet_acc(*h1.m_subleadingJet);
    h2.m_fourVector = h2.m_leadingJet->jetP4() + h2.m_subleadingJet->jetP4();
    h2.m_dRjets = ROOT::Math::VectorUtil::DeltaR(h2.m_leadingJet->jetP4(),
                                                 h2.m_subleadingJet->jetP4());

    // find here if leading and subleading fit the truths and come from same
    // higgs
    if (isMC)
    {
      // h1
      m_higgsVarsMap["resolved_h1_fromSameInitialParticle_" + wp] = 0.;
      h1.m_leadingJetClosestB = getClosestB(h1.m_leadingJet, truthBs);
      h1.m_subleadingJetClosestB = getClosestB(h1.m_subleadingJet, truthBs);
      // check if jets come from same initial particle
      if (h1.m_leadingJetClosestB.particle->parent()->barcode() ==
          h1.m_subleadingJetClosestB.particle->parent()->barcode())
      {
        m_higgsVarsMap["resolved_h1_fromSameInitialParticle_" + wp] = 1.;
      }
      // h2
      m_higgsVarsMap["resolved_h2_fromSameInitialParticle_" + wp] = 0.;
      h2.m_leadingJetClosestB = getClosestB(h2.m_leadingJet, truthBs);
      h2.m_subleadingJetClosestB = getClosestB(h2.m_subleadingJet, truthBs);
      // check if jets come from same initial particle
      if (h2.m_leadingJetClosestB.particle->parent()->barcode() ==
          h2.m_subleadingJetClosestB.particle->parent()->barcode())
      {
        m_higgsVarsMap["resolved_h2_fromSameInitialParticle_" + wp] = 1.;
      }

      // decorate dR's
      // clang-format off
      m_higgsVarsMap["resolved_h1_dR_leadingJet_closestB_" + wp] = h1.m_leadingJetClosestB.dR;
      m_higgsVarsMap["resolved_h2_dR_leadingJet_closestB_" + wp] = h1.m_subleadingJetClosestB.dR;
      m_higgsVarsMap["resolved_h1_dR_subleadingJet_closestB_" + wp] = h2.m_leadingJetClosestB.dR;
      m_higgsVarsMap["resolved_h2_dR_subleadingJet_closestB_" + wp] = h2.m_subleadingJetClosestB.dR;
      // clang-format on
    }

    // write to map
    // clang-format off
    m_higgsVarsMap["resolved_jet1_pt_" + wp] = ptSortedBtaggedJets[0]->pt();
    m_higgsVarsMap["resolved_jet2_pt_" + wp] = ptSortedBtaggedJets[1]->pt();
    m_higgsVarsMap["resolved_jet3_pt_" + wp] = ptSortedBtaggedJets[2]->pt();
    m_higgsVarsMap["resolved_jet4_pt_" + wp] = ptSortedBtaggedJets[3]->pt();
    m_higgsVarsMap["resolved_DeltaR12_" + wp] = dRtoLeadingJet_acc(*ptSortedBtaggedJets[1]);
    m_higgsVarsMap["resolved_DeltaR13_" + wp] = dRtoLeadingJet_acc(*ptSortedBtaggedJets[2]);
    m_higgsVarsMap["resolved_DeltaR14_" + wp] = dRtoLeadingJet_acc(*ptSortedBtaggedJets[3]);
    m_higgsVarsMap["resolved_DeltaR23_" + wp] = ROOT::Math::VectorUtil::DeltaR( ptSortedBtaggedJets[1]->jetP4(), ptSortedBtaggedJets[2]->jetP4());
    m_higgsVarsMap["resolved_DeltaR24_" + wp] = ROOT::Math::VectorUtil::DeltaR( ptSortedBtaggedJets[1]->jetP4(), ptSortedBtaggedJets[3]->jetP4());
    m_higgsVarsMap["resolved_DeltaR34_" + wp] = ROOT::Math::VectorUtil::DeltaR( ptSortedBtaggedJets[2]->jetP4(), ptSortedBtaggedJets[3]->jetP4());
    m_higgsVarsMap["resolved_h1_m_" + wp] = h1.m_fourVector.M();
    m_higgsVarsMap["resolved_h1_dR_jets_" + wp] = h1.m_dRjets;
    m_higgsVarsMap["resolved_h2_m_" + wp] = h2.m_fourVector.M();
    m_higgsVarsMap["resolved_h2_dR_jets_" + wp] = h2.m_dRjets;
    m_higgsVarsMap["resolved_hh_m_" + wp] = (h1.m_fourVector + h2.m_fourVector).M();
    // clang-format on

    return;
  };

  void
  DiHiggsAnalysis::makeBoostedAnalysis(const xAOD::JetContainer &largeRjets,
                                       std::string wp)
  {
    // count large jets with some requirements
    int nLargeJets = 0;
    for (const xAOD::Jet *jet : largeRjets)
    {
      if (jet->pt() > 250000. && std::abs(jet->eta() < 2.0))
      {
        nLargeJets += 1;
      }
    }
    m_higgsVarsMap["boosted_nLargeJets_" + wp] = nLargeJets;

    // check if we have at least 2 large R jets
    if (largeRjets.size() < 2)
    {
      return;
    }
    // leadding higgs candidate
    HiggsCandidate h1;
    // subleading higgs candidate
    HiggsCandidate h2;

    // get the leading large R in a shallow copy container
    ConstDataVector<xAOD::JetContainer> ptSortedLargeRJets(
        largeRjets.begin(), largeRjets.end(), SG::VIEW_ELEMENTS);
    // for details on sorting see resolved
    std::partial_sort(
        ptSortedLargeRJets.begin(),     // Iterator from which to start sorting
        ptSortedLargeRJets.begin() + 2, // Use begin + N to sort first N
        ptSortedLargeRJets.end(),       // iterator to end
        [](const xAOD::IParticle *left, const xAOD::IParticle *right)
        { return left->pt() > right->pt(); });

    // cuts from
    // https://cds.cern.ch/record/2708599/files/ATL-COM-PHYS-2020-083.pdf
    // don't use "or" here as the || operator is short-circuited in c++,
    // which means in the OR checking case it stops checking conditions once
    // one becomes true
    if (ptSortedLargeRJets[0]->pt() < 250000. || //
        ptSortedLargeRJets[1]->pt() < 250000. || //
        std::abs(ptSortedLargeRJets[0]->eta()) > 2.0 ||
        std::abs(ptSortedLargeRJets[1]->eta()) > 2.0
        // || ptSortedLargeRJets[0]->m() < 50000.
        // || ptSortedLargeRJets[1]->m() < 50000.
        // || std::abs(ptSortedLargeRJets[0]->eta() -
        //          ptSortedLargeRJets[0]->eta()) > 1.3
    )
    {
      return;
    }

    h1.m_largeRJet = ptSortedLargeRJets[0];
    h2.m_largeRJet = ptSortedLargeRJets[1];

    // @note create needed accessors
    // ghost associated VR track jets are only on the untrimmed 1.0 jets
    static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer>>
        m_acc_largeR_untrimmed("Parent");
    static const SG::AuxElement::ConstAccessor<
        std::vector<ElementLink<xAOD::IParticleContainer>>>
        m_acc_VRTrackJets("GhostAntiKtVR30Rmax4Rmin02PV0TrackJets");
    // accessor for the btagging info on the jet
    static const SG::AuxElement::ConstAccessor<char> isBtag("ftag_select_" +
                                                            wp);
    // recommended by ftag : Remove the event if any of your signal jets have
    // relativeDeltaRToVRJet = radius(jet_i)/min(dR(jet_i,jet_j)) < 1.0.
    // checks if any of the vr jets overlap
    static const SG::AuxElement::ConstAccessor<float> relativeDeltaRToVRJet(
        "relativeDeltaRToVRJet");
    bool isRelativeDeltaRToVRJet = false;

    // loop over the Higgs Candidate large R jets, get the btagged VR jets
    // inside of each, need to do std::ref to access the actual objects h1,
    // h2. "for (HiggsCandidate h : {h1, h2})" would make copies of h1, h2
    for (HiggsCandidate &h : {std::ref(h1), std::ref(h2)})
    {
      // get ghost associated VR track jets from untrimmed large R jet
      const xAOD::Jet *untrimmedLargeR =
          *m_acc_largeR_untrimmed(*h.m_largeRJet);
      std::vector<ElementLink<xAOD::IParticleContainer>> VRTrackjets =
          m_acc_VRTrackJets(*untrimmedLargeR);

      // filter VR jets
      // remove_if is efficient, because you don't have to copy things
      // somewhere else
      int nGhostAssocVrJets = 0;
      VRTrackjets.erase(
          std::remove_if(
              VRTrackjets.begin(), VRTrackjets.end(),
              [&nGhostAssocVrJets,
               &isRelativeDeltaRToVRJet] // need to "capture" external
                                         // variables for the lambda function
              (ElementLink<xAOD::IParticleContainer> & VRjet)
              {
                // count only the ftagged ones
                if ((*VRjet)->pt() > 10000.)
                {
                  nGhostAssocVrJets += 1;
                }
                if (!VRjet.isValid())
                {
                  ANA_MSG_WARNING("Skipped invalid VR jet link!");
                  return true;
                }
                // the ftag sequence does not tag all of them
                if (!isBtag.isAvailable(**VRjet))
                {
                  ANA_MSG_DEBUG("Skipped non B-tagged VR jet");
                  return true;
                }
                // keep only btagged jets with a designated wp
                // (the ftag sequence also cuts on 10GeV)
                if (!isBtag(**VRjet))
                {
                  return true;
                }
                // see accessor definition
                if (relativeDeltaRToVRJet(**VRjet) < 1.0)
                {
                  isRelativeDeltaRToVRJet = true;
                }

                return false;
              }),
          VRTrackjets.end());

      h.m_nGhostAssocVrJets = nGhostAssocVrJets;
      h.m_nBtaggedGhostAssocVrJets = VRTrackjets.size();
      h.m_fourVector = h.m_largeRJet->jetP4();

      // check if we have at least 2 per large R jet and no
      // isRelativeDeltaRToVRJet < 1.0
      if (VRTrackjets.size() < 2 || isRelativeDeltaRToVRJet)
      {
        h.m_doDecorate = false;
      }
      else
      {
        // find the leading vr jets
        std::partial_sort(
            VRTrackjets.begin(),     // Iterator from which to start sorting
            VRTrackjets.begin() + 2, // Use begin + N to sort first N
            VRTrackjets.end(),       // iterator to end
            [](ElementLink<xAOD::IParticleContainer> &left,
               ElementLink<xAOD::IParticleContainer> &right)
            { return (*left)->pt() > (*right)->pt(); });

        h.m_leadingJet = dynamic_cast<const xAOD::Jet *>(*VRTrackjets[0]);
        h.m_subleadingJet = dynamic_cast<const xAOD::Jet *>(*VRTrackjets[1]);

        // calc variables
        h.m_dRjets = ROOT::Math::VectorUtil::DeltaR(
            h.m_leadingJet->jetP4(), h.m_subleadingJet->jetP4());
      }
    }

    // write to map
    if (h1.m_doDecorate)
    {
      m_higgsVarsMap["boosted_h1_jet1_pt_" + wp] = h1.m_leadingJet->pt();
      m_higgsVarsMap["boosted_h1_jet2_pt_" + wp] = h1.m_subleadingJet->pt();
      m_higgsVarsMap["boosted_h1_dR_jets_" + wp] = h1.m_dRjets;
    }
    if (h2.m_doDecorate)
    {
      m_higgsVarsMap["boosted_h2_jet1_pt_" + wp] = h2.m_leadingJet->pt();
      m_higgsVarsMap["boosted_h2_jet2_pt_" + wp] = h2.m_subleadingJet->pt();
      m_higgsVarsMap["boosted_h2_dR_jets_" + wp] = h2.m_dRjets;
    }

    // clang-format off
    m_higgsVarsMap["boosted_h1_nGhostAssocVrJets_" + wp] = h1.m_nGhostAssocVrJets; 
    m_higgsVarsMap["boosted_h1_nBtaggedGhostAssocVrTrackJets_" + wp] = h1.m_nBtaggedGhostAssocVrJets; 
    m_higgsVarsMap["boosted_h2_nGhostAssocVrJets_" + wp] = h2.m_nGhostAssocVrJets; 
    m_higgsVarsMap["boosted_h2_nBtaggedGhostAssocVrTrackJets_" + wp] = h2.m_nBtaggedGhostAssocVrJets;
    m_higgsVarsMap["boosted_h1_m_" + wp] = h1.m_fourVector.M();
    m_higgsVarsMap["boosted_h2_m_" + wp] = h2.m_fourVector.M();
    m_higgsVarsMap["boosted_hh_m_" + wp] = (h1.m_fourVector + h2.m_fourVector).M();
    // clang-format on

    return;
  };

  std::unordered_map<std::string, float> DiHiggsAnalysis::getHiggsVarsMap()
  {
    return m_higgsVarsMap;
  };
};
