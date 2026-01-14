/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TopRecoAlg.h"
#include "TFile.h"
#include "PathResolver/PathResolver.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <vector>

namespace 
{
  // Helper: validate finite floats
  inline bool isFinite(float x)
  {
    return std::isfinite(x);
  }

    // Compute neutrino pz using W mass constraint (all inputs in GeV)
  TLorentzVector buildLeptonicW(const TLorentzVector& lep, double met_x, double met_y) 
  {
    const double mW = 80.385; // GeV

    const double px_l = lep.Px();
    const double py_l = lep.Py();
    const double pz_l = lep.Pz();
    const double El   = lep.E();
    const double pt_l2 = px_l*px_l + py_l*py_l;

    TLorentzVector nu;

    // Avoid pathological cases
    if (pt_l2 < 1e-8) {
      nu.SetPxPyPzE(met_x, met_y, 0.0, std::sqrt(met_x*met_x + met_y*met_y));
      return lep + nu;
    }

    const double mu = (mW*mW)/2.0 + px_l*met_x + py_l*met_y;
    double A = mu * pz_l / pt_l2;
    double B2 = (El*El * (met_x*met_x + met_y*met_y) - mu*mu) / pt_l2; // (B)^2

    double pz_nu = 0.0;
    if (B2 >= 0.0) {
      // Two solutions; choose the smaller |pz|
      const double B = std::sqrt(B2);
      const double pz1 = A + B;
      const double pz2 = A - B;
      pz_nu = (std::abs(pz1) < std::abs(pz2)) ? pz1 : pz2;
    } else {
      // Clamp negative discriminant (type-II): set B=0 → single solution
      pz_nu = A;
    }

    const double p_nu = std::sqrt(met_x*met_x + met_y*met_y + pz_nu*pz_nu);
    nu.SetPxPyPzE(met_x, met_y, pz_nu, p_nu);
    
    return lep + nu; // W = l + ν
  }

} // anonymous namespace

namespace HHBBYY
{

