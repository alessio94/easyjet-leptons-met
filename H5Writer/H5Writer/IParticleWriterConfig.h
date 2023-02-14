#ifndef IPARTICLE_WRITER_CONFIG_H
#define IPARTICLE_WRITER_CONFIG_H

#include "Primitive.h"

#include <string>
#include <vector>

struct AssociatedPrimitive
{
  std::string link_name;
  Primitive input;
};

struct IParticleWriterConfig
{
  std::string name;
  unsigned long long maximum_size;
  std::vector<AssociatedPrimitive> inputs;
};

#endif
