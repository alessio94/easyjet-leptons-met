#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <string>

struct Primitive {
  enum class Type {
    PRECISION_CUSTOM,
    CUSTOM,
    UCHAR,
    CHAR,
    USHORT,
    SHORT,
    UINT,
    INT,
    ULL,
    HALF,
    FLOAT,
    DOUBLE,
    UINT2UCHAR,
    INT2CHAR,
    UINT2USHORT,
    INT2SHORT
  };
  Type type;
  std::string source;
  std::string target;
};

#endif
