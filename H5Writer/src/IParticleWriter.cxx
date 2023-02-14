#include "H5Writer/IParticleWriter.h"
#include "H5Writer/IParticleWriterConfig.h"

#include "PrimitiveHelpers.h"

#include "xAODBase/IParticle.h"
#include "HDF5Utils/Writer.h"

// needed concrete classes for ElementLink access
#include "xAODBTagging/BTaggingContainer.h"

namespace {
  using In_t = IParticleWriter::Writer_t::input_type;
  using Consumer_t = IParticleWriter::Writer_t::consumer_type;


  template <typename A=detail::defaultAccessor_t<Consumer_t>>
  void addCustomType(Consumer_t& c,
                     const Primitive& p,
                     A a=detail::defaultAccessor<Consumer_t>) {
    using I = In_t;

    if (!detail::isCustom(p)) {
      throw std::logic_error("called addCustomType on non-custom type");
    }
    const auto h = (p.type == Primitive::Type::PRECISION_CUSTOM) ?
      H5Utils::Compression::STANDARD :
      H5Utils::Compression::HALF_PRECISION;

    const std::string s = p.source;
    if (s == "pt") {
      c.add<float>(s, [a](I in) {return a(in)->pt(); }, NAN);
    } else if (s == "eta") {
      c.add<float>(s, [a](I in) {return a(in)->eta(); }, NAN, h);
    } else if (s == "phi") {
      c.add<float>(s, [a](I in) {return a(in)->phi(); }, NAN, h);
    } else if (s == "mass") {
      c.add<float>(s, [a](I in) {return a(in)->m(); }, NAN);
    } else if (s == "ptGeV") {
      c.add<float>(s, [a](I in) {return a(in)->pt()*0.001; }, NAN, h);
    } else if (s == "massGeV") {
      c.add<float>(s, [a](I in) {return a(in)->m()*0.001; }, NAN, h);
    } else if (s == "valid") {
      c.add<bool>(s, [](I) {return true; }, false);
    } else {
      throw std::logic_error(s + " isn't a known custom primitive");
    }
  }


  // stuff to retrieve associated links
  template <typename T, typename R=SG::AuxElement>
  class LinkGetter
  {
  public:
    LinkGetter(std::string name);
    const R* operator()(In_t in) const;
  private:
    using LinkAccessor = SG::AuxElement::ConstAccessor<ElementLink<T>>;
    const LinkAccessor m_accessor;
    const std::string m_linkName;
  };
  template <typename T, typename R>
  LinkGetter<T,R>::LinkGetter(std::string name):
    m_accessor(name),
    m_linkName(name)
  {}
  template <typename T, typename R>
  const R* LinkGetter<T,R>::operator()(In_t in) const {
    auto elink = m_accessor(*in);
    if (!elink.isValid()) {
      throw std::runtime_error("invalid link " + m_linkName);
    }
    return *elink;
  }
}


IParticleWriter::IParticleWriter(
  H5::Group& group,
  const IParticleWriterConfig& cfg)
{
  using input_type = Writer_t::input_type;
  using IPC = xAOD::IParticleContainer;
  using IP = xAOD::IParticle;
  Writer_t::consumer_type c;
  for (const auto& input: cfg.inputs) {
    if (input.link_name.empty()) {
      const auto& primitive = input.input;
      if (detail::isCustom(primitive.type)) {
        addCustomType(c, primitive);
      } else {
        detail::addInput(c, primitive);
      }
    } else {
      // else we have some association to follow
      std::string n = input.link_name;
      // unfortunately we need a special case for b-tagging
      if (n == "btaggingLink") {
        LinkGetter<xAOD::BTaggingContainer> getter(n);
        detail::addInput(c, input.input, getter);
      } else {
        // everything else is an IParticle
        LinkGetter<IPC,IP> getter(n);
        if (detail::isCustom(input.input.type)) {
          addCustomType(c, input.input, getter);
        } else {
          detail::addInput(c, input.input, getter);
        }
      }
    }
  }
  m_writer = std::make_unique<Writer_t>(
    group, cfg.name, c, std::array{cfg.maximum_size});
}


IParticleWriter::~IParticleWriter() = default;


void IParticleWriter::fill(const std::vector<const xAOD::IParticle*>& info) {
  m_writer->fill(info);
}


void IParticleWriter::flush() {
  m_writer->flush();
}
