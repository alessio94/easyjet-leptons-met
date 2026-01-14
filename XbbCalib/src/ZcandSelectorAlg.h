/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/

#ifndef ZCANSELALGO_H
#define ZCANSELALGO_H
#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>

#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>

#include <AthContainers/ConstDataVector.h>

namespace XBBCALIB{
  class ZcandSelectorAlg final : public EL::AnaAlgorithm{
    /// \brief The standard constructor
  public:
    ZcandSelectorAlg(const std::string& name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;

  private:
    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer> m_lrjetHandle{
      this, "lrjets", "",   "Large-R jet container to read" };
    CP::SysReadHandle<xAOD::PhotonContainer> m_photonHandle{
      this, "photons", "",   "Photon container to read" };
    //Z cand jets to write
    CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>> m_ZcandjetOutHandle{ this, "zCandidateJets", "", "Z Jets candidates container to write"};


  };
}
#endif
