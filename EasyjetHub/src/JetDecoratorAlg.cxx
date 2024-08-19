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
    ATH_CHECK(m_truthJetsInKey.initialize());

    m_truthLabelDecorKey = m_truthJetsInKey.key() + ".HadronConeExclTruthLabelID";
    ATH_CHECK(m_truthLabelDecorKey.initialize());
    
    m_bJetTruthPtDecorKey = m_jetsInKey.key() + ".bJetTruthPt";
    m_bJetTruthDRDecorKey = m_jetsInKey.key() + ".bJetTruthDR";
    ATH_CHECK(m_bJetTruthPtDecorKey.initialize());
    ATH_CHECK(m_bJetTruthDRDecorKey.initialize());

    // for trigger matching
    if (!m_triggers.empty())
    {
      ATH_CHECK(m_trigDecTool.retrieve());
      for (auto &trig : m_triggers)
      {
        // convert trigger name to a valid branch name
        std::string modifiedTrigName = trig;
        std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '-',
                     '_');
        std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '.',
                     'p');

        m_jetHLTPtDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                              modifiedTrigName + "_pt");
        m_jetHLTEtaDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                               modifiedTrigName + "_eta");
        m_jetHLTPhiDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                               modifiedTrigName + "_phi");
        m_jetHLTDRDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                              modifiedTrigName + "_dr");
        m_jetHLTBtagDecorKeys.emplace(trig, m_jetsInKey.key() + ".match" +
                                                modifiedTrigName + "_btag");
        m_jetHLTThresholdsDecorKeys.emplace(
            trig, m_jetsInKey.key() + ".match" + modifiedTrigName + "_thresholds");
        ATH_CHECK(m_jetHLTPtDecorKeys.at(trig).initialize());
        ATH_CHECK(m_jetHLTEtaDecorKeys.at(trig).initialize());
        ATH_CHECK(m_jetHLTPhiDecorKeys.at(trig).initialize());
        ATH_CHECK(m_jetHLTDRDecorKeys.at(trig).initialize());
        ATH_CHECK(m_jetHLTBtagDecorKeys.at(trig).initialize());
        ATH_CHECK(m_jetHLTThresholdsDecorKeys.at(trig).initialize());

      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetDecoratorAlg ::execute(const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetsInKey,ctx);
    ATH_CHECK(jets.isValid());

    SG::ReadHandle<xAOD::JetContainer> truthJets(m_truthJetsInKey,ctx);
    ATH_CHECK(truthJets.isValid());

    SG::ReadDecorHandle<xAOD::JetContainer, int> truthLabel(m_truthLabelDecorKey);
    SG::WriteDecorHandle<xAOD::JetContainer, float> bJetTruthPt(m_bJetTruthPtDecorKey);
    SG::WriteDecorHandle<xAOD::JetContainer, float> bJetTruthDR(m_bJetTruthDRDecorKey);

    std::unordered_map<std::string,
                       SG::WriteDecorHandle<xAOD::JetContainer, float>>
        jetHLTPt, jetHLTEta, jetHLTPhi, jetHLTDR;
    std::unordered_map<std::string, SG::WriteDecorHandle<xAOD::JetContainer, std::vector<int>>> jetHLTThresholds;
    std::unordered_map<std::string, SG::WriteDecorHandle<xAOD::JetContainer, int>> jetHLTBtag;
    for (auto &trig : m_triggers)
    {
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
    Trig::FeatureRequestDescriptor frd;
    
    for(const xAOD::Jet* jet: *jets) {
      float minDR = m_minDR;
      const xAOD::Jet* bestTruth = nullptr;

      for(const xAOD::Jet* truthJet: *truthJets){
	if(truthJet->pt()<m_minTruthPt) continue;
	if(truthLabel(*truthJet)!=5) continue;

	float dR = jet->p4().DeltaR(truthJet->p4());
	if(dR < minDR){
	  bestTruth = truthJet;
	  minDR = dR;
	}
      }

      bJetTruthPt(*jet) = bestTruth ? bestTruth->pt() : -99.;
      bJetTruthDR(*jet) = bestTruth ? minDR : -99.;

      // trigger matching
      for (auto &trig : m_triggers)
      {
        const xAOD::IParticle* bestHLT = nullptr;
        float minDRHLT = 0.4; // hard-coded matching distance
        std::set<int> threshold = {};
        bool btag = false;
        if (m_trigDecTool->isPassed(trig))
        {
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
                  threshold.insert(legInfo.threshold);
                  btag = btag || hasBtag; // if any leg claims b-tag, then the jet is b-tagged
                }
                else if (dR < minDRHLT)
                {
                  minDRHLT = dR;
                  bestHLT = hlt_jet;
                  threshold.clear();
                  threshold.insert(legInfo.threshold);
                  btag = hasBtag;
                }
              }
            }
            ATH_MSG_VERBOSE(" =dRHLT: " << minDRHLT << " bestHLT pT: "
                                    << (bestHLT ? bestHLT->pt() : -99.)
                                    << " btag: "
                                    << btag
                                    << " thresholds: " << std::vector<int>(threshold.begin(), threshold.end()));
            ileg++;
          }

        }
        jetHLTPt.at(trig)(*jet) = bestHLT ? bestHLT->pt() : -99.;
        jetHLTEta.at(trig)(*jet) = bestHLT ? bestHLT->eta() : -99.;
        jetHLTPhi.at(trig)(*jet) = bestHLT ? bestHLT->phi() : -99.;
        jetHLTDR.at(trig)(*jet) = minDRHLT;
        jetHLTThresholds.at(trig)(*jet) = bestHLT ? std::vector<int>(threshold.begin(), threshold.end()) : std::vector<int>();
        jetHLTBtag.at(trig)(*jet) = bestHLT ? btag : -1; // Only works for trigger chain with a single b-tagging WP. Not sure how to handle multiple WPs.
        ATH_MSG_VERBOSE("Summary " << " Trigger: " << trig << " bestHLT pT: "
                                  << (bestHLT ? bestHLT->pt() : -99.)
                                  << " btag: "
                                  << btag
                                  << " thresholds: " << std::vector<int>(threshold.begin(), threshold.end()));
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
