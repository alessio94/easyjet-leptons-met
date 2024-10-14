/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author James Howarth and Adam Anderson

#include "NeutrinoWeightingTool.h"

namespace HHBBLL {
  NeutrinoWeightingTool::NeutrinoWeightingTool(const std::string &type, const std::string &name, const IInterface *parent)
    : AthAlgTool(type, name, parent){
  };

  StatusCode NeutrinoWeightingTool::initialize(){
    if (m_resolution_settings == "fixed"){
      m_flag_res_fixed = true;
      m_flag_res_dynamic_const_x_met = false;
    } else if (m_resolution_settings == "dynamic"){
      m_flag_res_fixed = false;
      m_flag_res_dynamic_const_x_met = true;
    } else {
      ATH_MSG_ERROR("NW: You've asked for a resolution setting that doesn't exist. Please choose 'fixed' or 'dynamic'");
      return StatusCode::FAILURE;
    }

    m_res_fixed_value = m_resolution_number;
    m_res_dynamic_const_x_met_value = m_resolution_number;

    m_random = TRandom3(12345);

    return StatusCode::SUCCESS;
  }

  void NeutrinoWeightingTool::Reset(){
    m_tops.clear();
    m_tbars.clear();
    m_nus.clear();
    m_nubars.clear();
    m_Wposs.clear();
    m_Wnegs.clear();

    m_weights.clear();
    m_highestWeight = 0;
    m_weight_threshold = 0; // lower limit allowed for weights


    m_highestWeightTop   = TLorentzVector();
    m_highestWeightTbar  = TLorentzVector();
    m_highestWeightNu    = TLorentzVector();
    m_highestWeightNubar = TLorentzVector();
    m_highestWeightWpos  = TLorentzVector();
    m_highestWeightWneg  = TLorentzVector();

    m_eta_points_nu.clear();
    m_eta_points_nubar.clear(); 
  }


