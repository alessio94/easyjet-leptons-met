#ifndef HH4B_DIHIGGSANALYSISHELPERS
#define HH4B_DIHIGGSANALYSISHELPERS

#include "DiHiggsAnalysis.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace HH4B
{

  // get truth b quarks if we have MC
  std::vector<const xAOD::TruthParticle *> getTruthBs();

  // find the closest b quark and return a struct holding the
  // xAOD::TruthParticle and its dR to the jet
  closestB getClosestB(const xAOD::Jet *jet,
                       std::vector<const xAOD::TruthParticle *> truthBs);
}
#endif