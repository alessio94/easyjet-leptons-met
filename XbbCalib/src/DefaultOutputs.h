#ifndef DEFAULT_OUTPUTS_H
#define DEFAULT_OUTPUTS_H

#include <concepts>

namespace XBBCALIB::defaults {
  // A valid float can take on pretty much any value in (-inf, inf),
  // NAN is usually the obvious "wrong" value.
  const float def_float = NAN;

  // Int is often used for counting, in which case -1 isn't a bad
  // default.
  const int def_int = -1;

  // for unsigned values this is a lot less clear, but it shouldn't be -1
  const unsigned int def_unsigned_int = 0;

  // Overloaded templates to get defaults for a generic type T
  template <std::signed_integral T>
  auto get_default() {
    return def_int;
  }
  template <std::unsigned_integral T>
  auto get_default() {
    return def_unsigned_int;
  }
  template <std::floating_point T>
  auto get_default() {
    return def_float;
  }

}

#endif
