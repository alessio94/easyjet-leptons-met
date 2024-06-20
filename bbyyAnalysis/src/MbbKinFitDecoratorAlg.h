/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef MBBKINFIT_DECORATORALG
#define MBBKINFIT_DECORATORALG

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <AthContainers/ConstDataVector.h>

#include "KinematicFitTool/KinematicFitTool.h"

namespace HHBBYY {
      /// \brief An algorithm for counting containers
  class MbbKinFitDecoratorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
  public:
      MbbKinFitDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// We use default finalize() -- this is for cleanup, and we don't do any

  private:

      /// \brief Setup syst-aware input container handles
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::PhotonContainer>
      m_photonHandle{ this, "photons", "", "Photon container to read"};

      CP::SysReadHandle<xAOD::JetContainer>
        m_jetHandle{ this, "jets", "",   "Jet container to read" };
      
      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      /// \brief Setup sys-aware output decorations

      CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>>
      m_jetOutHandle{ this, "jetContainerOutKey", "jetContainerOutKey",   "Jet container to write" };
            
    /// \brief Steerable properties
      Gaudi::Property<std::string> m_BtaggingWP { this, "bTagWPDecorName", "ftag_select_GN2v01LegacyWP_FixedCutBEff_77", 
              "B-Tagging WP to use"};
      Gaudi::Property<float> m_Jet_Min_pt { this, "JetMinPt", 20., 
              "Minimum Jet pT"};
      Gaudi::Property<float> m_angles_Res { this, "AnglesResolution", .01, 
              "Angles Resolution"};
      Gaudi::Property<bool> m_isFixAngles { this, "FixAnglesFit", true, 
              "Fix Angles in Fit"};       

      std::unique_ptr<KinematicFitTool> m_KFTool;

  };
}
  #endif
