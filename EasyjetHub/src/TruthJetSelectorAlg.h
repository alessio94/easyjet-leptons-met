/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef EASYJET_TRUTHJETSELECTORALG
#define EASYJET_TRUTHJETSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>


#include <AthContainers/ConstDataVector.h>
#include <xAODTruth/TruthEvent.h>
#include <xAODTruth/TruthEventContainer.h>
#include <xAODJet/JetContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class TruthJetSelectorAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    TruthJetSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    SG::ReadHandleKey<xAOD::TruthEventContainer>
      m_eventHandleKey{ this, "event", "TruthEvents",   "EventInfo container to read" };

    SG::ReadHandleKey<xAOD::JetContainer>
      m_inHandleKey{ this, "containerInKey", "",   "Jet container to read" };

    /// \brief Setup output container handles
    SG::WriteHandleKey<ConstDataVector<xAOD::JetContainer>>
      m_outHandleKey{ this, "containerOutKey", "",   "Jet container to write" };

    /// \brief Setup output decorations
    SG::WriteDecorHandleKey<xAOD::TruthEventContainer> m_nSelPartKey {this, "decorOutName", "nJets", 
        "Name of output decorator for number of selected jets"};

    SG::WriteDecorHandleKey<xAOD::JetContainer> m_isSelectedJetKey {this, "decoration", "isTruthJet", 
        "decoration for per-object if jet is selected"};

    SG::ReadDecorHandleKey<xAOD::JetContainer> m_truthLabelDecorKey{this, "truthLabelDecorName", "HadronConeExclTruthLabelID", "Name of input decorator for truth label"};

    Gaudi::Property<float> m_minPt            {this, "minPt", 25e3, "Minimum pT of jets"};
    Gaudi::Property<float> m_maxPt            {this, "maxPt", -1, "Maximum pT of jets"};
    Gaudi::Property<float> m_maxEta           {this, "maxEta", 4.5, "Maximum eta of jets"}; // default is central jets
    Gaudi::Property<float> m_minMass          {this, "minMass", -1, "Minimum mass of jets"};
    Gaudi::Property<float> m_maxMass          {this, "maxMass", -1, "Maximum mass of jets"};
    Gaudi::Property<int>   m_minimumAmount    {this, "minimumAmount", -1, "Minimum number of jets to consider"}; // -1 means ignores this
    Gaudi::Property<int>   m_maximumAmount    {this, "maximumAmount", -1, "Maximum number of jets to consider"};
    Gaudi::Property<bool>  m_pTsort           {this, "pTsort", true, "Sort jets by pT"};
    Gaudi::Property<int>   m_truncateAtAmount {this, "truncateAtAmount", -1, "Remove extra jets after pT sorting"}; // -1 means keep them all

    Gaudi::Property<bool>  m_hasTruthLabel    {this, "hasTruthLabel", false, "Does input container have a truth label for flavour selection?"};
    Gaudi::Property<int>   m_bjetAmount       {this, "bjetAmount", -1, "Number of jets to consider for isbjetXX decoration"};
    Gaudi::Property<int>   m_cjetAmount       {this, "cjetAmount", -1, "Number of jets to consider for iscjetXX decoration"};
    std::unordered_map<std::string, SG::WriteDecorHandleKey<xAOD::JetContainer>> m_bleadBranchesKeys;
    std::unordered_map<std::string, SG::WriteDecorHandleKey<xAOD::JetContainer>> m_cleadBranchesKeys;

  };
}

#endif