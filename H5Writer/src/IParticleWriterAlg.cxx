#include "src/IParticleWriterAlg.h"
#include "H5Writer/IParticleWriterConfig.h"

#include "AlgHelpers.h"

#include "H5Cpp.h"

IParticleWriterAlg::IParticleWriterAlg(const std::string& name,
                                       ISvcLocator* loc):
  AthAlgorithm(name, loc),
  m_writer(nullptr)
{
}

StatusCode IParticleWriterAlg::initialize() {
  ATH_CHECK(m_partKey.initialize());
  ATH_CHECK(m_output_svc.retrieve());

  IParticleWriterConfig cfg;
  cfg.name = m_dsName.value();
  cfg.maximum_size = m_maxSize.value();
  if (cfg.name.empty()) {
    ATH_MSG_ERROR("datasetName isn't specified in particle writer");
    return StatusCode::FAILURE;
  }
  for (const std::string& prim: m_primitives) {
    if (!m_primToType.value().count(prim)) {
      ATH_MSG_ERROR(prim << " not specified in type mapping");
      return StatusCode::FAILURE;
    }
    auto type = getPrimitiveType(m_primToType.value().at(prim));
    Primitive newprim{type, prim, prim};
    std::string link_name = "";
    if (m_primToAssociation.value().count(prim)) {
      link_name = m_primToAssociation.value().at(prim);
    }
    cfg.inputs.push_back(AssociatedPrimitive{link_name,newprim});
  }
  m_writer.reset(new IParticleWriter(*m_output_svc->file(), cfg));

  return StatusCode::SUCCESS;
}

StatusCode IParticleWriterAlg::execute() {
  SG::ReadHandle ipc(m_partKey);
  std::vector<const xAOD::IParticle*> parts(ipc->begin(), ipc->end());
  m_writer->fill(parts);
  return StatusCode::SUCCESS;
}

StatusCode IParticleWriterAlg::finalize() {
  m_writer->flush();
  return StatusCode::SUCCESS;
}
