#include "DiHiggsAnalysisHelpers.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <AthContainers/ConstDataVector.h>
#include <SystematicsHandles/SysReadHandle.h>

namespace HH4B
{
  std::vector<const xAOD::TruthParticle *> getTruthBs()
  {
    SG::ReadHandle<xAOD::TruthParticleContainer> truthBosons(
        "TruthBosonsWithDecayParticles");
    SG::ReadHandle<xAOD::TruthParticleContainer> truthBSM("TruthBSM");
    // collect Higgs and Scalars
    ConstDataVector<xAOD::TruthParticleContainer> truthInitialParticles(
        SG::VIEW_ELEMENTS);

    for (const xAOD::TruthParticle *boson : *truthBosons)
    {
      // get Higgs
      if (boson->pdgId() == 25)
      {
        truthInitialParticles.push_back(boson);
      }
    }
    for (const xAOD::TruthParticle *bsm : *truthBSM)
    {
      // get Scalar
      if (bsm->pdgId() == 35)
      {
        truthInitialParticles.push_back(bsm);
      }
    }

    std::vector<const xAOD::TruthParticle *> truthBs;
    for (const xAOD::TruthParticle *tp : truthInitialParticles)
    {
      // get the b quark children
      for (size_t i = 0; i < tp->nChildren(); i++)
      {
        const xAOD::TruthParticle *thisChild = tp->child(i);
        if (thisChild->absPdgId() == 5)
        {
          truthBs.push_back(thisChild);
        }
      }
    }

    return truthBs;
  }

  closestB getClosestB(const xAOD::Jet *jet,
                       std::vector<const xAOD::TruthParticle *> truthBs)
  {
    // closestB has members: particle and dR
    closestB b;
    static const SG::AuxElement::Accessor<std::vector<float>> dRtoTruthBs_acc(
        "dRtoTruthBs");
    std::vector<float> dRtoTruthBs = dRtoTruthBs_acc(*jet);
    std::vector<float>::iterator it =
        std::min_element(std::begin(dRtoTruthBs), std::end(dRtoTruthBs));
    int closestBindex = std::distance(std::begin(dRtoTruthBs), it);
    b.particle = truthBs[closestBindex];
    b.dR = dRtoTruthBs[closestBindex];

    return b;
  }
}