  StatusCode NeutrinoWeightingTool::SetupEtaSampling(TLorentzVector lepton_pos, TLorentzVector lepton_neg){


    ///-- This function setups up the sampling of eta steps. --///


    // Catch instances of users re-running when they don't need to
    if (m_flag_eta_sampling_linear && !m_flag_eta_sampling_do_random && (m_eta_points_nu.empty() || m_eta_points_nubar.empty())){
      ATH_MSG_WARNING("NW: It looks like you have asked for non-random linear sampling, but the eta scan point vectors are already filled.");
      ATH_MSG_WARNING("NW: This setting only needs to be run once, when the class is instantiated, you don't need to run it each time.");    
    }

    // Catch if user has asked for random sampling but hasn't set the seed to something new...
    if (m_flag_eta_sampling_do_random && m_random.GetSeed() == 12345 && m_flag_eta_sampling_linear){
      ATH_MSG_WARNING("NW: You've asked for random sampling and linear spacing but left the random seed to its initial value");
      ATH_MSG_WARNING("NW: This will lead to the same random sample points each time. Consider setting the seed to the event number or something similar with SetRandomSeed()");
    }

    // Catch inconsistent flag settings 
    if ((m_flag_eta_sampling_linear && m_flag_eta_sampling_gauss)||
        (m_flag_eta_sampling_linear && m_flag_eta_sampling_SM_lep) ||
        (m_flag_eta_sampling_gauss  && m_flag_eta_sampling_SM_lep)){
      ATH_MSG_WARNING("NW: You've asked for more than one type of sampling. ");
      ATH_MSG_WARNING("     flag_eta_sampling_linear = " << m_flag_eta_sampling_linear);
      ATH_MSG_WARNING("     flag_eta_sampling_gauss  = " << m_flag_eta_sampling_gauss );
      ATH_MSG_WARNING("     flag_eta_sampling_SM_lep = " << m_flag_eta_sampling_SM_lep);
      ATH_MSG_WARNING("NW: By default we'll run linear sampling with default settings and ignore the others");
      m_flag_eta_sampling_linear = true;
      m_flag_eta_sampling_gauss  = false;
      m_flag_eta_sampling_SM_lep = false;
    }

    m_eta_points_nu.clear();
    m_eta_points_nubar.clear();


    /////////////////////////////////////////////////////////////////////////////
    ///--                        Linear Sampling                            --///
    /////////////////////////////////////////////////////////////////////////////

    if (m_flag_eta_sampling_linear){
      if(m_flag_eta_sampling_do_random){
        for(uint i = 0; i < m_eta_sampling_nsamples; ++i){
          double point = m_random.Uniform(m_eta_sampling_linear_step_low, m_eta_sampling_linear_step_high);
          m_eta_points_nu.push_back(   point);
          m_eta_points_nubar.push_back(point);
        }
      } else {
        double step_size = (m_eta_sampling_linear_step_high - m_eta_sampling_linear_step_low)/m_eta_sampling_nsamples;
        for(uint i = 0; i < m_eta_sampling_nsamples; ++i){
          m_eta_points_nu.push_back(   m_eta_sampling_linear_step_low + i*step_size);
          m_eta_points_nubar.push_back(m_eta_sampling_linear_step_low + i*step_size);        
        }
      }
    } 

    /////////////////////////////////////////////////////////////////////////////
    ///--                        Gaussian Sampling                          --///
    /////////////////////////////////////////////////////////////////////////////
    else if (m_flag_eta_sampling_gauss){
      if(m_flag_eta_sampling_do_random){
        for(uint i = 0; i < m_eta_sampling_nsamples; ++i){
          m_eta_points_nu.push_back(   m_random.Gaus(m_eta_sampling_gauss_mean, m_eta_sampling_gauss_sigma));
          m_eta_points_nubar.push_back(m_random.Gaus(m_eta_sampling_gauss_mean, m_eta_sampling_gauss_sigma));
        }
      } else {
        ATH_MSG_ERROR("NW: THIS OPTION ISN'T IMPLEMENTED YET! DON'T USE!");
        return StatusCode::FAILURE;
      }    

    }


    /////////////////////////////////////////////////////////////////////////////
    ///--                  SM lep-nu eta relation Sampling                  --///
    /////////////////////////////////////////////////////////////////////////////
    else if (m_flag_eta_sampling_SM_lep){
      if(m_flag_eta_sampling_do_random){

        /// Check the leptons were actuall set properly ///
        if (lepton_pos==TLorentzVector() || lepton_neg==TLorentzVector()){
          ATH_MSG_ERROR("NW: YOU DIDN'T GIVE ME SENSIBLE LEPTONS TO RUN OVER!");
          return StatusCode::FAILURE;       
        }

        /*
        These are taken from ttbar MC by plotting the correlation between the
        lepton eta and neutrino eta in the W-boson decays and fitting a polynominal.
        These are the settings that were used in the Run2 ttbar measurements and may
        need to be reoptimised for Run3 and/or the bbll case.
        */

        double eta_mean_nu    = lepton_pos.Eta()*0.72 - 0.018*lepton_pos.Eta()*lepton_pos.Eta()*lepton_pos.Eta();
        double eta_mean_nubar = lepton_neg.Eta()*0.72 - 0.018*lepton_neg.Eta()*lepton_neg.Eta()*lepton_neg.Eta();
        double width = 1.16;

        for(uint i = 0; i < m_eta_sampling_nsamples; ++i){
          m_eta_points_nu.push_back(   m_random.Gaus(eta_mean_nu,    width));
          m_eta_points_nubar.push_back(m_random.Gaus(eta_mean_nubar, width));
        }
      } else {
        ATH_MSG_ERROR("NW: THIS OPTION ISN'T IMPLEMENTED YET! DON'T USE!");
        return StatusCode::FAILURE;
      } 

    }
    return StatusCode::SUCCESS;
  }


