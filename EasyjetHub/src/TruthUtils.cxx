#include "TruthUtils.h"


namespace Easyjet
{
  const xAOD::TruthParticle *
  getFinalParticleOfType(
      const xAOD::TruthParticle *p, const std::unordered_set<int> & ids)
  {
    for (size_t i = 0; i < p->nChildren(); i++)
    {
      if (std::find(ids.begin(), ids.end(), p->child(i)->pdgId()) != ids.end())
      {
        return getFinalParticleOfType(p->child(i), ids);
      }
    }
    return p;
  }
}
