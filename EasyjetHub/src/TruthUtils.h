/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthUtils.h
//
// This is a collection of useful functions for Truth algorithms
//
// Author: Jordy Degens, Osama Karkout
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HHANALYSIS_TRUTHUTILS
#define HHANALYSIS_TRUTHUTILS

#include <xAODTruth/TruthParticleContainer.h>


namespace Easyjet
{
  const xAOD::TruthParticle *
  getFinalParticleOfType(
      const xAOD::TruthParticle *p, const std::unordered_set<int> ids);
}

#endif