  StatusCode NeutrinoWeightingTool::Reconstruct(TLorentzVector lepton_pos, 
                                      TLorentzVector lepton_neg, 
                                      TLorentzVector b, 
                                      TLorentzVector bbar, 
                                      double met_ex, 
                                      double met_ey, 
                                      double mtop,
                                      double mtbar,
                                      double mWpos,
                                      double mWneg){


    ATH_CHECK(SetupEtaSampling(lepton_pos, lepton_neg));

    assert(m_eta_points_nu.size() == m_eta_points_nubar.size());

    std::vector<TLorentzVector> solution_nu;
    std::vector<TLorentzVector> solution_nubar;

    for(const auto eta : m_eta_points_nu){
      std::vector<TLorentzVector> temp_solution_nu = solveForNeutrinoEta(&lepton_pos, &b, eta, mtop, mWpos);
      solution_nu.insert( solution_nu.end(), temp_solution_nu.begin(), temp_solution_nu.end());

      if(m_stop_after_first_solution && solution_nu.empty()){
        break;
      }
    }

    for(const auto eta : m_eta_points_nubar){
      std::vector<TLorentzVector> temp_solution_nubar = solveForNeutrinoEta(&lepton_neg, &bbar, eta, mtbar, mWneg);
      solution_nubar.insert( solution_nubar.end(), temp_solution_nubar.begin(), temp_solution_nubar.end());

      if(m_stop_after_first_solution && solution_nubar.empty()){
        break;
      }
    }

    if(solution_nu.empty()   ) return StatusCode::SUCCESS;
    if(solution_nubar.empty()) return StatusCode::SUCCESS;

    TLorentzVector top, tbar, ttbar, Wpos, Wneg, nu, nubar;

    for(const auto& nu : solution_nu){
      for(const auto& nubar : solution_nubar){

        Wpos  = lepton_pos + nu;
        Wneg  = lepton_neg + nubar;
        top   = Wpos + b;
        tbar  = Wneg + bbar;

        m_tops.push_back(top);
        m_tbars.push_back(tbar);
        m_nus.push_back(nu);
        m_nubars.push_back(nubar);
        m_Wposs.push_back(Wpos);
        m_Wnegs.push_back(Wneg);

        float weight = get_weight(nu, nubar, met_ex, met_ey);
        m_weights.push_back(weight);
        if(weight > m_highestWeight && weight > m_weight_threshold && top.E() > 0 && tbar.E() > 0){      
          m_highestWeight        = weight;
          m_highestWeightTop   = top;
          m_highestWeightTbar  = tbar;
          m_highestWeightNu    = nu;
          m_highestWeightNubar = nubar;
          m_highestWeightWpos  = Wpos;
          m_highestWeightWneg  = Wneg;
        }
      }
    }
    return StatusCode::SUCCESS;
  }


