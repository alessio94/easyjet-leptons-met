/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJETPLUS_UTILITIES
#define EASYJETPLUS_UTILITIES

namespace PostProc
{
  enum VarType
    {
      Int = 0,
      Long = 1,
      Float = 2
    };

  struct varTypePointer{
    VarType type;
    void* pointer;
  };

}

#endif
