/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/


// Always protect against multiple includes!
#ifndef XBBCALIB_ENUMS
#define XBBCALIB_ENUMS

namespace XBBCALIB
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
	  // PASS_TRIGGER,
	  PASS_EXACTLY_ONE_PHOTON,
	  PASS_TWO_SF_LEPTONS,
  };
}

#endif
