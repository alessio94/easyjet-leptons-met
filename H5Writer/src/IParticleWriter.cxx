#include "H5Writer/IParticleWriter.h"
#include "H5Writer/IParticleWriterConfig.h"

#include "PrimitiveHelpers.h"

#include "xAODBase/IParticle.h"
#include "HDF5Utils/Writer.h"

// needed concrete classes for ElementLink access
#include "xAODBTagging/BTaggingContainer.h"

namespace {
  template <unsigned int N>
  using Writer_t = H5Utils::Writer<N, const xAOD::IParticle*>;
  using In_t = Writer_t<1>::input_type;
  using Consumer_t = Writer_t<1>::consumer_type;
  using CountWriter_t = H5Utils::Writer<0, unsigned char>;

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

    // check for match
    bool m = false;

    // these do most of the matching work
    auto match = [&c, s, a, &m, h](const std::string& n, auto func) {
      if (s == n) {
        c.add<float>(s, [a, func](I in) { return func(a(in)); }, NAN, h);
        m = true;
      }
    };
    auto matchGeV = [&c, s, a, &m, h](const std::string& n, auto func) {
      if (s == n) {
        if (h == H5Utils::Compression::STANDARD) {
          throw std::logic_error(
            "asked for a full precision version of a variable that can"
            " not be stored at half precision: " + s);
        }
        c.add<float>(s, [a, func](I in) {return func(a(in)); }, NAN);
        m = true;
      } else if (s == n + "GeV") {
        c.add(s, [a, func](I in) { return func(a(in))*0.001; }, NAN, h);
        m = true;
      }
    };

    // match cases
    matchGeV("pt", [](auto in) {return in->pt(); });
    match("eta", [](auto in) {return in->eta(); });
    match("phi", [](auto in) {return in->phi(); });
    matchGeV("px", [](auto in) {return in->p4().Px(); });
    matchGeV("py", [](auto in) {return in->p4().Py(); });
    matchGeV("pz", [](auto in) {return in->p4().Pz(); });
    matchGeV("mass", [](auto in) {return in->m(); });

    if (m) return;

    if (s == "valid") {
      c.add<bool>(s, [](I) {return true; }, false);
      return;
    }
    throw std::logic_error("unknow known custom primitive: " + s);
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

  CountWriter_t::consumer_type getOffsetConsumer()
  {
    using CountIn_t = CountWriter_t::input_type;
    CountWriter_t::consumer_type c;
    c.add<CountIn_t>("count", [](CountIn_t i) { return i; });
    return c;
  }

}


// IParticleWriter implementaitons
namespace details {
  class IParticleWriterBase
  {
  public:
    virtual ~IParticleWriterBase() = default;
    virtual void fill(const std::vector<const xAOD::IParticle*>& info) = 0;
    virtual void flush() = 0;
  };
}
namespace {
  // implementation for writer for 2d arrays
  class IParticle2dWriter: public details::IParticleWriterBase
  {
  private:
    Writer_t<1> m_writer;
  public:
    IParticle2dWriter(H5::Group& group,
                      const std::string& n,
                      Consumer_t c,
                      long long unsigned size):
      m_writer(group, n, c, {{size}}) {}
    ~IParticle2dWriter() = default;

    void fill(const std::vector<In_t>& v) override {
      m_writer.fill(v);
    }
    void flush() override {
      m_writer.flush();
    }
  };

  // implementation for writer for awkward arrays
  class IParticleAwkwardWriter: public details::IParticleWriterBase
  {
  private:
    H5::Group m_group;
    Writer_t<0> m_writer;
    CountWriter_t m_counts;
  public:
    IParticleAwkwardWriter(H5::Group& parent,
                           const std::string& n,
                           Consumer_t c):
      m_group(parent.createGroup(n)),
      m_writer(m_group, "raw", c),
      m_counts(m_group, "counts", getOffsetConsumer())
      {}
    ~IParticleAwkwardWriter() = default;

    void fill(const std::vector<In_t>& v) override {
      using Count_t = CountWriter_t::input_type;
      constexpr auto max = std::numeric_limits<Count_t>::max();
      auto n_entries = v.size();
      if (n_entries > max) {
        throw std::overflow_error(
          "number of entries exceeds maximum for this datatype "
          "[" + std::to_string(n_entries) + " > " + std::to_string(max) + "]"
          );
      }
      for (const auto& e: v) m_writer.fill(e);
      m_counts.fill(n_entries);
    }
    void flush() override {
      m_writer.flush();
      m_counts.flush();
    }
  };

}


IParticleWriter::IParticleWriter(
  H5::Group& group,
  const IParticleWriterConfig& cfg)
{
  using input_type = In_t;
  using IPC = xAOD::IParticleContainer;
  using IP = xAOD::IParticle;
  Consumer_t c;
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
  if (cfg.maximum_size == 0) {
    m_writer = std::make_unique<IParticleAwkwardWriter>(
      group, cfg.name, c);
  } else {
    m_writer = std::make_unique<IParticle2dWriter>(
      group, cfg.name, c, cfg.maximum_size);
  }
}


IParticleWriter::~IParticleWriter() = default;


void IParticleWriter::fill(const std::vector<In_t>& info) {
  m_writer->fill(info);
}


void IParticleWriter::flush() {
  m_writer->flush();
}
