#ifndef DEFAULT_OUTPUTS_H
#define DEFAULT_OUTPUTS_H

#include <concepts>

namespace XBBCALIB::defaults {
  // To do: make this somethig more standard like NAN
  const float def_float = -99;
  // To do: make this something more standard like -1
  const int def_int = -99;

  // Overloaded templates to get defaults for a generic type T
  template <std::integral T>
  auto get_default() {
    return def_int;
  }
  template <std::floating_point T>
  auto get_default() {
    return def_float;
  }

}

#endif
