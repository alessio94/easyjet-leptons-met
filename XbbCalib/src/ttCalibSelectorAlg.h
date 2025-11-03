/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/// @author Derrick Allen
/// @author Jason Oliver - systematics, scale factors, handle aliasing
// Always protect against multiple includes!

#ifndef SELECTIONFLAGSXBBCALIBALG_H
#define SELECTIONFLAGSXBBCALIBALG_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

// Systematics aware handles for retreiving object containers
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

// Once containers are retreived, we need xAOD object class
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace XBBCALIB
{

    using GaudiBool_t = Gaudi::Property<bool>;
    using GaudiFloat_t = Gaudi::Property<float>;
    using GaudiString_t = Gaudi::Property<std::string>;


    using listHandle_t = CP::SysListHandle;

    using EventInfoHandle_t  = CP::SysReadHandle<xAOD::EventInfo>;
    using JetHandle_t        = CP::SysReadHandle<xAOD::JetContainer>;
    using ElectronHandle_t   = CP::SysReadHandle<xAOD::ElectronContainer>;
    using MuonHandle_t       = CP::SysReadHandle<xAOD::MuonContainer>;
    using METHandle_t        = CP::SysReadHandle<xAOD::MissingETContainer>;

    using charHandle_t = CP::SysReadDecorHandle<char>;



    /// \brief An algorithm for counting containers
    class ttCalibSelectorAlg final : public AthHistogramAlgorithm {

        public:
            ttCalibSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
            /// \brief Initialisation method, for setting up tools and other persistent
            /// configs
            StatusCode initialize() override;
            /// \brief Execute method, for actions to be taken in the event loop
            StatusCode execute() override;
            /// \brief This is the mirror of initialize() and is called after all events are processed.
            StatusCode finalize() override; ///I added this to write the cutflow histogram.

        private :

        // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
        // "someInfo"};

        GaudiBool_t m_bypass { this, "bypass", false, "Run selector algorithm in pass-through mode" };
        listHandle_t  m_systematicsList {this};

        /// \brief Setup syst-aware input container handles

        EventInfoHandle_t  m_eventHandle            {this, "event",     "EventInfo",              "EventInfo container to read"};
        JetHandle_t        m_ttcalib_jetHandle     {this, "ttcalib_jets",      "ttCalibJets_%SYS%",     "Jet container to read"};
        JetHandle_t        m_ttcalib_lrjetHandle   {this, "ttcalib_lrjets",    "ttCalibLRJets_%SYS%",   "Large-R jet container to read"};
        ElectronHandle_t   m_ttcalib_electronHandle{this, "ttcalib_electrons", "ttCalibElectrons_%SYS%","Electron container to read"};
        MuonHandle_t       m_ttcalib_muonHandle    {this, "ttcalib_muons",     "ttCalibMuons_%SYS%",    "Muon container to read"};
        METHandle_t        m_metHandle              {this, "met",       "AnalysisMET_%SYS%",      "MET container to read"};



        GaudiFloat_t m_minMet {this, "minMet", 20000, "Minimum MET cut"};

        GaudiString_t m_eleWPName { this, "eleWP", "","Electron ID + Iso working point" };
        GaudiString_t m_muonWPName { this, "muonWP", "","Muon ID + Iso cuts" };

        charHandle_t m_eleWPDecorHandle{"", this};
        charHandle_t m_muonWPDecorHandle{"", this};

        CP::SysFilterReporterParams m_filterParams {this, "ttCalib selection"};

    };

}

#endif // SELECTIONFLAGSXBBCALIBALG_H
