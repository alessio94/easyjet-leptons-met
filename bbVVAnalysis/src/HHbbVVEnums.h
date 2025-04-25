#ifndef BBVVANALYSIS_HHBBVVENUMS
#define BBVVANALYSIS_HHBBVVENUMS

namespace HHBBVV
{

  enum Channel
  {
    Boosted1Lep = 0,
    SplitBoosted1Lep = 1,
    Boosted0Lep = 2,
    SplitBoosted0Lep = 3,
    VBFboosted1Lep = 4,
    VBFsplitboosted1Lep = 5,
  };
  enum Var
  {
    ele = 0,
    mu = 1,
  };

  enum TriggerChannel
  {
    SLT,
  };
    
  enum Booleans
  {
    pass_trigger_SET,
    pass_trigger_SMT,
    pass_trigger_SR,
    pass_trigger_SLT,
    pass_trigger_LRT,
  };

}

#endif
