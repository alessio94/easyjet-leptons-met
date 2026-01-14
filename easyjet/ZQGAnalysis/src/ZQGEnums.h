#ifndef ZQGANALYSIS_ENUMS
#define ZQGANALYSIS_ENUMS

namespace ZQG
{
  enum TriggerChannel
  {
    SLT,
  };

  enum Var {
    ele = 0,
    mu = 1,
    leadingele = 2,
    subleadingele = 3,
    leadingmu = 4,
    subleadingmu = 5
  };

  enum Booleans
  {
    IS_ee,
    IS_mm,
    IS_em,

    IS_ee_TRUTH,
    IS_mm_TRUTH,
    IS_em_TRUTH,
    
    pass_trigger_SLT,
    pass_trigger_DLT,

    PASS_TRIGGER,
    EXACTLY_TWO_LEPTONS,
    OPPOSITE_CHARGE_LEPTONS,
    DILEPTON_MASS_WINDOW,
    ONE_B_JETS,
    TWO_B_JETS,
    ONE_C_JETS,
    TWO_C_JETS,
    ONE_LARGE_JET,


    EXACTLY_TWO_LEPTONS_TRUTH,
    OPPOSITE_CHARGE_LEPTONS_TRUTH,
    DILEPTON_MASS_WINDOW_TRUTH,
    ONE_B_JETS_TRUTH,
    TWO_B_JETS_TRUTH,
    ONE_C_JETS_TRUTH,
    TWO_C_JETS_TRUTH,
    ONE_LARGE_JET_TRUTH,
    
  };
};
#endif