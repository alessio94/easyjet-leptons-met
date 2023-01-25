#include "H5Writer/EventInfoWriter.h"
#include "H5Writer/EventInfoWriterConfig.h"

#include "xAODEventInfo/EventInfo.h"
#include "HDF5Utils/Writer.h"

// helper functions
namespace {
  // build accessor
  template <typename T, typename S=T>
  std::function<T(EventInfoWriter::Writer_t::input_type)> makeGetter(
    std::string source) {
    SG::AuxElement::ConstAccessor<S> acc(source);
    return [acc](EventInfoWriter::Writer_t::input_type in) {
      return acc(in);
    };
  }

}

EventInfoWriter::EventInfoWriter(
  H5::Group& group,
  const EventInfoWriterConfig& cfg)
{
  Writer_t::consumer_type c;
  using Tp = Primitive::Type;
  for (const auto& input: cfg.inputs) {
    std::string s = input.source;
    std::string t = input.target.empty() ? input.source: input.target;
    const auto h = H5Utils::Compression::HALF_PRECISION;
    switch (input.type) {
    case Tp::UCHAR: c.add(t, makeGetter<unsigned char>(s), char(0)); break;
    case Tp::CHAR: c.add(t, makeGetter<char>(s), char(-1)); break;
    case Tp::UINT: c.add(t, makeGetter<unsigned int>(s), 0); break;
    case Tp::INT: c.add(t, makeGetter<int>(s), -1); break;
    case Tp::ULL: c.add(t, makeGetter<unsigned long long>(s),0); break;
    case Tp::HALF: c.add(t, makeGetter<float>(s), NAN, h); break;
    case Tp::FLOAT: c.add(t, makeGetter<float>(s), NAN); break;
    case Tp::DOUBLE: c.add(t, makeGetter<double>(s), NAN); break;
    }
  }
  m_writer = std::make_unique<Writer_t>(group, cfg.name, c);
}

EventInfoWriter::~EventInfoWriter() = default;

void EventInfoWriter::fill(const xAOD::EventInfo& info) {
  m_writer->fill(info);
}

void EventInfoWriter::flush() {
  m_writer->flush();
}
