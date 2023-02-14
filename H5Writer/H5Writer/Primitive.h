#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <string>

struct Primitive {
  enum class Type {
    PRECISION_CUSTOM,
    CUSTOM,
    UCHAR,
    CHAR,
    UINT,
    INT,
    ULL,
    HALF,
    FLOAT,
    DOUBLE
  };
  Type type;
  std::string source;
  std::string target;
};

#endif