  std::vector<TLorentzVector> NeutrinoWeightingTool::solveForNeutrinoEta(TLorentzVector* lepton, 
                                                                    TLorentzVector* bJet, 
                                                                    double nu_eta, 
                                                                    double mtop, 
                                                                    double mW) {

  /*
  Details of the calculation can be found in the following paper: 
  https://bonndoc.ulb.uni-bonn.de/xmlui/bitstream/handle/20.500.11811/3192/1277.pdf?sequence=1
  */

    double nu_cosh = cosh(nu_eta);
    double nu_sinh = sinh(nu_eta);

    double Wmass2  = mW*mW;
    double bmass   = 4.5;
    double Elprime = lepton->E() * nu_cosh - lepton->Pz() * nu_sinh;
    double Ebprime = bJet->E()   * nu_cosh - bJet->Pz()   * nu_sinh;

    double A = (lepton->Py() * Ebprime - bJet->Py() * Elprime) / (bJet->Px() * Elprime - lepton->Px() * Ebprime);
    double B = (Elprime * (mtop * mtop - Wmass2 - bmass * bmass - 2. * lepton->Dot(*bJet)) - Ebprime * Wmass2) / (2. * (lepton->Px() * Ebprime - bJet->Px() * Elprime));

    double par1 = (lepton->Px() * A + lepton->Py()) / Elprime;
    double C    = A * A + 1. - par1 * par1;
    double par2 = (Wmass2 / 2. + lepton->Px() * B) / Elprime;
    double D    = 2. * (A * B - par2 * par1);
    double F    = B * B - par2 * par2;
    double det  = D * D - 4. * C * F;

    std::vector<TLorentzVector> sol;

    ///-- 0 solutions case --///
    if (det < 0.0){
      return sol;
    }

    ///-- Only one real solution case --///
    if (det == 0.) {
      double py1 = -D / (2. * C);
      double px1 = A * py1 + B;
      double pT2_1 = px1 * px1 + py1 * py1;
      double pz1 = sqrt(pT2_1) * nu_sinh;

      TLorentzVector a1(px1, py1, pz1, sqrt(pT2_1 + pz1 * pz1));

      if (!TMath::IsNaN(a1.E()) )
        sol.push_back(a1);
      return sol;
    }

    ///-- 2 solutions case --///
    if(det > 0){
      double tmp   = sqrt(det) / (2. * C);
      std::vector<int> signs = {-1, 1};
      for (const auto sign: signs) {
        double py = -D / (2. * C) + sign * tmp;
        double px = A * py + B;
        double pT2 = px * px + py * py;
        double pz = sqrt(pT2) * nu_sinh;
        TLorentzVector a(px, py, pz, sqrt(pT2 + pz * pz));
        if (!TMath::IsNaN(a.E()) )
          sol.push_back(a);
      }

      return sol;
    }
    
    ///-- Should never reach this point --///
    return sol;
  }


  float NeutrinoWeightingTool::get_weight(TLorentzVector nu1, TLorentzVector nu2, double met_ex, double met_ey){

      double dx = met_ex - (nu1 + nu2).Px();
      double dy = met_ey - (nu1 + nu2).Py();

      double m_sigma_met_ex = 0;
      double m_sigma_met_ey = 0;

      if(m_flag_res_fixed && m_flag_res_dynamic_const_x_met){
        ATH_MSG_WARNING("NW: You've asked for both fixed and dynamic resoltion. Ignoring fixed and giving you dynamic (FIX THIS!)");
      }

      if(m_flag_res_fixed){
        m_sigma_met_ex = m_res_fixed_value;
        m_sigma_met_ey = m_res_fixed_value;
      }
      if(m_flag_res_dynamic_const_x_met){
        m_sigma_met_ex = m_res_dynamic_const_x_met_value*met_ex;
        m_sigma_met_ey = m_res_dynamic_const_x_met_value*met_ey;
      }  

      double numerator_x = -dx*dx;
      double numerator_y = -dy*dy;

      double denominator_x = m_sigma_met_ex*m_sigma_met_ex;
      double denominator_y = m_sigma_met_ey*m_sigma_met_ey;

      double exp_x = exp(numerator_x/denominator_x);
      double exp_y = exp(numerator_y/denominator_y);

      return static_cast<float>(exp_x*exp_y);
  }


  TLorentzVector NeutrinoWeightingTool::Average(std::vector<TLorentzVector> vecs){

    double average_m = 0;
    TVector3 three_vecs = TVector3();

    for (const auto& vec : vecs) {
      average_m += vec.M();
      three_vecs += vec.Vect();
    }

    three_vecs = three_vecs*(1./vecs.size());
    average_m = average_m/vecs.size();

    TLorentzVector new_vec = TLorentzVector();
    new_vec.SetPtEtaPhiM(three_vecs.Pt(), three_vecs.Eta(), three_vecs.Phi(), average_m);
    return new_vec;
  }

  StatusCode NeutrinoWeightingTool::finalize(){
    return StatusCode::SUCCESS;
  }
}
