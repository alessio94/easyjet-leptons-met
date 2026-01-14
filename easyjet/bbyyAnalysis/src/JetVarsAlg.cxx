/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetVarsAlg.h"

namespace HHBBYY
{
  JetVarsAlg::JetVarsAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode JetVarsAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       JetVarsAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_intVariables) {
      CP::SysWriteDecorHandle<int> var {string_var+"_%SYS%", this};
      m_Ibranches.emplace(string_var, var);
      ATH_CHECK (m_Ibranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_floatVariables) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }
  
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode JetVarsAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
        // In case of special Higgs sample, run only on NOSYS
        if (!m_doSystematics && sys.name()!="") continue;

        // container we read in
        const xAOD::EventInfo *event = nullptr;
        ANA_CHECK (m_eventHandle.retrieve (event, sys));
    
        // Jets
        const xAOD::JetContainer* jets = nullptr;
        ANA_CHECK (m_jetHandle.retrieve(jets, sys));

        // Check b-jet WP
        const bool WPgiven = !m_isBtag.empty();
        const std::size_t nJets = jets->size();

        const xAOD::Jet* j1 = nullptr;
        const xAOD::Jet* j2 = nullptr;

        if (nJets >= 1) j1 = (*jets)[0];
        if (nJets >= 2) j2 = (*jets)[1];

        // Number of (central) (b-)jets with pT > 30
        // standard number of jets with pT > 25 GeV is already calculated in BaselineVarsbbyyAlg
        int nJets_30 = 0;
        int nCentralJets_30 = 0;
        int nBJets_30 = 0;
        for (const auto jet : *jets) {
            if (jet->pt() > 30000.) nJets_30++;
            if (jet->pt() > 30000. && fabs(jet->eta()) < 2.5) nCentralJets_30++;
            if (jet->pt() > 30000. && fabs(jet->eta()) < 2.5 && WPgiven && m_isBtag.get(*jet, sys)) nBJets_30++;
        }


        if (!WPgiven) {
          nBJets_30 = -1; // if no b-tagging WP is given, set to -1
        }
        
        m_Ibranches.at("nJets_30").set(*event, nJets_30, sys);
        m_Ibranches.at("nCentralJets_30").set(*event, nCentralJets_30, sys);
        m_Ibranches.at("nBJets_30").set(*event, nBJets_30, sys);

        // Di-jet invariant mass
        // pT of the di-jet system
        float m_jj = -99.;
        float pT_jj = -99.;
        float m_jj_30 = -99.;
        if (nJets >= 2) {
            m_jj = (j1->p4() + j2->p4()).M();
            pT_jj = (j1->p4() + j2->p4()).Pt();
            if ( (j2->pt() > 30000.) ) {
                m_jj_30 = m_jj;
            }
        }
        m_Fbranches.at("m_jj").set(*event, m_jj, sys);
        m_Fbranches.at("pT_jj").set(*event, pT_jj, sys);
        m_Fbranches.at("m_jj_30").set(*event, m_jj_30, sys);

        // Leading jet pT (for pT > 30 GeV)
        float pT_j1_30 = -99.;
        if (nJets >= 1) {
            if ( (j1->pt() > 30000.) ) {
                pT_j1_30 = j1->pt();
            }
        }
        m_Fbranches.at("pT_j1_30").set(*event, pT_j1_30, sys);

        // Difference in rapidity, phi and eta between the two leading jets
        float Dy_j_j = -99.;
        float Dphi_j_j = -99.;
        float Deta_j_j = -99.;
        if (nJets >= 2) {
            Dy_j_j = fabs( j1->rapidity() - j2->rapidity() );
            Dphi_j_j = fabs( j1->p4().DeltaPhi(j2->p4()) );
            Deta_j_j = fabs( j1->eta() - j2->eta() );
        }
        m_Fbranches.at("Dy_j_j").set(*event, Dy_j_j, sys);
        m_Fbranches.at("Dphi_j_j").set(*event, Dphi_j_j, sys);
        m_Fbranches.at("Deta_j_j").set(*event, Deta_j_j, sys);

        // Invariant mass of all the jets
        TLorentzVector all;
        float m_alljets = -99.;
        for (auto jet : *jets) { all += jet->p4(); }
        if (nJets >= 3) {
            m_alljets = all.M();
        }
        m_Fbranches.at("m_alljets").set(*event, m_alljets, sys);

        // Scalar sum of pT of all jets with pT > 30 GeV (HT_30)
        float HT_30 = 0.;
        for (const auto jet : *jets) {
            if (jet->pt() > 30000.) HT_30 += jet->pt();
        }
        m_Fbranches.at("HT_30").set(*event, HT_30, sys);

    }

    return StatusCode::SUCCESS;
  }

}