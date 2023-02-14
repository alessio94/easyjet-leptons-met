#ifndef IPARTICLE_WRITER_H
#define IPARTICLE_WRITER_H

#include <memory>
#include <vector>

namespace H5 {
  class Group;
}
namespace H5Utils {
  template <size_t N, typename I> class Writer;
}
namespace xAOD {
  class IParticle;
}
struct IParticleWriterConfig;

class IParticleWriter {
public:
  using Writer_t = H5Utils::Writer<1, const xAOD::IParticle*>;
  IParticleWriter(H5::Group& output_group, const IParticleWriterConfig&);
  ~IParticleWriter();
  void fill(const std::vector<const xAOD::IParticle*>&);
  void flush();
private:
  std::unique_ptr<Writer_t> m_writer;
};

#endif
