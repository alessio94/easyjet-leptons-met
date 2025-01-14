/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JJSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace VBSVV4q{

  JJSelectorAlg::JJSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator) : EL::AnaAlgorithm(name, pSvcLocator){  }

  StatusCode JJSelectorAlg::initialize(){

    ATH_MSG_INFO("*********************\n");
    ATH_MSG_INFO("     JJSelectorAlg   \n");
    ATH_MSG_INFO("*********************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    ATH_CHECK (m_passTriggerSJT.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_vbsjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_LargeRJetsHandle.initialize(m_systematicsList));
    ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_LargeRJetsHandle));
    ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_LargeRJetsHandle));
    ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_LargeRJetsHandle));
    ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_LargeRJetsHandle));

    //if (!m_isBtag.empty()) {
    //  ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_signaljetHandle));
    //}

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));    
    ATH_CHECK (m_year.initialize(m_systematicsList, m_eventHandle));

    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    }
   
    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_VBSVV4qCuts.CheckInputCutList(m_inputCutList, boolnameslist);

    m_inputCutKeys.resize(m_inputCutList.size());
    std::vector<bool> inputWasFound (m_inputCutList.size(), false);
    for (const auto& [key, value]: m_boolnames) {
      auto it = std::find(m_inputCutList.begin(), m_inputCutList.end(), value);
      if (it != m_inputCutList.end()) {
        auto index = it - m_inputCutList.begin();
        m_inputCutKeys.at(index) = key;
        inputWasFound.at(index) = true;
      }
    }
    for (unsigned int index = 0; index < inputWasFound.size(); index++) {
      if(inputWasFound.at(index)) continue;
      ATH_MSG_ERROR("CutLists don't match. Please double check your configuration " + m_inputCutList[index]);
    }

    for (const auto &cut : m_inputCutKeys) {
      m_VBSVV4qCuts.add(m_boolnames[cut]);
    }

    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());
    
    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_VBSVV4qCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of VBSVV4q cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of VBSVV4q cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of VBSVV4q cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }
  
  StatusCode JJSelectorAlg::execute(){
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));
  
      const xAOD::JetContainer *vbsjets = nullptr;
      ANA_CHECK (m_vbsjetHandle.retrieve (vbsjets, sys));

      const xAOD::JetContainer *largeJets = nullptr;
      ANA_CHECK (m_LargeRJetsHandle.retrieve (largeJets, sys));

      for (auto& [key, value] : m_boolnames) m_bools.at(key) = false;

      if ( largeJets->size() >= 1 )  m_bools.at(VBSVV4q::PASS_ONE_LARGE_JET) = true;
      
      if (!m_passTriggerSJT.empty() and m_passTriggerSJT.get(*event, sys)) {
        m_bools.at(VBSVV4q::PASS_TRIGGER) = true;
      } 
      
      vbsjetsSelection(vbsjets);

      bool pass_preselection = true;
      //bool pass_preselection = m_bools.at(VBSVV4q::PASS_RES_BASELINE) || m_bools.at(VBSVV4q::PASS_MERG_BASELINE);
      m_passallcuts.set(*event, pass_preselection, sys);

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()==""){
      
        // Compute total_events
        m_total_events+=1;

        // Count how many cuts the event passed and increase the relative counter
        for (const auto &cut : m_inputCutKeys) {
          if(m_VBSVV4qCuts.exists(m_boolnames.at(cut))) {
            m_VBSVV4qCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_VBSVV4qCuts(m_boolnames.at(cut)).passed){
              m_VBSVV4qCuts(m_boolnames.at(cut)).counter+=1;
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_VBSVV4qCuts.size(); ++i) {
          if (m_VBSVV4qCuts[i].passed)
            consecutive_cuts++;
          else
            break;
        }

        // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        for (unsigned int i=0; i<consecutive_cuts; i++) {
          m_VBSVV4qCuts[i].relativeCounter+=1;
        }
      }

      // Fill syst-aware output decorators
      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      }

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      if (!m_bypass && !pass_preselection) continue;
      filter.setPassed(true);
    }
    return StatusCode::SUCCESS;
  }

  StatusCode JJSelectorAlg::finalize(){

    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_VBSVV4qCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_VBSVV4qCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_VBSVV4qCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_VBSVV4qCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_VBSVV4qCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));

    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }
    return StatusCode::SUCCESS;
  }

  void JJSelectorAlg :: vbsjetsSelection(const xAOD::JetContainer * vbsjets){
    
    if (vbsjets->size() >= 2){
      //TODO, create an object holding the vbs jets passing the mjj + deta selections
      const xAOD::Jet* vbsJet1 = vbsjets->at(0);
      const xAOD::Jet* vbsJet2 = vbsjets->at(1);;
      double mjj = (vbsJet1->p4() + vbsJet2->p4()).M();
      double dEta_jj = std::abs(vbsJet1->eta() - vbsJet2->eta());

      if ( mjj > 300 * Athena::Units::GeV  && dEta_jj > 3.0 ) {
        //m_bools.at(VBSVV4q::PASS_VBS_BASELINE) = true;
      }
    }

  }//vbsjetsSelection

}//name space
