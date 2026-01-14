/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBLLANALYSIS_BJETSVARSBBLLALG
#define HHBBLLANALYSIS_BJETSVARSBBLLALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace HHBBLL
{

  /// \brief An algorithm for counting containers
  class BJetsVarsbbllAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    BJetsVarsbbllAlg(const std::string &name, ISvcLocator *pSvcLocator);
    
    StatusCode initialize() override;
    StatusCode execute() override;
  private:
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_bbllJetHandle{ this, "bbllJets", "bbllAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
      m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
    Gaudi::Property<std::vector<std::string>> m_PCBTnames
      {this, "PCBTDecorList", {}, "Name list of pseudo-continuous b-tagging decorator"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<int>> m_PCBTs;
    CP::SysReadDecorHandle<int> m_truthFlav{"HadronConeExclTruthLabelID", this};

    CP::SysReadDecorHandle<int> m_nmuons{"n_muons_%SYS%", this};

    Gaudi::Property<bool> m_doSystematics
      { this, "doSystematics", false, "Run on all systematics" };

    Gaudi::Property<bool> m_save_extra_vars
      { this, "save_extra_vars", false, "Compute quantities which may be useful." };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
        m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    
 };
}
#endif



