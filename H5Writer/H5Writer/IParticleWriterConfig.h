#ifndef IPARTICLE_WRITER_CONFIG_H
#define IPARTICLE_WRITER_CONFIG_H

#include "Primitive.h"

#include <string>
#include <vector>

struct AssociatedPrimitive
{
  // name of ElementLink to primative, empty means the primative is on
  // our IParticle
  std::string link_name;
  Primitive input;
};

struct IParticleWriterConfig
{
  // name of the dataset to write
  std::string name;

  // maximum size of 2d array to write, in other words the maximum
  // particles per event
  //
  // NOTE: a special value of zero saves an awkward array
  // representation (one dataset of raw jets, another specifying the
  // offsets)
  unsigned long long maximum_size;

  std::vector<AssociatedPrimitive> inputs;
};

#endif