  TopRecoAlg::TopRecoAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode TopRecoAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       TopRecoAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_eventHandle));
    }

    if (!m_PCBT.empty()) {
        ATH_CHECK (m_PCBT.initialize(m_systematicsList, m_jetHandle));
    }

    // Load BDT Model
    std::string resolvedPath = PathResolverFindCalibFile(m_topBDT_path);
    TFile *f = TFile::Open(resolvedPath.c_str());
    if (!f || f->IsZombie()) {
      ATH_MSG_ERROR("Cannot open file \"" << resolvedPath << "\" or the file is in a bad state.");
      return StatusCode::FAILURE;
    }

    TTree *tree = nullptr;
    f->GetObject("xgboost", tree);
    m_topBDT = std::make_unique<MVAUtils::BDT>(tree);

    f->Close();


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


  StatusCode TopRecoAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      // In case of special Higgs sample, run only on NOSYS
      if (!m_doSystematics && sys.name()!="") continue;

      // Inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer* jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve(jets, sys));

      const xAOD::MuonContainer* muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve(muons, sys));

      const xAOD::ElectronContainer* electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve(electrons, sys));

      const xAOD::MissingETContainer* met_container = nullptr;
      ANA_CHECK (m_metHandle.retrieve(met_container, sys));

      // Counters
      const int nLeptons = static_cast<int>(electrons->size() + muons->size());
      const int nJets    = static_cast<int>(jets->size());

      // Book-keeping for chosen indices and scores
      int   idx_bdt_leptop1   = -1; 
      float bdt_leptop1 = -1.f;
      int   idx1_bdt_hadtop1  = -1, idx2_bdt_hadtop1 = -1, idx3_bdt_hadtop1 = -1; 
      float bdt_hadtop1 = -1.f;
      int   idx1_bdt_hadtop2  = -1, idx2_bdt_hadtop2 = -1, idx3_bdt_hadtop2 = -1; 
      float bdt_hadtop2 = -1.f;

      // Variables in the order used by the BDT training
      float W_pt=-99.f, W_eta=-99.f, W_phi=-99.f, W_m=-99.f;
      float b_pt=-99.f, b_eta=-99.f, b_phi=-99.f, b_E=-99.f;
      float Wjj_dR=-99.f, tWb_dR=-99.f; 
      float t_m=-99.f;
      int btag1=-1, btag2=-1, btag3=-1;


      // Build leptonic W (if exactly one lepton)
      TLorentzVector lepW; 
      lepW.SetPtEtaPhiM(0.,0.,0.,0.);

      // --- Semi-leptonic branch: build leptonic top candidate ---
      if (nLeptons == 1 && nJets > 0) {
        // 1) Build the lepton 4-vector (GeV)
        TLorentzVector lep; lep.SetPtEtaPhiM(0.,0.,0.,0.);
        if (!muons->empty()) {
          const auto mu = (*muons)[0];
          lep = mu->p4() * 0.001;
        } else if (!electrons->empty()) {
          const auto el = (*electrons)[0];
          lep = el->p4() * 0.001;
        }

        // 2) MET (GeV)
        const xAOD::MissingET* met = nullptr;
        if (met_container) met = (*met_container)["Final"];
        if (!met) {
          ATH_MSG_WARNING("MissingET TST not found; skipping event");
          continue;
        }
        const double met_x = met->mpx()/1000.;
        const double met_y = met->mpy()/1000.;

        // 3) Build W = l + ν with mass constraint
        lepW = buildLeptonicW(lep, met_x, met_y);

        // 4) Score each jet as the b from the leptonic top
        int jidx = -1;
        for (const auto& jet : *jets) {
          ++jidx;
          TLorentzVector k1;
          k1 = jet->p4() * 0.001; 

          // BDT variables for leptonic case: we keep meaningful pieces and sentinel others
          W_pt = lepW.Pt(); 
          W_eta = lepW.Eta(); 
          W_phi = lepW.Phi(); 
          W_m = lepW.M();
          
          b_pt = k1.Pt(); 
          b_eta = k1.Eta(); 
          b_phi = k1.Phi(); 
          b_E = k1.E();
          Wjj_dR = -1.f; // no jj in leptonic W
          tWb_dR = lepW.DeltaR(k1);
          btag1 = m_PCBT.get(*jet, sys);
          btag2 = 0; 
          btag3 = 0;
          t_m = (k1 + lepW).M();

          std::vector<float> vars = {
                                     W_pt, W_eta, W_phi, W_m,
                                     b_pt, b_eta, b_phi, b_E, 
                                     Wjj_dR, tWb_dR,
                                     static_cast<float> (btag1),
                                     static_cast<float> (btag2),
                                     static_cast<float> (btag3),
                                     t_m
                                    };
          bool valid=true; 
          for (auto v:vars){ 
            if (!isFinite(v)){ 
              valid=false; 
              break; 
            }
          }
          if (!valid) continue;

          float score = m_topBDT->GetClassification(vars);
          if (score > bdt_leptop1) 
          { 
            bdt_leptop1 = score; 
            idx_bdt_leptop1 = jidx; 
          }
        }

        // 5) Additionally, try to reconstruct a hadronic top from remaining jets
        if (nJets > 2 && idx_bdt_leptop1 >= 0) {
          int i1=-1,i2=-1,i3=-1; 
          TLorentzVector k1,k2,k3;
          for (const auto jet1 : *jets) {
            ++i1; 
            if (i1==idx_bdt_leptop1) continue;
            k1 = jet1->p4() * 0.001;
            i2=-1;
            for (const auto jet2 : *jets) {
              ++i2; 
              if (i2==i1 || i2==idx_bdt_leptop1) continue;
              k2 = jet2->p4() * 0.001;
              i3=-1;
              for (const auto jet3 : *jets) {
                ++i3; 
                if (i3==i1 || i3==i2 || i3==idx_bdt_leptop1) continue;
                k3 = jet3->p4() * 0.001;

                W_pt=(k2+k3).Pt(); 
                W_eta=(k2+k3).Eta(); 
                W_phi=(k2+k3).Phi(); 
                W_m=(k2+k3).M();
                
                b_pt=k1.Pt(); 
                b_eta=k1.Eta(); 
                b_phi=k1.Phi(); 
                b_E=k1.E();
                
                Wjj_dR=k2.DeltaR(k3); 
                tWb_dR=(k2+k3).DeltaR(k1); 
                t_m=(k1+k2+k3).M();
                btag1 = m_PCBT.get(*jet1, sys);
                btag2 = m_PCBT.get(*jet2, sys);
                btag3 = m_PCBT.get(*jet3, sys);

                std::vector<float> vars = {
                                           W_pt,W_eta,W_phi,W_m,b_pt,
                                           b_eta,b_phi,b_E,Wjj_dR,tWb_dR,
                                           static_cast<float> (btag1),
                                           static_cast<float> (btag2),
                                           static_cast<float> (btag3),
                                           t_m
                                          };

                bool valid=true; 
                for (auto v:vars) if (!isFinite(v)) { valid=false; break; }
                if (!valid) continue;                
                float score = m_topBDT->GetClassification(vars);

                if (score > bdt_hadtop1) 
                { 
                  bdt_hadtop1=score; 
                  idx1_bdt_hadtop1=i1; 
                  idx2_bdt_hadtop1=i2; 
                  idx3_bdt_hadtop1=i3; 
                }
              }
            }
          }
        }
      }
      // --- Fully hadronic: scan for one or two hadronic tops ---
      else if (nLeptons == 0 && nJets > 2) {
        
        int i1=-1, i2=-1, i3=-1; 
        TLorentzVector k1, k2, k3;

        for (const auto jet1 : *jets) {
          ++i1; if (i1==idx_bdt_leptop1) continue; // likely -1 here, but keeps logic symmetric
          k1 = jet1->p4() * 0.001;
          i2=-1;
          
          for (const auto jet2 : *jets) {
            ++i2; if (i2==i1 || i2==idx_bdt_leptop1) continue;
            k2 = jet2->p4() * 0.001;
            i3=-1;
            for (const auto jet3 : *jets) {
              ++i3; if (i3==i1 || i3==i2 || i3==idx_bdt_leptop1) continue;
              k3 = jet3->p4() * 0.001;

              W_pt=(k2+k3).Pt(); W_eta=(k2+k3).Eta(); W_phi=(k2+k3).Phi(); W_m=(k2+k3).M();
              b_pt=k1.Pt(); b_eta=k1.Eta(); b_phi=k1.Phi(); b_E=k1.E();
              Wjj_dR=k2.DeltaR(k3); tWb_dR=(k2+k3).DeltaR(k1); t_m=(k1+k2+k3).M();
              btag1 = m_PCBT.get(*jet1, sys);
              btag2 = m_PCBT.get(*jet2, sys);
              btag3 = m_PCBT.get(*jet3, sys);
              std::vector<float> vars = {
                                         W_pt, W_eta, W_phi, W_m, b_pt,
                                         b_eta, b_phi, b_E, Wjj_dR, tWb_dR,
                                         static_cast<float> (btag1),
                                         static_cast<float> (btag2),
                                         static_cast<float> (btag3),
                                         t_m}
                                         ;
              bool valid=true; 
              for (auto v:vars) if (!isFinite(v)) { valid=false; break; }
              if (!valid) continue;
              float score = m_topBDT->GetClassification(vars);
              if (score > bdt_hadtop1) { bdt_hadtop1=score; idx1_bdt_hadtop1=i1; idx2_bdt_hadtop1=i2; idx3_bdt_hadtop1=i3; }
            }
          }
        }

        // Second hadronic top from remaining jets
        if (nJets > 5 && idx1_bdt_hadtop1>=0) {
          
          int j1=-1,j2=-1,j3=-1; 
          TLorentzVector r1,r2,r3;
          
          for (const auto jet1 : *jets) {
            ++j1; 
            if (j1==idx_bdt_leptop1) continue;
            if (j1==idx1_bdt_hadtop1 || j1==idx2_bdt_hadtop1 || j1==idx3_bdt_hadtop1) continue;
            r1 = jet1->p4() * 0.001;
            j2 = -1;
            
            for (const auto jet2 : *jets) 
            {
              ++j2; 
              
              if (j2==j1 || j2==idx_bdt_leptop1) continue;
              if (j2==idx1_bdt_hadtop1 || j2==idx2_bdt_hadtop1 || j2==idx3_bdt_hadtop1) continue;
              r2=jet2->p4() * 0.001;
              j3=-1;
              
              for (const auto jet3 : *jets) {
                
                ++j3; if (j3==j1 || j3==j2 || j3==idx_bdt_leptop1) continue;
                if (j3==idx1_bdt_hadtop1 || j3==idx2_bdt_hadtop1 || j3==idx3_bdt_hadtop1) continue;
                r3=jet3->p4() * 0.001;

                W_pt=(r2+r3).Pt(); 
                W_eta=(r2+r3).Eta(); 
                W_phi=(r2+r3).Phi(); 
                W_m=(r2+r3).M();
                
                b_pt=r1.Pt(); 
                b_eta=r1.Eta(); 
                b_phi=r1.Phi();
                b_E=r1.E();
                
                Wjj_dR=r2.DeltaR(r3); 
                tWb_dR=(r2+r3).DeltaR(r1);
                t_m=(r1+r2+r3).M();
                
                btag1 = m_PCBT.get(*jet1, sys);
                btag2 = m_PCBT.get(*jet2, sys);
                btag3 = m_PCBT.get(*jet3, sys);

                std::vector<float> vars = {
                                           W_pt,W_eta,W_phi,W_m,b_pt,
                                           b_eta,b_phi,b_E,Wjj_dR,tWb_dR,
                                           static_cast<float> (btag1),
                                           static_cast<float> (btag2),
                                           static_cast<float> (btag3),
                                           t_m
                                          };
                
                bool valid=true; for (auto v:vars) if (!isFinite(v)) { valid=false; break; }
                
                if (!valid) continue;
                float score = m_topBDT->GetClassification(vars);
                if (score > bdt_hadtop2) 
                { 
                  bdt_hadtop2=score; 
                  idx1_bdt_hadtop2=j1; 
                  idx2_bdt_hadtop2=j2; 
                  idx3_bdt_hadtop2=j3; 
                }
              }
            }
          }
        }
      }

      // --- Store top scores ---
      // Topology flags (default zeros)
      int flag_is_top1_had = 0;
      int flag_is_top1_lep = 0;
      int flag_has_top2    = 0;

      float score_recotop1 = -1.f;
      if (nLeptons == 1 && nJets > 0 && bdt_leptop1 >= 0.f) {
        score_recotop1 = bdt_leptop1;
      } else if (nLeptons == 0 && nJets > 2 && bdt_hadtop1 >= 0.f) {
        score_recotop1 = bdt_hadtop1;
      }
      m_Fbranches.at("score_recotop1").set(*event, score_recotop1, sys);

      // Set flags for top1 type
      if (nLeptons == 1 && nJets > 0 && bdt_leptop1 >= 0.f) flag_is_top1_lep = 1;
      if (nLeptons == 0 && nJets > 2 && bdt_hadtop1 >= 0.f) flag_is_top1_had = 1;

      float score_recotop2 = -1.f;
      if (nLeptons == 0 && nJets > 5 && bdt_hadtop2 >= 0.f) {
        score_recotop2 = bdt_hadtop2;
      } else if (nLeptons == 1 && nJets > 3 && bdt_hadtop1 >= 0.f) {
        score_recotop2 = bdt_hadtop1; // semi-leptonic second top = hadronic triplet
      }
      m_Fbranches.at("score_recotop2").set(*event, score_recotop2, sys);


      // Set has_top2 if we actually formed a proper second top
      if ( (nLeptons==0 && nJets>5 && bdt_hadtop2>=0.f) || (nLeptons==1 && nJets>3 && bdt_hadtop1>=0.f) ) {
        flag_has_top2 = 1;
      }

      // --- Build kinematics ---
      TLorentzVector top1, top2;

      if (nLeptons == 1 && nJets > 0 &&
          bdt_leptop1 >= 0.f &&
          idx_bdt_leptop1 >= 0 && idx_bdt_leptop1 < nJets)
      {
        const xAOD::Jet* bjet = (*jets)[idx_bdt_leptop1];            
        
        TLorentzVector b = bjet->p4() * 0.001;
        top1 = lepW + b;
      }
      else if (nLeptons == 0 && nJets > 2 &&
               bdt_hadtop1 >= 0.f &&
               idx1_bdt_hadtop1 >= 0 && idx3_bdt_hadtop1 < nJets)
      {
        const xAOD::Jet* j1 = (*jets)[idx1_bdt_hadtop1];
        const xAOD::Jet* j2 = (*jets)[idx2_bdt_hadtop1];
        const xAOD::Jet* j3 = (*jets)[idx3_bdt_hadtop1];
      

        TLorentzVector k1 = j1->p4() * 0.001;
        TLorentzVector k2 = j2->p4() * 0.001;
        TLorentzVector k3 = j3->p4() * 0.001; 
        top1 = k1 + k2 + k3;
      }
      
      if (score_recotop2 >= 0.f) {
        if (nLeptons == 1 && nJets > 3 &&
            bdt_hadtop1 >= 0.f &&
            idx1_bdt_hadtop1 >= 0 && idx3_bdt_hadtop1 < nJets)
        {
          const xAOD::Jet* j1 = (*jets)[idx1_bdt_hadtop1];
          const xAOD::Jet* j2 = (*jets)[idx2_bdt_hadtop1];
          const xAOD::Jet* j3 = (*jets)[idx3_bdt_hadtop1];
      
          TLorentzVector k1 = j1->p4() * 0.001;
          TLorentzVector k2 = j2->p4() * 0.001;
          TLorentzVector k3 = j3->p4() * 0.001; 

          top2 = k1 + k2 + k3;
        }
        else if (nLeptons == 0 && nJets > 5 &&
                 bdt_hadtop2 >= 0.f &&
                 idx1_bdt_hadtop2 >= 0 && idx3_bdt_hadtop2 < nJets)
        {
          const xAOD::Jet* j1 = (*jets)[idx1_bdt_hadtop2];
          const xAOD::Jet* j2 = (*jets)[idx2_bdt_hadtop2];
          const xAOD::Jet* j3 = (*jets)[idx3_bdt_hadtop2];

          TLorentzVector k1 = j1->p4() * 0.001;
          TLorentzVector k2 = j2->p4() * 0.001;
          TLorentzVector k3 = j3->p4() * 0.001;

          top2 = k1 + k2 + k3;

        }
      }

      // Store kinematics (GeV)
      m_Fbranches.at("recotop1_pT").set(*event, static_cast<float>(top1.Pt()), sys);
      m_Fbranches.at("recotop1_eta").set(*event, static_cast<float>(top1.Eta()), sys);
      m_Fbranches.at("recotop1_phi").set(*event, static_cast<float>(top1.Phi()), sys);
      m_Fbranches.at("recotop1_m").set(*event,  static_cast<float>(top1.M()), sys);
      
      m_Fbranches.at("recotop2_pT").set(*event, static_cast<float>(top2.Pt()), sys);
      m_Fbranches.at("recotop2_eta").set(*event, static_cast<float>(top2.Eta()), sys);
      m_Fbranches.at("recotop2_phi").set(*event, static_cast<float>(top2.Phi()), sys);
      m_Fbranches.at("recotop2_m").set(*event,  static_cast<float>(top2.M()), sys);

      // --- Compute dR_Wb_t2 ---
      float dR_Wb_t2 = std::numeric_limits<float>::quiet_NaN();

      auto get_pcb = [&](int jidx)->int {
        if (jidx < 0 || jidx >= nJets) return std::numeric_limits<int>::min();
        return m_PCBT.get(*(*jets)[jidx], sys);
      };

      auto compute_dR_for_triplet = [&](int a, int b, int c)->float {
        int ids[3] = {a,b,c};
        int pcb[3] = {get_pcb(a), get_pcb(b), get_pcb(c)};
        int ib = 0; 
        if (pcb[1] > pcb[ib]) ib = 1; 
        if (pcb[2] > pcb[ib]) ib = 2;
        int bidx = ids[ib]; 
        int w1 = ids[(ib+1) % 3]; 
        int w2 = ids[(ib+2) % 3];

        TLorentzVector jb = (*jets)[bidx]->p4() * 0.001;
        TLorentzVector j1 = (*jets)[w1]->p4() * 0.001;
        TLorentzVector j2 = (*jets)[w2]->p4() * 0.001;
        TLorentzVector W = j1 + j2;

        return static_cast<float>(W.DeltaR(jb));

      };

      if (nLeptons==0 && nJets>5 && bdt_hadtop2>=0.f && idx1_bdt_hadtop2>=0 && idx3_bdt_hadtop2<nJets) {
        dR_Wb_t2 = compute_dR_for_triplet(idx1_bdt_hadtop2, idx2_bdt_hadtop2, idx3_bdt_hadtop2);
      } else if (nLeptons==1 && nJets>3 && bdt_hadtop1>=0.f && idx1_bdt_hadtop1>=0 && idx3_bdt_hadtop1<nJets) {
        // semi-leptonic case: second top = hadtop1 triplet
        dR_Wb_t2 = compute_dR_for_triplet(idx1_bdt_hadtop1, idx2_bdt_hadtop1, idx3_bdt_hadtop1);
      } else {
        // hybrid fallback: if exactly 5 jets and top1 was hadronic, take the two remaining jets
        if (nJets==5 && nLeptons==0 && idx1_bdt_hadtop1>=0 && idx3_bdt_hadtop1<nJets) {
          std::vector<int> rest; rest.reserve(2);
          for (int i=0;i<nJets;++i) {
            if (i==idx1_bdt_hadtop1 || i==idx2_bdt_hadtop1 || i==idx3_bdt_hadtop1) continue;
            rest.push_back(i);
          }
          if ((int)rest.size()==2) {
            TLorentzVector j1 = (*jets)[rest[0]]->p4() * 0.001;
            TLorentzVector j2 = (*jets)[rest[1]]->p4() * 0.001;
            dR_Wb_t2 = static_cast<float>(j1.DeltaR(j2));
          }
        }
      }

      m_Fbranches.at("dR_Wb_t2").set(*event, dR_Wb_t2, sys);
      
      // Write topology flags
      m_Ibranches.at("is_top1_had").set(*event, flag_is_top1_had, sys);
      m_Ibranches.at("is_top1_lep").set(*event, flag_is_top1_lep, sys);
      m_Ibranches.at("has_top2").set(*event,    flag_has_top2,    sys);


        // === HYBRID TOP IMPLEMENTATION ===
        // Definition: if a fully reconstructed second top exists, hybrid=that top;
        // otherwise, hybrid = sum of all jets NOT used by the first reconstructed top.
        TLorentzVector hybrid2; hybrid2.SetPtEtaPhiM(0.,0.,0.,0.);
        int is_hybrid2 = 0;

        //  if we have a fully reconstructed top2 (score_recotop2 >= 0), reuse it as hybrid
        if (score_recotop2 >= 0.f && top2.Pt() > 0.) {
            hybrid2 = top2;
            is_hybrid2 = 0; // not a fallback
        }
        // build remainder system as fallback
        else {
            // Build a mask of jets to exclude (those used by the first reconstructed top)
            std::vector<int> exclude;
            if (nLeptons == 1 && idx_bdt_leptop1 >= 0) {
                exclude.push_back(idx_bdt_leptop1);
            } else if (nLeptons == 0 && idx1_bdt_hadtop1 >= 0) {
                exclude.push_back(idx1_bdt_hadtop1);
                exclude.push_back(idx2_bdt_hadtop1);
                exclude.push_back(idx3_bdt_hadtop1);
            }
            auto isExcluded = [&exclude](int i){ return std::find(exclude.begin(), exclude.end(), i) != exclude.end(); };

            // Sum all remaining jets
            for (int i=0; i<nJets; ++i) {
                if (isExcluded(i)) continue;
                const auto j = (*jets)[i];
                TLorentzVector k; 
                k.SetPtEtaPhiM(j->pt()/1000., j->eta(), j->phi(), j->m()/1000.);
                hybrid2 += k;
            }
            if (hybrid2.Pt() > 0.) {
                is_hybrid2 = 1; // we used the fallback definition
            }
        }

        // Store hybrid-top2 variables 
        m_Fbranches.at("hybrtop_pT").set(*event, static_cast<float>(hybrid2.Pt()), sys);
        m_Fbranches.at("hybrtop_eta").set(*event, static_cast<float>(hybrid2.Eta()), sys);
        m_Fbranches.at("hybrtop_phi").set(*event, static_cast<float>(hybrid2.Phi()), sys);
        m_Fbranches.at("hybrtop_m").set(*event,  static_cast<float>(hybrid2.M()), sys);
        m_Ibranches.at("is_hybridtop2").set(*event, is_hybrid2, sys);
    }

    return StatusCode::SUCCESS;
  }

}