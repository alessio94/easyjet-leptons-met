/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SemiLepSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace VBSHIGGS{

  SemiLepSelectorAlg::SemiLepSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator) : EL::AnaAlgorithm(name, pSvcLocator){  }
  StatusCode SemiLepSelectorAlg::initialize(){
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     SemiLepSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

     // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    ATH_CHECK (m_passTriggerSLT.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_signaljetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_HCandHandle.initialize(m_systematicsList));
    ATH_CHECK (m_vbsjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_largejetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_largejetHandle));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_signaljetHandle));
    }

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    }

     // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    if(m_saveCutFlow) ATH_CHECK (initialiseCutflow());
    return StatusCode::SUCCESS;
  }
  StatusCode SemiLepSelectorAlg::execute(){
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      
      const xAOD::JetContainer *signaljets = nullptr;
      ANA_CHECK (m_signaljetHandle.retrieve (signaljets, sys));

      const xAOD::JetContainer *HJets = nullptr;
      ANA_CHECK (m_HCandHandle.retrieve (HJets, sys));

      const xAOD::JetContainer *vbsjets = nullptr;
      ANA_CHECK (m_vbsjetHandle.retrieve (vbsjets, sys));

      const xAOD::JetContainer *largeJets = nullptr;
      ANA_CHECK (m_largejetHandle.retrieve (largeJets, sys));

      bool WPgiven = !m_isBtag.empty();
      std::vector<const xAOD::Jet*> bjets;
      for(const xAOD::Jet* jet : *signaljets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets.push_back(jet);
        }
      }

      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;
      }

      for (auto& [key, value] : m_boolnames) m_bools.at(key) = false;

      if (!m_passTriggerSLT.empty() and m_passTriggerSLT.get(*event, sys)) {
        m_bools.at(VBSHIGGS::PASS_TRIGGER) = true;
      } 
      int nLeptons = electrons->size() + muons->size();
      bool pass_OneLepton = (nLeptons == 1);
      bool pass_preselection =  m_bools.at(VBSHIGGS::PASS_TRIGGER) && pass_OneLepton ;

      m_passallcuts.set(*event, pass_preselection, sys);

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()=="" && m_saveCutFlow){

        // Compute total_events
        m_total_events+=1;

        // Count how many cuts the event passed and increase the relative counter
        for (const auto &cut : m_inputCutKeys) {
          if(m_vbshiggsCuts.exists(m_boolnames.at(cut))) {
            m_vbshiggsCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_vbshiggsCuts(m_boolnames.at(cut)).passed){
              m_vbshiggsCuts(m_boolnames.at(cut)).counter+=1;
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_vbshiggsCuts.size(); ++i) {
          if (m_vbshiggsCuts[i].passed)
            consecutive_cuts++;
          else
            break;
        }

	      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        for (unsigned int i=0; i<consecutive_cuts; i++) {
          m_vbshiggsCuts[i].relativeCounter+=1;
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
  StatusCode SemiLepSelectorAlg::finalize(){
    
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_vbshiggsCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_vbshiggsCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_vbshiggsCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_vbshiggsCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_vbshiggsCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));

    }

    return StatusCode::SUCCESS;
  }


  StatusCode SemiLepSelectorAlg::initialiseCutflow(){
        std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_vbshiggsCuts.CheckInputCutList(m_inputCutList, boolnameslist);

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
      m_vbshiggsCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_vbshiggsCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of vbshiggs cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of vbshiggs cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of vbshiggs cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }

}//name space
