#ifndef EVENT_INFO_WRITER_CONFIG
#define EVENT_INFO_WRITER_CONFIG

#include <string>
#include <vector>

struct Primitive {
  enum class Type {UCHAR,CHAR,UINT,INT,ULL,HALF,FLOAT,DOUBLE};
  Type type;
  std::string source;
  std::string target;
};


struct EventInfoWriterConfig
{
  std::string name;
  std::vector<Primitive> inputs;
};

#endif
