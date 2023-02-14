#ifndef PRIMATIVE_HELPERS_H
#define PRIMATIVE_HELPERS_H

#include "H5Writer/Primitive.h"

#include "HDF5Utils/Writer.h"
#include "AthContainers/AuxElement.h"

#include <stdexcept>

// helper functions
namespace detail {

  bool isCustom(const Primitive&);
  bool isCustom(const Primitive::Type&);

  // build accessor
  template <typename T, typename I, typename A, typename S=T>
  std::function<T(I)> makeGetter(std::string source, A ass) {
    SG::AuxElement::ConstAccessor<S> acc(source);
    using rettype = decltype(ass(std::declval<I>()));
    if constexpr (std::is_pointer<rettype>::value) {
      return [acc, ass](I in) { return acc(*ass(in)); };
    } else {
      return [acc, ass](I in) { return acc(ass(in)); };
    }
  }

  template <typename T>
  auto defaultAccessor = [](typename T::input_type in){ return in; };
  template <typename T>
  using defaultAccessor_t = decltype(defaultAccessor<T>);

  template <typename T, typename A=defaultAccessor_t<T>>
  void addInput(T& c, const Primitive& input, A a=defaultAccessor<T>) {
    using I = typename T::input_type;
    using Tp = Primitive::Type;
    std::string s = input.source;
    std::string t = input.target.empty() ? input.source : input.target;
    const auto h = H5Utils::Compression::HALF_PRECISION;
    const char cz = char(0);
    switch (input.type) {
    case Tp::PRECISION_CUSTOM: // intentional fall-through
    case Tp::CUSTOM: throw std::logic_error("custom type unsupported");
    case Tp::UCHAR: c.add(t, makeGetter<unsigned char,I>(s,a), cz); return;
    case Tp::CHAR: c.add(t, makeGetter<char,I>(s,a), char(-1)); return;
    case Tp::UINT: c.add(t, makeGetter<unsigned int,I>(s,a), 0); return;
    case Tp::INT: c.add(t, makeGetter<int,I>(s,a), -1); return;
    case Tp::ULL: c.add(t, makeGetter<unsigned long long,I>(s,a),0); return;
    case Tp::HALF: c.add(t, makeGetter<float,I>(s,a), NAN, h); return;
    case Tp::FLOAT: c.add(t, makeGetter<float,I>(s,a), NAN); return;
    case Tp::DOUBLE: c.add(t, makeGetter<double,I>(s,a), NAN); return;
    }
  }

}

#endif
