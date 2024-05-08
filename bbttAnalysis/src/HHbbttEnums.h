#ifndef BBTTANALYSIS_ENUMS
#define BBTTANALYSIS_ENUMS

namespace HHBBTT
{

  enum Channel
  {
    LepHad = 0,
    HadHad = 1,
    LepHad1B = 2,
    HadHad1B = 3,
    ZCR = 4,
    TopEMuCR = 5,
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
  };

}

#endif
