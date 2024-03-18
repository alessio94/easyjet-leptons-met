#ifndef TAUTRIGGERDECORATOR_HELPER
#define TAUTRIGGERDECORATOR_HELPER

namespace Easyjet
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
    STT,
    DTT,
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