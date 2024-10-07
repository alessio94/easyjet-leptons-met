/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/


// Always protect against multiple includes!
#ifndef VBSHIGGSANALYSIS_ENUMS
#define VBSHIGGSANALYSIS_ENUMS

namespace VBSHIGGS
{
  enum TriggerChannel
  {
    SLT,
  };

  enum Var_Trigger 
  {
    ele = 0,
    mu = 1,
    leadingele = 2,
    leadingmu = 3,
    subleadingele = 4,
    subleadingmu = 5,
  };

  enum RunBooleans
  {
    is22_75bunches,
    is23_75bunches,
    is23_400bunches,
  };

  enum Booleans{
    PASS_TRIGGER,
    PASS_ONE_LEPTON,
    PASS_AT_LEAST_TWO_LEPTONS,
    PASS_EXACTLY_TWO_LEPTONS,
    PASS_TWO_SS_CHARGE_LEPTONS,
    PASS_TWO_OS_CHARGE_LEPTONS,
    PASS_MET,
    PASS_RES_AT_LEAST_ONE_B_JET,
    PASS_RES_EXACTLY_ONE_B_JET,
    PASS_RES_EXACTLY_TWO_B_JETS,
    PASS_RES_NOADD_B_JET,
    PASS_ONE_LARGE_JET,
    PASS_TWO_SIGNAL_JETS,
    PASS_DELTA_R_BB,
    PASS_RES_H_WINDOW,
    PASS_VBS_BASELINE,
    PASS_RES_BASELINE,
    PASS_MERG_BASELINE,
    IS_SF,
    IS_ee,
    IS_mm,
    IS_em,
    Pass_ll,
  };
}

#endif