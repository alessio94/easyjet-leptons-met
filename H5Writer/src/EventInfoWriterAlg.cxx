#include "src/EventInfoWriterAlg.h"
#include "H5Writer/EventInfoWriterConfig.h"

#include "H5Cpp.h"

namespace {

#define CHECK_TYPE(string, target)                      \
  if (string == #target) return Primitive::Type::target
  Primitive::Type getPrimitiveType(const std::string& name) {
    CHECK_TYPE(name, UCHAR);
    CHECK_TYPE(name, CHAR);
    CHECK_TYPE(name, UINT);
    CHECK_TYPE(name, INT);
    CHECK_TYPE(name, ULL);
    CHECK_TYPE(name, HALF);
    CHECK_TYPE(name, FLOAT);
    CHECK_TYPE(name, DOUBLE);
    throw std::domain_error("unknown type " + name);
  }
#undef CHECK_TYPE

}


EventInfoWriterAlg::EventInfoWriterAlg(const std::string& name,
                                       ISvcLocator* loc):
  AthAlgorithm(name, loc),
  m_writer(nullptr)
{
}

StatusCode EventInfoWriterAlg::initialize() {
  ATH_CHECK(m_infoKey.initialize());
  ATH_CHECK(m_output_svc.retrieve());

  EventInfoWriterConfig cfg;
  cfg.name = "event";
  for (const std::string& prim: m_primitives) {
    if (!m_primToType.value().count(prim)) {
      ATH_MSG_ERROR(prim << " not specified in type mapping");
    }
    std::string type = m_primToType.value().at(prim);
    cfg.inputs.push_back(Primitive{getPrimitiveType(type), prim, prim});
  }
  m_writer.reset(new EventInfoWriter(*m_output_svc->file(), cfg));

  return StatusCode::SUCCESS;
}

StatusCode EventInfoWriterAlg::execute() {
  SG::ReadHandle event_info(m_infoKey);
  ATH_CHECK(event_info.isValid());
  m_writer->fill(*event_info);
  return StatusCode::SUCCESS;
}

StatusCode EventInfoWriterAlg::finalize() {
  m_writer->flush();
  return StatusCode::SUCCESS;
}
