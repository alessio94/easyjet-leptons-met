/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SmallRJetTriggerSFAlg.h"
#include "TrigCompositeUtils/ChainNameParser.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include "PathResolver/PathResolver.h"
#include <TFile.h>

namespace HH4B
{
  SmallRJetTriggerSFAlg::SmallRJetTriggerSFAlg(const std::string &name,
                                                   ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode SmallRJetTriggerSFAlg::initialize()
  {
    ATH_CHECK(m_inJetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK(m_outJetHandle.initialize(m_systematicsList));

    if (m_matchingLevel == "L1") m_matchingLevelEnum = TrigMatchingLevel::L1;
    else if (m_matchingLevel == "HLT") m_matchingLevelEnum = TrigMatchingLevel::HLT;
    else
    {
      ATH_MSG_ERROR("Invalid matching level " << m_matchingLevel << ". Must be L1 or HLT.");
      return StatusCode::FAILURE;
    }

    // Load calibration file by year
    if (m_years.empty()) {
      ATH_MSG_ERROR("Year is not set");
      return StatusCode::FAILURE;
    }
    int year = m_years.size() == 1 ? m_years[0] : 2016; // in case of mc20a which corresponds to 2015+2016
    std::string path;
    if (year >= 2022) path = "EasyjetHub/jet_trigger_scale_factors_2223.root"; // Run 3 calibration file
    else path = "EasyjetHub/jet_trigger_scale_factors_run2.root"; // Run 2 calibration file
    std::string resolvedPath = PathResolverFindCalibFile(path);
    if (resolvedPath=="") ATH_MSG_WARNING("Failed to load calibration file " << path << ". " + m_matchingLevel + " jet scale factor set to 1.");
    TFile* jetSFFile = new TFile(resolvedPath.c_str(), "READ");

    // get trigger leg thresholds from trigger name. Build the matching pattern and the SF map.
    std::regex l1NameParser("(\\d*)(J)(\\d*)((p|\\.)(\\d*)ETA(\\d*))?");
    for (auto &trig : m_triggers)
    {
      std::vector<int> thresholds;
      if (m_matchingLevelEnum == TrigMatchingLevel::L1)
      {
        std::string l1Name = ChainNameParser::HLTChainInfo(trig).l1Item();
        std::stringstream ss(l1Name);
        std::string legName;
        std::smatch match;
        while (getline(ss, legName, '_'))
        {
          if (std::regex_match(legName, match, l1NameParser))
          {
            std::string threshold = match[3].str();
            unsigned int multiplicity = match[1].str()=="" ? 1 : std::stoi(match[1].str());
            std::string sfName = m_matchingLevel+"_J"+threshold;
            std::string histName = std::to_string(year)+"/"+sfName+"/"+sfName;
            TH2D *h(dynamic_cast<TH2D *>(jetSFFile->Get((histName+"_scale_factors").c_str())));
            if (h)
            {
              m_jetTriggerSFMap.emplace(std::stoi(threshold), h);
              TH2D *h_stats_unc(dynamic_cast<TH2D *>(jetSFFile->Get((histName+"_stats_abs_uncertainty").c_str())));
              TH2D *h_syst_unc(dynamic_cast<TH2D *>(jetSFFile->Get((histName+"_systematic_abs_uncertainty").c_str())));
              if (h_stats_unc) {m_jetTriggerSFStatsMap.emplace(std::stoi(threshold), h_stats_unc);}
              if (h_syst_unc) {m_jetTriggerSFSystMap.emplace(std::stoi(threshold), h_syst_unc);}
            }
            else { ATH_MSG_WARNING("No trigger jet scale factor for L1 threshold J" << threshold); }
            for (unsigned int i=0; i< multiplicity; i++) // flattern the multiplicity
            {
              thresholds.push_back(std::stoi(threshold));
            }
          }
        }
      }
      if (m_matchingLevelEnum == TrigMatchingLevel::HLT)
      {
        for (const ChainNameParser::LegInfo &legInfo :
             ChainNameParser::HLTChainInfo(trig))
        {
          if (legInfo.signature == "j")
          {
            if (jetSFFile && m_jetTriggerSFMap.find(legInfo.threshold) == m_jetTriggerSFMap.end())
            {
              std::string sfName = m_matchingLevel+"_j"+std::to_string(legInfo.threshold);
              std::string histName = std::to_string(year)+"/"+sfName+"/"+sfName;
              TH2D* h(dynamic_cast<TH2D *>(jetSFFile->Get((histName+"_scale_factors").c_str())));
              if (h)
              {
                m_jetTriggerSFMap.emplace(legInfo.threshold, h);
                TH2D* h_stats_unc(dynamic_cast<TH2D *>(jetSFFile->Get((histName+"_stats_abs_uncertainty").c_str())));
                TH2D* h_syst_unc(dynamic_cast<TH2D *>(jetSFFile->Get((histName+"_systematic_abs_uncertainty").c_str())));
                if (h_stats_unc) {m_jetTriggerSFStatsMap.emplace(legInfo.threshold, h_stats_unc);}
                if (h_syst_unc) {m_jetTriggerSFSystMap.emplace(legInfo.threshold, h_syst_unc);}
              }
              else { ATH_MSG_WARNING("No trigger jet scale factor for HLT threshold j" << legInfo.threshold); }
            }
            for (unsigned int i = 0; i < legInfo.multiplicity; i++) // flattern the multiplicity
            {
              thresholds.push_back(legInfo.threshold);
            }
            if (legInfo.legName().find("SHARED") != std::string::npos)
            { // the next leg is a SHARED leg because SHARED is parsed as part of
              // the previous leg name. Break here to ignore the shared leg.
              break;
            }
          }
        }
      }
      std::ranges::sort(thresholds, std::greater<>()); // sort the thresholds into decending order
      m_triggerLegThresholds.emplace(trig, thresholds);

      // convert trigger name to a valid branch name
      std::string modifiedTrigName = trig;
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '-', '_');
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '.', 'p');

      // initialize the read handle to read a list of matched thresholds
      m_ThresholdsDecorKey.emplace(
            trig, m_inJetHandle.getNamePattern() + ".match" +
                      modifiedTrigName + "_" + m_matchingLevel + "thresholds");
      ATH_CHECK(m_ThresholdsDecorKey.at(trig).initialize());

      // initialize write handle to write a single matched threshold after event-level ambiguity resolution
      m_Threshold.emplace(
          trig,
          CP::SysWriteDecorHandle<int>(
              this, modifiedTrigName + m_matchingLevel + "threshold",
              modifiedTrigName + "_" + m_matchingLevel + "threshold_%SYS%",
                    "Trigger threshold for applying trigger SF"));
      ATH_CHECK(m_Threshold.at(trig).initialize(m_systematicsList, m_outJetHandle));

      // initialize read handle to read back the single matched threshold
      m_Thresholdread.emplace(
          trig, CP::SysReadDecorHandle<int>(modifiedTrigName + "_" + m_matchingLevel + "threshold_%SYS%", this));
      ATH_CHECK(m_Thresholdread.at(trig).initialize(m_systematicsList, m_inJetHandle));

      // initialize SF write handles
      m_jetSF.emplace(
          trig, CP::SysWriteDecorHandle<float>(
                    this, modifiedTrigName + m_matchingLevel + "SF",
                    modifiedTrigName + "_" + m_matchingLevel + "SF_%SYS%",
                    "jet-level jet trigger SF"));
      m_jetSFStatsUp.emplace(
          trig,
          CP::SysWriteDecorHandle<float>(
              this, modifiedTrigName + m_matchingLevel + "SFStatsUp",
              modifiedTrigName + "_" + m_matchingLevel + "SF_stats__1up",
              "jet-level jet trigger SF stats uncertainty up"));
      m_jetSFSystUp.emplace(
          trig,
          CP::SysWriteDecorHandle<float>(
              this, modifiedTrigName + m_matchingLevel + "SFSystUp",
              modifiedTrigName + "_" + m_matchingLevel + "SF_syst__1up",
              "jet-level jet trigger SF syst uncertainty up"));
      ATH_CHECK(m_jetSF.at(trig).initialize(m_systematicsList, m_outJetHandle));
      ATH_CHECK(m_jetSFStatsUp.at(trig).initialize(m_systematicsList, m_outJetHandle));
      ATH_CHECK(m_jetSFSystUp.at(trig).initialize(m_systematicsList, m_outJetHandle));

      m_eventSFKey.emplace(trig, m_eventHandle.getNamePattern() + ".trigSF_" + modifiedTrigName + "_" + m_matchingLevel + "SF");
      m_eventSFStatsUpKey.emplace(trig, m_eventHandle.getNamePattern() + ".trigSF_" + modifiedTrigName + "_" + m_matchingLevel + "SF_stats__1up");
      m_eventSFSystUpKey.emplace(trig, m_eventHandle.getNamePattern() + ".trigSF_" + modifiedTrigName + "_" + m_matchingLevel + "SF_syst__1up");
      ATH_CHECK(m_eventSFKey.at(trig).initialize());
      ATH_CHECK(m_eventSFStatsUpKey.at(trig).initialize());
      ATH_CHECK(m_eventSFSystUpKey.at(trig).initialize());
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode SmallRJetTriggerSFAlg::execute()
  {
    std::unordered_map< std::string, SG::ReadDecorHandle<xAOD::JetContainer, std::vector<int>>> jetThresholds;
    std::unordered_map<std::string, SG::WriteDecorHandle<xAOD::EventInfo, float>> eventSF, eventSFStatsUp, eventSFSystUp;
    for (auto &trig : m_triggers)
    {
      jetThresholds.emplace(trig, m_ThresholdsDecorKey.at(trig));
      SG::WriteDecorHandle<xAOD::EventInfo, float> wdh_eventSF(m_eventSFKey.at(trig));
      SG::WriteDecorHandle<xAOD::EventInfo, float> wdh_eventSFStatsUp(m_eventSFStatsUpKey.at(trig));
      SG::WriteDecorHandle<xAOD::EventInfo, float> wdh_eventSFSystUp(m_eventSFSystUpKey.at(trig));
      eventSF.emplace(trig, wdh_eventSF);
      eventSFStatsUp.emplace(trig, wdh_eventSFStatsUp);
      eventSFSystUp.emplace(trig, wdh_eventSFSystUp);
    }

    // Loop over all systs
    for (const auto &sys : m_systematicsList.systematicsVector())
    {
      // Retrieve inputs
      const xAOD::JetContainer *inJets = nullptr;
      ATH_CHECK(m_inJetHandle.retrieve(inJets, sys));
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // fill workContainer with "views" of the inContainer and initialize SF and Threshold
      // only fill the workContainer with jets that matched to any trigger
      auto workContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(SG::VIEW_ELEMENTS);
      for (const xAOD::Jet *jet : *inJets)
      {
        bool isTriggerMatched = false;
        for (auto &trig : m_triggers)
        {
          m_jetSF.at(trig).set(*jet, 1, sys);
          m_jetSFStatsUp.at(trig).set(*jet, 1, sys);
          m_jetSFSystUp.at(trig).set(*jet, 1, sys);
          if (jetThresholds.at(trig)(*jet).empty())
            m_Threshold.at(trig).set(*jet, -99, sys);
          else {
            m_Threshold.at(trig).set(*jet, 0, sys);
            isTriggerMatched = true;
          }
        }
        if (isTriggerMatched) workContainer->push_back(jet);
      }

      // sort the trigger matched jets by pT before b-jet energy correction
      std::sort(workContainer->begin(), workContainer->end(),
              [](const xAOD::Jet *jet1, const xAOD::Jet *jet2) {
                return jet1->jetP4("NoBJetCalibMomentum").Pt() > jet2->jetP4("NoBJetCalibMomentum").Pt();
              });

      // resolve matching ambituity and assign the SF according to the resolved threshold
      for (auto &trig : m_triggers)
      {
        float tmpEventSF = 1.0; // multiply all jet-level  SFs to get event-level  SF
        float tmpEventSFStatsUp = 1.0;
        float tmpEventSFSystUp = 1.0;

        // L1 matching can have multiple thresholds even after ambiguity resolution
        // create a map to keep track of used thresholds to avoid double-matching
        std::vector<std::vector<int>> assignedL1Thresholds(workContainer->size(), std::vector<int>());
        for (auto legThreshold : m_triggerLegThresholds.at(trig))
        {
          for (size_t ijet = 0; ijet < workContainer->size(); ijet++)
          {
            const xAOD::Jet *jet = workContainer->at(ijet);

            // In L1 matching, if the jet has been assigned a threshold same as the current leg threshold,
            // continue to try the next jet to meet the multiplicity requirement.
            // If the assigned threshold is different from the current leg threshold, the jet can be resued in matching
            // due to no overlap removal between L1 legs.
            if (m_matchingLevelEnum == TrigMatchingLevel::L1 &&
                std::find(assignedL1Thresholds.at(ijet).begin(),
                          assignedL1Thresholds.at(ijet).end(),
                          legThreshold) != assignedL1Thresholds.at(ijet).end())
              continue;

            // In HLT matching, if the jet has been assigned a threshold, continue to try the next jet
            if (m_matchingLevelEnum == TrigMatchingLevel::HLT && m_Thresholdread.at(trig).get(*jet, sys) > 0) continue;

            // In other cases, do the matching: Check if the jet is matched to the current leg threshold
            //   Yes - Matching is done. Break and continue to the next leg threshold.
            //   No - Continue to try the next jet for the same leg threshold.
            if ( std::find(jetThresholds.at(trig)(*jet).begin(),
                        jetThresholds.at(trig)(*jet).end(),
                        legThreshold) !=
                  jetThresholds.at(trig)(*jet).end())
            {
              // in L1 matching, if the jet has been assigned a threshold,
              // consider the matching successful but keep the previously matched threshold and SF.
              // Otherwise it is the first match, proceed to assign the threshold and SF.
              if (m_matchingLevelEnum == TrigMatchingLevel::L1)
              {
                assignedL1Thresholds.at(ijet).push_back(legThreshold);
                if (assignedL1Thresholds.at(ijet).size() > 1) break;
              }

              m_Threshold.at(trig).set(*jet, legThreshold, sys);

              float jetSF = 1.0;
              float jetSFStatsUp = 1.0;
              float jetSFSystUp = 1.0;
              float jetPt = jet->jetP4("NoBJetCalibMomentum").Pt() * 0.001;
              float jetEta = jet->jetP4("NoBJetCalibMomentum").Eta();

              // calibration valid range
              float maxPt = 300.;
	      float minPt = 20.;
	      for (const auto& year : m_years) {
	      	if (year < 2022) minPt = 40.;
	      }
	      float maxEta = 2.4;
              if (m_matchingLevelEnum == TrigMatchingLevel::L1) 
              {
                if (legThreshold == 45) maxEta = 2.1;
		else if (legThreshold == 15 || legThreshold == 20 || legThreshold == 30 || legThreshold == 75 || legThreshold == 85) maxEta = 2.5;
              }

              // if SF exists and jet in the valid kinematic region, assign jet-level SF as a function of NoBJetCalibMomentum pt, eta and Threshold.
              if (jetPt>=minPt && std::abs(jetEta)<maxEta && m_jetTriggerSFMap.find(legThreshold) != m_jetTriggerSFMap.end())
              {
                int bin = m_jetTriggerSFMap.at(legThreshold)->FindBin(jetPt<maxPt ? jetPt : maxPt-0.1, jetEta);
                jetSF = m_jetTriggerSFMap.at(legThreshold)->GetBinContent(bin);
                if (jetSF == 0)
                {
                  ATH_MSG_WARNING("Jet " + m_matchingLevel + " SF j" << legThreshold
                                  << " is zero for jet with pt=" << jetPt
                                  << " GeV and eta=" << jet->jetP4("NoBJetCalibMomentum").Eta()
                                  << ". Setting SF to 1.");
                  jetSF = 1.;
                }
                else {
                  jetSFStatsUp = jetSF + m_jetTriggerSFStatsMap.at(legThreshold)->GetBinContent(bin);
                  jetSFSystUp = jetSF + m_jetTriggerSFSystMap.at(legThreshold)->GetBinContent(bin);
                }
              }

              m_jetSF.at(trig).set(*jet, jetSF, sys);
              m_jetSFStatsUp.at(trig).set(*jet, jetSFStatsUp, sys);
              m_jetSFSystUp.at(trig).set(*jet, jetSFSystUp, sys);
              tmpEventSF *= jetSF;
              tmpEventSFStatsUp *= jetSFStatsUp;
              tmpEventSFSystUp *= jetSFSystUp;

              break; // only label one offline jet for a given leg threshold
            }
          }
        }

        eventSF.at(trig)(*event) = tmpEventSF;
        eventSFStatsUp.at(trig)(*event) = tmpEventSFStatsUp;
        eventSFSystUp.at(trig)(*event) = tmpEventSFSystUp;
      }

      // Write to eventstore
      ATH_CHECK(m_outJetHandle.record(std::move(workContainer), sys));
    }

    return StatusCode::SUCCESS;
  }
}
