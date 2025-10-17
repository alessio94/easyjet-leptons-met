/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/// @author Derrick Allen

#include "BaselineVarsttCalibAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

namespace XBBCALIB
{
  BaselineVarsttCalibAlg::BaselineVarsttCalibAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  { }

  StatusCode BaselineVarsttCalibAlg::initialize()
  {
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      m_ele_SF = CP::SysReadDecorHandle<float>("effSF_"+m_eleWPName+"_%SYS%", this);
      ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));
      m_mu_SF = CP::SysReadDecorHandle<float>("effSF_"+m_muWPName+"_%SYS%", this);
      ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle));
    }


    for(const auto& wp: m_GN2X_wps)
      m_GN2X_wp_Handles.emplace_back("xbb_select_GN2Xv01_" + wp, this);
    for(auto& handle : m_GN2X_wp_Handles)
      ATH_CHECK(handle.initialize(m_systematicsList,  m_lrjetHandle));



    // Intialise syst-aware output decorators
    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &var : m_intVariables){
      ATH_MSG_DEBUG("initializing integer variable: " << var);
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_lrjetHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsttCalibAlg::execute()
  {
    //Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
      	ATH_MSG_ERROR("Could not retrieve MET");
      	return StatusCode::FAILURE;
      }
      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      bool hasOneLepton = false;

      TLorentzVector lepton;
      const xAOD::Jet *tagjet = nullptr;
      const xAOD::Jet *probejet = nullptr;
      float minDeltaR = std::numeric_limits<float>::max();

      if ( muons->size() == 1 && electrons->size() == 0 ) {
        lepton = muons->at(0)->p4();
        if(m_isMC){
          float SF = m_mu_SF.get(*muons->at(0),sys);
          m_Fbranches.at("Lepton1_effSF").set(*event, SF, sys);
        }
        hasOneLepton = true;
      } else if ( electrons->size() == 1 && muons->size() == 0 ) {
        lepton = electrons->at(0)->p4();
        if(m_isMC){
          float SF = m_ele_SF.get(*electrons->at(0),sys);
          m_Fbranches.at("Lepton1_effSF").set(*event, SF, sys);
        }
        hasOneLepton = true;
      }


      if ( hasOneLepton && met->met() > m_minMet) {
        for ( const xAOD::Jet *jet: *jets ) {
          float deltaR = lepton.DeltaR(jet->p4());
          if ( deltaR < minDeltaR ) {
            minDeltaR = deltaR;
            tagjet = jet;
          }
        }
      }


      if (tagjet && !lrjets->empty()) {
        probejet = lrjets->at(0);
        float phbb = m_GN2Xv01_phbb.get(*probejet, sys);
        float phcc = m_GN2Xv01_phcc.get(*probejet, sys);
        float pqcd = m_GN2Xv01_pqcd.get(*probejet, sys);
        float ptop = m_GN2Xv01_ptop.get(*probejet, sys);
        float fhbb = 0.03;
        float fhcc = 0.02;
        float ftop = 0.25;
        float dhcc = log(phcc / (fhbb * phbb + ftop * ptop + pqcd * (1 - fhbb - ftop)));
        float dhbb = log(phbb / (fhcc * phcc + ftop * ptop + pqcd * (1 - fhcc - ftop)));
        float tag_lep_m = (tagjet->p4() + lepton).M();
        m_Fbranches.at("tag_lep_m").set(*event, tag_lep_m, sys);
        m_Fbranches.at("probe_jet_eta").set(*event, probejet->eta(), sys);
        m_Fbranches.at("probe_jet_phi").set(*event, probejet->phi(), sys);
        m_Fbranches.at("probe_jet_pt").set(*event, probejet->pt(), sys);
        m_Fbranches.at("probe_jet_m").set(*event, probejet->m(), sys);
        m_Fbranches.at("probe_jet_phbb").set(*event, phbb, sys);
        m_Fbranches.at("probe_jet_phcc").set(*event, phcc, sys);
        m_Fbranches.at("probe_jet_pqcd").set(*event, pqcd, sys);
        m_Fbranches.at("probe_jet_ptop").set(*event, ptop, sys);
        m_Fbranches.at("probe_jet_dhcc").set(*event, dhcc, sys);
        m_Fbranches.at("probe_jet_dhbb").set(*event, dhbb, sys);
        m_Fbranches.at("tag_jet_pt").set(*event, tagjet->pt(), sys);
        m_Fbranches.at("tag_jet_eta").set(*event, tagjet->eta(), sys);
        m_Fbranches.at("tag_jet_phi").set(*event, tagjet->phi(), sys);
        m_Fbranches.at("tag_jet_m").set(*event, tagjet->m(), sys);
        m_Fbranches.at("dRJetLep").set(*event, minDeltaR, sys);
        m_Fbranches.at("lepton_pt").set(*event, lepton.Pt(), sys);
        m_Fbranches.at("lepton_eta").set(*event, lepton.Eta(), sys);
        m_Fbranches.at("lepton_phi").set(*event, lepton.Phi(), sys);

        for(unsigned int wp=0; wp<m_GN2X_wps.size(); wp++) {
          int pass_GN2X = static_cast<int>(m_GN2X_wp_Handles.at(wp).get(*lrjets->at(0), sys));
          m_Ibranches.at("probe_jet_Pass_GN2X_"+m_GN2X_wps[wp]).set(*event, pass_GN2X, sys);
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
