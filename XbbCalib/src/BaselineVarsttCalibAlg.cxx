/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/// @author Derrick Allen
/// @author Jason Oliver - systematics, scale factors, handle aliasing.
#include "BaselineVarsttCalibAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

namespace XBBCALIB
{
    BaselineVarsttCalibAlg::BaselineVarsttCalibAlg(const std::string &name,
                                            ISvcLocator *pSvcLocator)
        : AthHistogramAlgorithm(name, pSvcLocator)
    { }

    StatusCode BaselineVarsttCalibAlg::initialize(){
        ATH_CHECK (m_ttcalib_jetHandle.initialize(m_systematicsList));
        ATH_CHECK (m_ttcalib_lrjetHandle.initialize(m_systematicsList));
        ATH_CHECK (m_ttcalib_muonHandle.initialize(m_systematicsList));
        ATH_CHECK (m_ttcalib_electronHandle.initialize(m_systematicsList));
        ATH_CHECK (m_metHandle.initialize(m_systematicsList));
        ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

        if(m_isMC){

            // retreive the AnalysisElectrons and AnalysisMuon Containers for scale factor extraction
            ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
            ATH_CHECK (m_muonHandle.initialize(m_systematicsList));

            // scale factor Read handles
            m_ele_SF = FloatHandle_t("effSF_"+m_electronWPName+"_%SYS%", this);
            m_mu_SF = FloatHandle_t("effSF_"+m_muonWPName+"_%SYS%", this);

            // initialize the scale factos from AnalysisElectrons and AnalysisMuons
            ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));
            ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle));

        }


        for(const auto& WP: m_GN2X_WPs){
            m_GN2X_WP_Handles.emplace_back("xbb_select_GN2Xv01_" + WP, this);
        }

        for(auto& GN2XHandle : m_GN2X_WP_Handles){
            ATH_CHECK(GN2XHandle.initialize(m_systematicsList,  m_ttcalib_lrjetHandle));
        }


        // Intialise syst-aware output decorators
        for (const std::string &variable : m_floatVariables) {
            FloatWriteHandle_t writeHandle{variable+"_%SYS%", this};
            m_floatBranches.emplace(variable, writeHandle);
            ATH_CHECK (m_floatBranches.at(variable).initialize(m_systematicsList, m_eventHandle));

        }

        for (const std::string &variable : m_intVariables){
            ATH_MSG_DEBUG("initializing integer variable: " << variable);
            IntWriteHandle_t writeHandle{variable+"_%SYS%", this};
            m_intBranches.emplace(variable, writeHandle);
            ATH_CHECK(m_intBranches.at(variable).initialize(m_systematicsList, m_eventHandle));

        };

        ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_ttcalib_lrjetHandle));
        ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_ttcalib_lrjetHandle));
        ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_ttcalib_lrjetHandle));
        ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_ttcalib_lrjetHandle));

        // Intialise syst list (must come after all syst-aware inputs and outputs)
        ATH_CHECK (m_systematicsList.initialize());

        return StatusCode::SUCCESS;
    }

    StatusCode BaselineVarsttCalibAlg::execute(){
        //Loop over all systs
        for (const auto& systematic : m_systematicsList.systematicsVector()){


            const xAOD::EventInfo *event = nullptr;
            const xAOD::JetContainer *jets = nullptr;
            const xAOD::JetContainer *lrjets = nullptr;
            const xAOD::MuonContainer *muons = nullptr;
            const xAOD::ElectronContainer *electrons = nullptr;
            const xAOD::MissingETContainer *metCont = nullptr;


            ANA_CHECK (m_eventHandle.retrieve (event, systematic));
            ANA_CHECK (m_ttcalib_jetHandle.retrieve (jets, systematic));
            ANA_CHECK (m_ttcalib_lrjetHandle.retrieve (lrjets, systematic));
            ANA_CHECK (m_ttcalib_muonHandle.retrieve (muons, systematic));
            ANA_CHECK (m_ttcalib_electronHandle.retrieve (electrons, systematic));
            ANA_CHECK (m_metHandle.retrieve (metCont, systematic));

            const xAOD::MissingET* met = (*metCont)["Final"];

            if (!met) {
                ATH_MSG_ERROR("Could not retrieve MET");
                return StatusCode::FAILURE;

            }



            for (const std::string &string_var: m_floatVariables) {
                m_floatBranches.at(string_var).set(*event, -99., systematic);

            }

            for (const auto& var: m_intVariables) {
                m_intBranches.at(var).set(*event, -99, systematic);

            }

            int nLeptons = muons->size() + electrons->size();
            int nMuons = muons->size();
            int nElectrons = electrons->size();
            bool isSingleLeptonEvent = (nLeptons == 1);
            bool isSingleMuonEvent = (nMuons == 1 && nElectrons == 0);
            bool isSingleElectronEvent = (nElectrons == 1 && nMuons == 0);

            int nLargeRJets = lrjets->size();
            int hasProbeJetCandidate = (nLargeRJets >= 1) ? 1 : 0;

            bool pass_minMet_selection = met->met() > m_minMet;


            TLorentzVector lepton;
            const xAOD::Jet *tagjet   = nullptr;
            const xAOD::Jet *probejet = nullptr;
            float minDeltaR = std::numeric_limits<float>::max();

            if ( isSingleMuonEvent ) {
                lepton = muons->at(0)->p4();
                if(m_isMC){
                    float SF = m_mu_SF.get(*muons->at(0), systematic);
                    m_floatBranches.at("Lepton1_effSF").set(*event, SF, systematic);
                }

            } else if ( isSingleElectronEvent ) {
                lepton = electrons->at(0)->p4();
                if(m_isMC){
                    float SF = m_ele_SF.get(*electrons->at(0),systematic);
                    m_floatBranches.at("Lepton1_effSF").set(*event, SF, systematic);
                }

            }


            if ( isSingleLeptonEvent && pass_minMet_selection) {
                for ( const xAOD::Jet *jet: *jets ) {
                    float deltaR = lepton.DeltaR(jet->p4());
                    if ( deltaR < minDeltaR ) {
                        minDeltaR = deltaR;
                        tagjet = jet;
                    }
                }
            }


            if (tagjet && hasProbeJetCandidate) {
                probejet = lrjets->at(0);
                float phbb = m_GN2Xv01_phbb.get(*probejet, systematic);
                float phcc = m_GN2Xv01_phcc.get(*probejet, systematic);
                float pqcd = m_GN2Xv01_pqcd.get(*probejet, systematic);
                float ptop = m_GN2Xv01_ptop.get(*probejet, systematic);
                float fhbb = 0.03;
                float fhcc = 0.02;
                //float ftop = 0.25;
                float ftop = 0.;
                float dhcc = log(phcc / (fhbb * phbb + ftop * ptop + pqcd * (1 - fhbb - ftop)));
                float dhbb = log(phbb / (fhcc * phcc + ftop * ptop + pqcd * (1 - fhcc - ftop)));
                float tag_lep_m = (tagjet->p4() + lepton).M();
                m_floatBranches.at("tag_lep_m").set(*event, tag_lep_m, systematic);
                m_floatBranches.at("probe_jet_eta").set(*event, probejet->eta(), systematic);
                m_floatBranches.at("probe_jet_phi").set(*event, probejet->phi(), systematic);
                m_floatBranches.at("probe_jet_pt").set(*event, probejet->pt(), systematic);
                m_floatBranches.at("probe_jet_m").set(*event, probejet->m(), systematic);
                m_floatBranches.at("probe_jet_phbb").set(*event, phbb, systematic);
                m_floatBranches.at("probe_jet_phcc").set(*event, phcc, systematic);
                m_floatBranches.at("probe_jet_pqcd").set(*event, pqcd, systematic);
                m_floatBranches.at("probe_jet_ptop").set(*event, ptop, systematic);
                m_floatBranches.at("probe_jet_dhcc").set(*event, dhcc, systematic);
                m_floatBranches.at("probe_jet_dhbb").set(*event, dhbb, systematic);
                m_floatBranches.at("tag_jet_pt").set(*event, tagjet->pt(), systematic);
                m_floatBranches.at("tag_jet_eta").set(*event, tagjet->eta(), systematic);
                m_floatBranches.at("tag_jet_phi").set(*event, tagjet->phi(), systematic);
                m_floatBranches.at("tag_jet_m").set(*event, tagjet->m(), systematic);
                m_floatBranches.at("dRJetLep").set(*event, minDeltaR, systematic);
                m_floatBranches.at("lepton_pt").set(*event, lepton.Pt(), systematic);
                m_floatBranches.at("lepton_eta").set(*event, lepton.Eta(), systematic);
                m_floatBranches.at("lepton_phi").set(*event, lepton.Phi(), systematic);

                for(unsigned int wp=0; wp<m_GN2X_WPs.size(); wp++) {
                    int pass_GN2X = static_cast<int>(m_GN2X_WP_Handles.at(wp).get(*probejet, systematic));
                    m_intBranches.at("probe_jet_Pass_GN2X_"+m_GN2X_WPs[wp]).set(*event, pass_GN2X, systematic);
                }
            }
        }

        return StatusCode::SUCCESS;
    }
}
