/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerUtils.h"

namespace VBSVV4q
{

  std::vector<std::string> getSingleLargeRJetsTriggers(int year){

    std::vector<std::string> list;

    if(year == 2015){
      list = {"HLT_j360_a10_lcw_sub_L1J100"};
    }
    else if(2016==year){
      list = {"HLT_j420_a10_lcw_L1J100"};
    }
    else if(2017<=year && year<=2018){
      list = {"HLT_j460_a10t_lcw_jes_L1J100"};
    }

    return list;

  }

}