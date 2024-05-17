/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NeutrinoWeightingTool_h
#define NeutrinoWeightingTool_h
#include <iostream>

#include "AthenaBaseComps/AthAlgTool.h"

#include "TRandom3.h"
#include "TLorentzVector.h"
#include <TMath.h>

namespace HHBBLL {

  class NeutrinoWeightingTool final: public AthAlgTool {

  private:

    Gaudi::Property<std::string> m_resolution_settings
    { this, "resolution_settings", "", "Select between fixed or dynamic resolution" };
    Gaudi::Property<float> m_resolution_number
    { this, "resolution_number", false, "Select resolution number" };

    std::vector<TLorentzVector> m_tops;
    std::vector<TLorentzVector> m_tbars;  
    std::vector<TLorentzVector> m_nus;
    std::vector<TLorentzVector> m_nubars;  
    std::vector<TLorentzVector> m_Wposs;
    std::vector<TLorentzVector> m_Wnegs;  

    std::vector<float> m_weights;

    TLorentzVector m_highestWeightTop;
    TLorentzVector m_highestWeightTbar;
    TLorentzVector m_highestWeightNu;
    TLorentzVector m_highestWeightNubar;
    TLorentzVector m_highestWeightWpos;
    TLorentzVector m_highestWeightWneg;

    TRandom3 m_random;

    bool m_flag_eta_sampling_linear; // sampling in equal step sizes
    bool m_flag_eta_sampling_SM_lep; // sampling using lep-nu eta SM relationship
    bool m_flag_eta_sampling_gauss;  // sampling using a user-defined gaussian
    bool m_flag_eta_sampling_do_random;  // sample using random numbers, not for linear

    float m_eta_sampling_linear_step_low;
    float m_eta_sampling_linear_step_high;
    float m_eta_sampling_gauss_mean;
    float m_eta_sampling_gauss_sigma;

    bool m_flag_res_fixed;
    bool m_flag_res_dynamic_const_x_met;
    float m_res_fixed_value;
    float m_res_dynamic_const_x_met_value;

    uint m_eta_sampling_nsamples; // how many sample points to do 

    std::vector<float> m_eta_points_nu;
    std::vector<float> m_eta_points_nubar;  

    float m_highestWeight;
    float m_weight_threshold;
    bool m_stop_after_first_solution;

  public:

    virtual ~NeutrinoWeightingTool() = default;
    NeutrinoWeightingTool(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize() override;

    StatusCode Reconstruct(TLorentzVector lepton_pos, 
                    TLorentzVector lepton_neg, 
                    TLorentzVector b, 
                    TLorentzVector bbar, 
                    double met_ex, 
                    double met_ey, 
                    double mtop,
                    double mtbar,
                    double mWpos,
                    double mWneg);

    float get_weight(TLorentzVector nu1, 
                      TLorentzVector nu2, 
                      double met_ex, 
                      double met_ey);

    void Reset();
    StatusCode SetupEtaSampling(); // sets up sampling points based on settings

    std::vector<TLorentzVector> solveForNeutrinoEta(TLorentzVector* lepton, 
                                                    TLorentzVector* bJet, 
                                                    double nu_eta, 
                                                    double mtop, 
                                                    double mW);

    ///-- Getters for objects --///
    std::vector<TLorentzVector> GetTops(){  return m_tops;};
    std::vector<TLorentzVector> GetTbars(){ return m_tbars;};
    std::vector<TLorentzVector> GetNus(){   return m_nus;};
    std::vector<TLorentzVector> GetNubars(){return m_nubars;};
    std::vector<TLorentzVector> GetWposs(){ return m_Wposs;};
    std::vector<TLorentzVector> GetWnegs(){ return m_Wnegs;};
    TLorentzVector              GetTop(){   return m_highestWeightTop;};
    TLorentzVector              GetTbar(){  return m_highestWeightTbar;};
    TLorentzVector              GetNu(){    return m_highestWeightNu;};
    TLorentzVector              GetNubar(){ return m_highestWeightNubar;};
    TLorentzVector              GetWpos(){  return m_highestWeightWpos;};
    TLorentzVector              GetWneg(){  return m_highestWeightWneg;};


    ///-- Functions Related to weights --///
    std::vector<float> GetWeights(){return m_weights;};
    float              GetWeight(){ return m_highestWeight;};
    void SetWeightThreshold(double choice=0.0){m_weight_threshold=choice;};


    ///-- Setters for sampling options --///
    void SetEtaSamplingLinear(          bool choice=true){ m_flag_eta_sampling_linear     = choice; m_flag_eta_sampling_SM_lep= false; m_flag_eta_sampling_gauss = false;};
    void SetEtaSamplingSMLep(           bool choice=true){ m_flag_eta_sampling_SM_lep     = choice; m_flag_eta_sampling_linear= false; m_flag_eta_sampling_gauss = false;};
    void SetEtaSamplingGauss(           bool choice=true){ m_flag_eta_sampling_gauss      = choice; m_flag_eta_sampling_linear= false; m_flag_eta_sampling_SM_lep= false;};
    void SetEtaSamplingDoRandom(        bool choice=true){ m_flag_eta_sampling_do_random  = choice;};
    void SetEtaSamplingLinearStepLow(  float choice=-5.0){ m_eta_sampling_linear_step_low = choice;};
    void SetEtaSamplingLinearStepHigh( float choice= 5.0){ m_eta_sampling_linear_step_low = choice;};
    void SetEtaSamplingGaussMean(      float choice= 0.0){ m_eta_sampling_gauss_mean      = choice;};
    void SetEtaSamplingGaussSigma(     float choice= 1.0){ m_eta_sampling_gauss_sigma     = choice;};  
    void SetEtaSamplingNsamples( int number=2){m_eta_sampling_nsamples = number;};
    StatusCode SetupEtaSampling(TLorentzVector, TLorentzVector);

    void SetFlagResFixed(            bool choice=false){ m_flag_res_fixed                = choice;};
    void SetFlagResDynamicConstXMet( bool choice=false){ m_flag_res_dynamic_const_x_met  = choice;};
    void SetResFixedValue(          float choice=10){    m_res_fixed_value               = choice;};
    void SetResConstXMetValue(      float choice=2){     m_res_dynamic_const_x_met_value = choice;};


    ///-- Other helper functions --///
    TLorentzVector Average(std::vector<TLorentzVector> vecs);
    void SetRandomSeed(int seed){m_random.SetSeed(seed);};
    void StopAfterFirstSolution(bool choice=false){m_stop_after_first_solution = choice;};

    ///-- Finalize --///
    virtual StatusCode finalize() override;
  };
}

  #endif