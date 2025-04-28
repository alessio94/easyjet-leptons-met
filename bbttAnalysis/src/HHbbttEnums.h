#ifndef BBTTANALYSIS_ENUMS
#define BBTTANALYSIS_ENUMS

namespace HHBBTT
{

  enum Channel
  {
    LepHad,
    HadHad,
    ZCR,
    TopEMuCR,
    AntiIsoLepHad,
    Boosted
  };

  enum TriggerChannel
  {
    SLT,
    LTT,
    ETT,
    ETT_4J12,
    MTT_2016,
    MTT_high,
    MTT_low,
    STT,
    DTT,
    DBT,
    DTT_2016,
    DTT_4J12,
    DTT_L1Topo,
    DTT_4J12_delayed,
    DTT_L1Topo_delayed,
    trigMatch_Tau35,
    trigMatch_Tau25,
    L1,
    HLT,
    LARGE_R_JETS
  };

  enum Var
  {
    ele = 0,
    mu = 1,
    leadingtau = 2,
    leadingtaumax = 3,
    subleadingtau = 4,
    leadingjet = 5,
    subleadingjet = 6,
    leadingjet_matchL1 = 7,
    subleadingjet_matchL1 = 8,
  };

  enum RunBooleans
  {
    is16PeriodA,
    is16PeriodB_D3,
    is16PeriodD4_end,
    is17PeriodB1_B4,
    is17PeriodB5_B7,
    is17PeriodB8_end,
    is18PeriodB_end,
    is18_tau35_medium1,
    is18PeriodK_end,
    is22_75bunches,
    is23_75bunches,
    is23_400bunches,
    is23_from1200bunches,
    is23_first_2400bunches,
    l1topo_disabled
  };

  enum Booleans
  {
    pass_trigger_SR,
    pass_trigger_SLT,
    pass_trigger_LTT,
    pass_trigger_STT,
    pass_trigger_DTT,
    pass_trigger_DTT_2016,
    pass_trigger_DTT_4J12,
    pass_trigger_DTT_L1Topo,
    pass_trigger_DTT_4J12_delayed,
    pass_trigger_DTT_L1Topo_delayed,
    pass_trigger_DBT,
    pass_trigger_LARGE_R_JETS,
    
    AT_LEAST_TWO_LRJETS,
    TWO_JETS,
    TWO_BJETS,
    ONE_BJET,
    MTAUTAU_VIS_MASS_CUT_LEPHAD,
    N_LEPTONS_CUT_LEPHAD,
    N_LEPTONS_CUT_ANTIISOLEPHAD,
    ONE_TAU,
    OS_CHARGE_LEPHAD,
    pass_baseline_SLT,
    pass_baseline_LTT,
    pass_SLT_2B,
    pass_LTT_2B,
    pass_SLT_1B,
    pass_LTT_1B,

    MTAUTAU_VIS_MASS_CUT_HADHAD,
    N_LEPTONS_CUT_HADHAD,
    TWO_TAU,
    OS_CHARGE_HADHAD,
    OS_CHARGE_LEPTONS,
    pass_baseline_STT,
    pass_baseline_DTT_2016,
    pass_baseline_DTT_4J12,
    pass_baseline_DTT_L1Topo,
    pass_baseline_DTT,
    pass_baseline_DBT,
    pass_baseline_SR,

    pass_STT_2B,
    pass_DTT_2016_2B,
    pass_DTT_4J12_2B,
    pass_DTT_L1Topo_2B,
    pass_DTT_4J12_delayed_2B,
    pass_DTT_L1Topo_delayed_2B,
    pass_DTT_2B,
    pass_DBT_2B,
    pass_SR_2B,

    pass_STT_1B,
    pass_DTT_2016_1B,
    pass_DTT_4J12_1B,
    pass_DTT_L1Topo_1B,
    pass_DTT_4J12_delayed_1B,
    pass_DTT_L1Topo_delayed_1B,
    pass_DTT_1B,
    pass_DBT_1B,
    pass_SR_1B,

    pass_baseline_LepHad,
    pass_baseline_HadHad,
    pass_LepHad_2B,
    pass_HadHad_2B,
    pass_LepHad_1B,
    pass_HadHad_1B,
    pass_LepHad,
    pass_HadHad,

    pass_ZCR,
    pass_TopEMuCR,
    pass_AntiIsoLepHad,
    pass_Boosted
  };

  // working points used for selecting leptons:
  enum LepSelWpDeco
  {
    loose_iso,
    tight_noniso,
    tight_iso,
  };
}

#endif
