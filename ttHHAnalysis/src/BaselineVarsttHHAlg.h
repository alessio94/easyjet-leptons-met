/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef TTHHANALYSIS_FINALVARSTTHHALG
#define TTHHANALYSIS_FINALVARSTTHHALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <AthenaKernel/Units.h>

namespace ttHH
{

  /// \brief An algorithm for counting containers
  class BaselineVarsttHHAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsttHHAlg(const std::string &name, ISvcLocator *pSvcLocator);
    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};
    
    template<typename ParticleType>
      std::pair<int, int> truthOrigin(const ParticleType* particle);
    template<typename ParticleType>
      void updateLeptonBranch(const xAOD::EventInfo *event, int leptonIndex, const ParticleType* particle,  
                                       int lep_pdgid, float lep_sf, 
                                             const CP::SystematicSet& sys);
    
    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
    m_pairedJetHandle{ this, "pairedJets", "pairedttHHAnalysisJets_%SYS%", "Paired jet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_bjetHandle{ this, "bjets", "ttHHAnalysisJets_BTag_%SYS%", "BJet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "jets", "ttHHAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};

    CP::SysReadDecorHandle<int> 
    m_PCBT {this, "PCBTDecorName", "", "Name of pseudo-continuous b-tagging decorator"};

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "ttHHAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "ttHHAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%",   "MET container to read "};

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_SF{"", this};

    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_mu_SF{"", this};

    CP::SysReadDecorHandle<bool> 
      m_selected_el { this, "selected_el", "selected_el_%SYS%", "Name of input decorator for selected el"};
    CP::SysReadDecorHandle<bool> 
      m_selected_mu { this, "selected_mu", "selected_mu_%SYS%", "Name of input decorator for selected mu"};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
        m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int> > m_Ibranches;

    const float m_targetMassH = 125e3; // Higgs target mass to be used in chi square calculation
    const float m_massResolution = 20.0e3; // Mass resolution used in chi square calculation
    const double e_mass = .511 * Athena::Units::MeV;
    const double mu_mass = 105.6 * Athena::Units::MeV;
    const float topmass = 173 * Athena::Units::GeV;
    const float wmass = 80 * Athena::Units::GeV;

    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> getPairKinematics(const xAOD::JetContainer& jetPairs);
    std::tuple<double, double, double> calculateVectorStats(const std::vector<double>& inputVector);
    double computeChiSquaretops(const ConstDataVector<xAOD::JetContainer>& jets,
				std::vector<std::tuple<int, double>> leptonmasses,
				TLorentzVector met,
				bool top_had,
				std::vector<unsigned int> &jet_locations,
				std::vector<std::tuple<unsigned int, double>> &lepton_locations,
				const xAOD::ElectronContainer *electrons,
				const xAOD::MuonContainer *muons);
    float computeChiSquare(float observedMass1, float observedMass2, float targetMass1, float targetMass2, float massResolution);
  };
}

#endif
