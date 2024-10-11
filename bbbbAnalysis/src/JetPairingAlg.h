/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_JETPAIRINGALG
#define HH4BANALYSIS_JETPAIRINGALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <xAODEventInfo/EventInfo.h>
#include "MVAUtils/BDT.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <xAODJet/JetContainer.h>

namespace HH4B
{
  class JetSet {
  public:
    float dr1 {0.};
    float dr2 {0.};
    float deta1 {0.};
    float deta2 {0.};
    float dphi1 {0.};
    float dphi2 {0.};
    float jet_system_pt_1 {0.};
    float jet_system_pt_2 {0.};
    float jet_system_mass_1 {0.};
    float jet_system_mass_2 {0.};
    float systems_mass_difference {0.};
    float m4j {0.};
    float BDT_score {0.};

    JetSet() = default;
  };

  enum PairingStrategy {
    minDeltaR = 0,
    BDT,
    invalid
  };

  enum pairingBDT {
    oddEvents  = 0,
    evenEvents = 1
  };

  enum pairingBDTinputs {
    dEta1 = 0,
    dPhi1 = 1,
    dR1 = 2,
    dEta2 = 3,
    dPhi2 = 4,
    dR2 = 5,
    recoM4j = 6,
    recoPtbb1 = 7,
    recoPtbb2 = 8,
    recoSHmassdiff = 9
  };

  /// \brief An algorithm for counting containers
  class JetPairingAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    JetPairingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

    PairingStrategy stringtoPairingStrategy(const std::string& pairing_strategy_str);
    void loadJetPairingBDT(const std::string &trainingGroup, std::string &trainingFile, std::unique_ptr<MVAUtils::BDT> &bdt);

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    CP::SysListHandle m_systematicsList {this};
    CP::SysReadHandle<xAOD::JetContainer>
      m_inJetHandle{this, "containerInKey", "", "Input jet container to read"};
    CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>>
      m_outJetHandle{this, "containerOutKey", "", "Output jet container to write"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
        this, "eventInfoKey", "EventInfo", "eventInfo container"};

    Gaudi::Property<std::string> m_pairingStrategy {
        this, "pairingStrategy", "", "Pairing strategy."};
    std::string m_bdt_kinematicGroup;
    // Declare the BDTs
    std::vector<std::unique_ptr<MVAUtils::BDT>> m_pairing_bdts;
    // Declare the enum for jet pairing strategy
    PairingStrategy m_pairing_strategy {invalid};
  };
}

#endif
