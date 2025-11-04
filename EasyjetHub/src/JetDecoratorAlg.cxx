/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    ATH_CHECK(m_HLTJetsInKey.initialize(m_doHLTMatching));

    if (!m_triggers.empty())
    {
      ATH_CHECK(m_trigDecTool.retrieve());
      for (const auto &trig : m_triggers)
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
          m_jetHLTThresholdsDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" + modifiedTrigName + "_HLTthresholds");
          ATH_CHECK(m_jetHLTPtDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetHLTEtaDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetHLTPhiDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetHLTDRDecorKeys.at(trig).initialize());
          ATH_CHECK(m_jetHLTThresholdsDecorKeys.at(trig).initialize());
        }

      }

      if(m_useEmulationTool) ATH_CHECK(m_emulationTool.retrieve());
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

    SG::ReadHandle<xAOD::JetRoIContainer> l1Jets;
    SG::ReadHandle<xAOD::JetContainer> hltJetsFromCont;
    static const std::unordered_set<std::string> trigger_navigation_bug = {
      "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",
      "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25"
    };
    for (const auto &trig : m_triggers)
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
        hltJetsFromCont = SG::makeHandle(m_HLTJetsInKey, ctx);
        ATH_CHECK(hltJetsFromCont.isValid());
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_pt(m_jetHLTPtDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_eta(m_jetHLTEtaDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_phi(m_jetHLTPhiDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_dr(m_jetHLTDRDecorKeys.at(trig));
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<int>> wdh_thresholds(m_jetHLTThresholdsDecorKeys.at(trig));
        jetHLTPt.emplace(trig, wdh_pt);
        jetHLTEta.emplace(trig, wdh_eta);
        jetHLTPhi.emplace(trig, wdh_phi);
        jetHLTDR.emplace(trig, wdh_dr);
        jetHLTThresholds.emplace(trig, wdh_thresholds);
      }
    }

    std::regex l1NameParser("(\\d*)(J)(\\d*)((p|\\.)(\\d*)ETA(\\d*))?");
    Trig::FeatureRequestDescriptor frd;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::pair<const xAOD::Jet*, bool>>>> emulatedJets = {};
    if (m_doHLTMatching && m_useEmulationTool) // prepare Run2 emulation results
    {
      for (const auto &trig : m_triggers)
      {
        emulatedJets[trig] = m_emulationTool->getEmulatedJets(trig);
      }
    }

    for(const xAOD::Jet* jet: *jets) {

      // trigger matching
      for (const auto &trig : m_triggers)
      {
        const xAOD::JetRoI* bestL1 = nullptr;
        float minDRL1 = 0.4; // hard-coded matching distance
        std::set<int> L1Thresholds = {};

        const xAOD::IParticle* bestHLT = nullptr;
        float minDRHLT = 0.4; // hard-coded matching distance
        std::set<int> HLTThresholds = {};

        if (m_trigDecTool->isPassed(trig))
        {
          if (m_doL1Matching)
          {
            const TrigConf::HLTChain* hltChain = m_trigDecTool->ExperimentalAndExpertMethods().getChainConfigurationDetails(trig);
            const std::string& l1Name = hltChain->lower_chain_name();
            for (const auto l1_jet : *l1Jets)
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

                if (!m_useEmulationTool) {
                  frd.setRestrictRequestToLeg(ileg);

                  auto hlt_jetsFromtrigDec = m_trigDecTool->features<xAOD::IParticleContainer>(frd);

                  std::vector<const xAOD::IParticle*> allHLTJets;

                  for (const auto& hlt_jet_link : hlt_jetsFromtrigDec) {
                    const xAOD::IParticle *hlt_jetFromtrigDec = *hlt_jet_link.link;
                    if (!hlt_jetFromtrigDec) continue;
                    allHLTJets.push_back(hlt_jetFromtrigDec);
                  }
                  // Start adding missing HLT jets -- only for buggy triggers
                  // These extra jets are added because of bug in trigger navigation
                  // Will be removed once bug fixed at DAOD level
                  if (trigger_navigation_bug.contains(trig)) {
                    for (const xAOD::Jet* jetFromCont : *hltJetsFromCont) {
                      bool alreadyIn = false;
                      for (const xAOD::IParticle* seenJet : allHLTJets) {
                        if (isSameJet(seenJet, jetFromCont)) {
                          alreadyIn = true;
                          break;
                        }
                      }
                      if (alreadyIn) continue;
                      allHLTJets.push_back(jetFromCont);
                      ATH_MSG_DEBUG("Added missing HLT jet from container: pt="
                                    << jetFromCont->pt() << " eta=" << jetFromCont->eta()
                                    << " phi=" << jetFromCont->phi());
                    }
                  }
                  for (const xAOD::IParticle* hlt_jet : allHLTJets) {
                    float dR = jet->p4().DeltaR(hlt_jet->p4());

                    bool fromtrigDec = false;
                    for (const auto& hlt_jet_link : hlt_jetsFromtrigDec) {
                      if (*hlt_jet_link.link == hlt_jet) {
                        fromtrigDec = true;
                        break;
                      }
                    }

                    ATH_MSG_VERBOSE("  pt: "
                                  << hlt_jet->pt() << " eta: " << hlt_jet->eta()
                                  << " phi: " << hlt_jet->phi() << " dR: " << dR
                                  << " (fromContainer=" << !fromtrigDec << ")");

                    if (bestHLT && isSameJet(bestHLT, hlt_jet))
                    {
                      HLTThresholds.insert(legInfo.threshold);
                    }
                    else if (dR < minDRHLT)
                    {
                      minDRHLT = dR;
                      bestHLT = hlt_jet;
                      HLTThresholds.clear();
                      HLTThresholds.insert(legInfo.threshold);
                    }
                  }
                }

		// if m_useEmulationTool
                else {
                  auto hlt_emulated_jets = emulatedJets[trig][legInfo.legName()]; // use pre-fetched emulation results
                  ATH_MSG_DEBUG(" Emulated jets for " << legInfo.legName() << ": " << hlt_emulated_jets.size());

                  for (const auto& [hlt_jet, passBtag]: hlt_emulated_jets) {
                    float dR = jet->p4().DeltaR(hlt_jet->p4());
                    ATH_MSG_VERBOSE("  pt: " << hlt_jet->pt()
                                             << " eta: " << hlt_jet->eta()
                                             << " phi: " << hlt_jet->phi()
                                             << " dR: " << dR);

                    if (bestHLT && isSameJet(bestHLT, hlt_jet))
                    {
                      HLTThresholds.insert(legInfo.threshold);
                    }
                    else if (dR < minDRHLT)
                    {
                      minDRHLT = dR;
                      bestHLT = hlt_jet;
                      HLTThresholds.clear();
                      HLTThresholds.insert(legInfo.threshold);
                    }
                  }
                    
                  ATH_MSG_DEBUG(trig << " isPassed "<<m_emulationTool->isPassed(trig));
                }
              }

              ATH_MSG_VERBOSE(" =dRHLT: " << minDRHLT << " bestHLT pT: "
                                      << (bestHLT ? bestHLT->pt() : -99.)
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
          jetL1Thresholds.at(trig)(*jet) = bestL1 ?
	    std::vector<int>(L1Thresholds.begin(), L1Thresholds.end()) :
	    std::vector<int>();
        }

        if (m_doHLTMatching) {
          // TODO: Only works for trigger chain with a single b-tagging WP. Need to save a vector of b-tagging WPs to handle multiple WPs
          jetHLTPt.at(trig)(*jet) = bestHLT ? bestHLT->pt() : -99.;
          jetHLTEta.at(trig)(*jet) = bestHLT ? bestHLT->eta() : -99.;
          jetHLTPhi.at(trig)(*jet) = bestHLT ? bestHLT->phi() : -99.;
          jetHLTDR.at(trig)(*jet) = minDRHLT;
          jetHLTThresholds.at(trig)(*jet) = bestHLT ?
	    std::vector<int>(HLTThresholds.begin(), HLTThresholds.end()) :
	    std::vector<int>();

          ATH_MSG_VERBOSE("Summary " << " Trigger: " << trig << " bestHLT pT: "
                                    << (bestHLT ? bestHLT->pt() : -99.)
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
