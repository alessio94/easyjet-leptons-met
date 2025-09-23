/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetDecoratorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include "TrigCompositeUtils/ChainNameParser.h"

#include <AthenaKernel/Units.h>


namespace Easyjet
{
  JetDecoratorAlg::JetDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode JetDecoratorAlg::initialize()
  {
    ATH_CHECK(m_jetsInKey.initialize());

    // truth matching
    ATH_CHECK(m_truthJetsInKey.initialize(m_isMC));
    m_truthLabelDecorKey = m_truthJetsInKey.key() + ".HadronConeExclTruthLabelID";
    ATH_CHECK(m_truthLabelDecorKey.initialize(m_isMC));

    m_bJetTruthPtDecorKey = m_jetsInKey.key() + ".bJetTruthPt";
    m_bJetTruthDRDecorKey = m_jetsInKey.key() + ".bJetTruthDR";
    ATH_CHECK(m_bJetTruthPtDecorKey.initialize(m_isMC));
    ATH_CHECK(m_bJetTruthDRDecorKey.initialize(m_isMC));

    // trigger matching
    ATH_CHECK(m_L1JetsInKey.initialize(m_doL1Matching));

    if (!m_triggers.empty())
    {
      ATH_CHECK(m_trigDecTool.retrieve());
      for (auto &trig : m_triggers)
      {
        // convert trigger name to a valid branch name
        std::string modifiedTrigName = trig;
        std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '-', '_');
        std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '.', 'p');

        if (m_doL1Matching) {
          m_jetL1EtDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                               modifiedTrigName + "_L1et");
          m_jetL1EtaDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                                modifiedTrigName + "_L1eta");
          m_jetL1PhiDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                                modifiedTrigName + "_L1phi");
          m_jetL1DRDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                               modifiedTrigName + "_L1dr");
          m_jetL1ThresholdsDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                                    modifiedTrigName + "_L1thresholds");
          ATH_CHECK(m_jetL1EtDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetL1EtaDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetL1PhiDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetL1DRDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetL1ThresholdsDecorKeys.at(trig).initialize());
        }
        if (m_doHLTMatching) {
          m_jetHLTPtDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" + modifiedTrigName + "_HLTpt");
          m_jetHLTEtaDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" + modifiedTrigName + "_HLTeta");
          m_jetHLTPhiDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" + modifiedTrigName + "_HLTphi");
          m_jetHLTDRDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" + modifiedTrigName + "_HLTdr");
          m_jetHLTBtagDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" + modifiedTrigName + "_HLTbtag");
          m_jetHLTThresholdsDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" + modifiedTrigName + "_HLTthresholds");
          ATH_CHECK(m_jetHLTPtDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetHLTEtaDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetHLTPhiDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetHLTDRDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetHLTBtagDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetHLTThresholdsDecorKeys.at(trig).initialize());
        }

      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetDecoratorAlg ::execute(const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetsInKey,ctx);
    ATH_CHECK(jets.isValid());

    if (m_isMC) {
      SG::ReadHandle<xAOD::JetContainer> truthJets(m_truthJetsInKey, ctx);
      ATH_CHECK(truthJets.isValid());

      SG::ReadDecorHandle<xAOD::JetContainer, int> truthLabel(m_truthLabelDecorKey);
      SG::WriteDecorHandle<xAOD::JetContainer, float> bJetTruthPt(m_bJetTruthPtDecorKey);
      SG::WriteDecorHandle<xAOD::JetContainer, float> bJetTruthDR(m_bJetTruthDRDecorKey);

      for(const xAOD::Jet* jet: *jets) {
        float minDR = m_minDR;
        const xAOD::Jet *bestTruth = nullptr;

        for (const xAOD::Jet *truthJet : *truthJets)
        {
          if (truthJet->pt() < m_minTruthPt)
            continue;
          if (truthLabel(*truthJet) != 5)
            continue;

          float dR = jet->p4().DeltaR(truthJet->p4());
          if (dR < minDR)
          {
            bestTruth = truthJet;
            minDR = dR;
          }
        }

        bJetTruthPt(*jet) = bestTruth ? bestTruth->pt() : -99.;
        bJetTruthDR(*jet) = bestTruth ? minDR : -99.;
      }
    }

    std::unordered_map<std::string,
                       SG::WriteDecorHandle<xAOD::JetContainer, float>>
        jetL1Et, jetL1Eta, jetL1Phi, jetL1DR;
    std::unordered_map<std::string, SG::WriteDecorHandle<xAOD::JetContainer, std::vector<int>>> jetL1Thresholds;

    std::unordered_map<std::string,
                       SG::WriteDecorHandle<xAOD::JetContainer, float>>
        jetHLTPt, jetHLTEta, jetHLTPhi, jetHLTDR;
    std::unordered_map<std::string, SG::WriteDecorHandle<xAOD::JetContainer, std::vector<int>>> jetHLTThresholds;
    std::unordered_map<std::string, SG::WriteDecorHandle<xAOD::JetContainer, int>> jetHLTBtag;
    SG::ReadHandle<xAOD::JetRoIContainer> l1Jets;
    for (auto &trig : m_triggers)
    {
      if (m_doL1Matching) {
        l1Jets = SG::makeHandle(m_L1JetsInKey, ctx);
        ATH_CHECK(l1Jets.isValid());
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_et(m_jetL1EtDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_eta(m_jetL1EtaDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_phi(m_jetL1PhiDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_dr(m_jetL1DRDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<int>> wdh_thresholds(m_jetL1ThresholdsDecorKeys.at(trig));
        jetL1Et.emplace(trig, wdh_et);
        jetL1Eta.emplace(trig, wdh_eta);
        jetL1Phi.emplace(trig, wdh_phi);
        jetL1DR.emplace(trig, wdh_dr);
        jetL1Thresholds.emplace(trig, wdh_thresholds);
      }
      if (m_doHLTMatching) {
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_pt(m_jetHLTPtDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_eta(m_jetHLTEtaDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_phi(m_jetHLTPhiDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_dr(m_jetHLTDRDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, int> wdh_btag(m_jetHLTBtagDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<int>> wdh_thresholds(m_jetHLTThresholdsDecorKeys.at(trig));
        jetHLTPt.emplace(trig, wdh_pt);
        jetHLTEta.emplace(trig, wdh_eta);
        jetHLTPhi.emplace(trig, wdh_phi);
        jetHLTDR.emplace(trig, wdh_dr);
        jetHLTThresholds.emplace(trig, wdh_thresholds);
        jetHLTBtag.emplace(trig, wdh_btag);
      }
    }

    std::regex l1NameParser("(\\d*)(J)(\\d*)((p|\\.)(\\d*)ETA(\\d*))?");
    Trig::FeatureRequestDescriptor frd;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::pair<const xAOD::Jet*, bool>>>> emulatedJets = {};
    if (m_doHLTMatching && m_LHCPeriod == 2) // prepare Run2 emulation results
    {
      for (auto &trig : m_triggers)
      {
        emulatedJets[trig] = m_emulationTool->getEmulatedJets(trig);
      }
    }

    for(const xAOD::Jet* jet: *jets) {

      // trigger matching
      for (auto &trig : m_triggers)
      {
        const xAOD::JetRoI* bestL1 = nullptr;
        float minDRL1 = 0.4; // hard-coded matching distance
        std::set<int> L1Thresholds = {};

        const xAOD::IParticle* bestHLT = nullptr;
        float minDRHLT = 0.4; // hard-coded matching distance
        std::set<int> HLTThresholds = {};
        bool btag = false;

        if (m_trigDecTool->isPassed(trig))
        {
          if (m_doL1Matching)
          {
            const TrigConf::HLTChain* hltChain = m_trigDecTool->ExperimentalAndExpertMethods().getChainConfigurationDetails(trig);
            const std::string& l1Name = hltChain->lower_chain_name();
            for (auto l1_jet : *l1Jets)
            {
              TLorentzVector l1_jet_p4;
              l1_jet_p4.SetPtEtaPhiM(l1_jet->et8x8(), l1_jet->eta(), l1_jet->phi(), 0.);
              float dR = jet->p4().DeltaR(l1_jet_p4);
              if (dR < minDRL1)
              {
                minDRL1 = dR;
                bestL1 = l1_jet;
                std::vector<std::string> thrNames = l1_jet->thrNames();
                std::stringstream ss(std::regex_replace(l1Name, std::regex("-"), "_"));
                std::string legName;
                std::smatch match;
                while (getline(ss, legName, '_')) // loop over legs
                {
                  if (std::regex_match(legName, match, l1NameParser))
                  { // regex match
                    std::string legName_noMultiplicity = match[2].str() + match[3].str() + match[4].str();
                    int threshold = match[3].str()=="" ? 1 : std::stoi(match[3].str());
                    for (const auto &thr : thrNames)
                    { // compare with passed thresholds
                      if (thr == legName_noMultiplicity)
                      {
                        L1Thresholds.insert(threshold);
                      }
                    }
                  } // end of regex match
                } // loop over legs
              }
            } // loop over L1 jets
          } // L1 matching

          if (m_doHLTMatching) {
            frd.setChainGroup(trig);

            ATH_MSG_VERBOSE("Trigger: " << trig);
            int ileg = 0;
            for (const ChainNameParser::LegInfo &legInfo :
                 ChainNameParser::HLTChainInfo(trig))
            {
              if (legInfo.signature == "j")
              {
                ATH_MSG_VERBOSE(" Leg" << ileg << ": "
                      << " " << legInfo.legName() << " "
                      << legInfo.type() << " " << legInfo.signature
                      << " " << legInfo.threshold);

                if (m_LHCPeriod == 3) {
                  frd.setRestrictRequestToLeg(ileg);
                  auto hlt_jets = m_trigDecTool->features<xAOD::IParticleContainer>(frd);
                  for (auto hlt_jet_link : hlt_jets)
                  {
                    const xAOD::IParticle *hlt_jet = *hlt_jet_link.link;
                    float dR = jet->p4().DeltaR(hlt_jet->p4());
                    bool hasBtag = hlt_jet_link.source->hasObjectLink("btag");
                    ATH_MSG_VERBOSE("  pt: "
                                  << hlt_jet->pt() << " eta: " << hlt_jet->eta()
                                  << " phi: " << hlt_jet->phi() << " dR: " << dR
                                  << " btag: " << hasBtag);

                    if (bestHLT && isSameJet(bestHLT, hlt_jet))
                    {
                      HLTThresholds.insert(legInfo.threshold);
                      btag = btag || hasBtag; // if any leg claims b-tag, then the jet is b-tagged
                    }
                    else if (dR < minDRHLT)
                    {
                      minDRHLT = dR;
                      bestHLT = hlt_jet;
                      HLTThresholds.clear();
                      HLTThresholds.insert(legInfo.threshold);
                      btag = hasBtag;
                    }
                  }
                }
                else {
                  auto hlt_emulated_jets = emulatedJets[trig][legInfo.legName()]; // use pre-fetched emulation results
                  ATH_MSG_DEBUG(" Emulated jets for " << legInfo.legName() << ": " << hlt_emulated_jets.size());
                  for (const auto& [hlt_jet, passBtag]: hlt_emulated_jets) {
                    float dR = jet->p4().DeltaR(hlt_jet->p4());
                    ATH_MSG_VERBOSE("  pt: " << hlt_jet->pt()
                                             << " eta: " << hlt_jet->eta()
                                             << " phi: " << hlt_jet->phi()
                                             << " dR: " << dR
                                             << " btag: " << passBtag);
                    if (bestHLT && isSameJet(bestHLT, hlt_jet))
                    {
                      HLTThresholds.insert(legInfo.threshold);
                      btag = btag || passBtag; // if any leg claims b-tag, then the jet is b-tagged
                    }
                    else if (dR < minDRHLT)
                    {
                      minDRHLT = dR;
                      bestHLT = hlt_jet;
                      HLTThresholds.clear();
                      HLTThresholds.insert(legInfo.threshold);
                      btag = passBtag;
                    }
                  }
                    
                  ATH_MSG_DEBUG(trig << " isPassed "<<m_emulationTool->isPassed(trig));
                }
              }
              ATH_MSG_VERBOSE(" =dRHLT: " << minDRHLT << " bestHLT pT: "
                                      << (bestHLT ? bestHLT->pt() : -99.)
                                      << " btag: " << btag
                                      << " thresholds: " << std::vector<int>(HLTThresholds.begin(), HLTThresholds.end()));
              ileg++;
            }
          } // HLT matching
        } // if trigger passed

        if (m_doL1Matching)
        {
          // TODO: Only save pT thresholds but not eta thresholds. May not provide enough info in the case of same pT threshold but different eta ranges.
          jetL1Et.at(trig)(*jet) = bestL1 ? bestL1->et8x8() : -99.;
          jetL1Eta.at(trig)(*jet) = bestL1 ? bestL1->eta() : -99.;
          jetL1Phi.at(trig)(*jet) = bestL1 ? bestL1->phi() : -99.;
          jetL1DR.at(trig)(*jet) = minDRL1;
          jetL1Thresholds.at(trig)(*jet) = bestL1 ? std::vector<int>(L1Thresholds.begin(), L1Thresholds.end()) : std::vector<int>();
        }
        if (m_doHLTMatching) {
          // TODO: Only works for trigger chain with a single b-tagging WP. Need to save a vector of b-tagging WPs to handle multiple WPs
          jetHLTPt.at(trig)(*jet) = bestHLT ? bestHLT->pt() : -99.;
          jetHLTEta.at(trig)(*jet) = bestHLT ? bestHLT->eta() : -99.;
          jetHLTPhi.at(trig)(*jet) = bestHLT ? bestHLT->phi() : -99.;
          jetHLTDR.at(trig)(*jet) = minDRHLT;
          jetHLTThresholds.at(trig)(*jet) = bestHLT ? std::vector<int>(HLTThresholds.begin(), HLTThresholds.end()) : std::vector<int>();
          jetHLTBtag.at(trig)(*jet) = bestHLT ? btag : -1;
          ATH_MSG_VERBOSE("Summary " << " Trigger: " << trig << " bestHLT pT: "
                                    << (bestHLT ? bestHLT->pt() : -99.)
                                    << " btag: " << btag
                                    << " thresholds: " << std::vector<int>(HLTThresholds.begin(), HLTThresholds.end()));
        }
      }
    }

    return StatusCode::SUCCESS;
  }

  bool JetDecoratorAlg::isSameJet(const xAOD::IParticle *jet1, const xAOD::IParticle *jet2) const
  {
    // Need this function because jet1 == jet2 would return false when comparing b-jet to untagged jet
    return (jet1->p4().DeltaR(jet2->p4()) < 0.01) && (std::abs(jet1->pt() - jet2->pt()) < 100);
  }
}
