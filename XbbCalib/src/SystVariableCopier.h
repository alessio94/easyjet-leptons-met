#ifndef SYSTVARIABLECOPIER_H
#define SYSTVARIABLECOPIER_H

#include "DefaultOutputs.h"

namespace XBBCALIB {

  // Each SystVariableCopier can copy one type T of variable. Likely
  // choices are float and int.
  template <typename T>
  class SystVariableCopier
  {
  private:
    using SRDH_t = CP::SysReadDecorHandle<T>;
    using SWDH_t = CP::SysWriteDecorHandle<T>;
    using rw_pair_t = std::pair<SRDH_t, SWDH_t>;
    std::vector<std::unique_ptr<rw_pair_t>> m_copy_pairs;
  public:
    template <typename I, typename O>
    SystVariableCopier(Gaudi::Algorithm* owner,
                       const std::vector<std::string>& range,
                       CP::SysListHandle& syst_list,
                       I& input,
                       O& output,
                       const std::string& prefix = "")
    {
      for (const auto& var: range) {
        auto& rwpair = m_copy_pairs.emplace_back(
          std::make_unique<rw_pair_t>(
            SRDH_t{var, owner},
            SWDH_t{prefix + var + "_%SYS%", owner})
          );
        auto in_rc = rwpair->first.initialize(syst_list, input);
        auto out_rc = rwpair->second.initialize(syst_list, output);
        if (in_rc.isFailure() || out_rc.isFailure()) {
          throw std::logic_error("initialization failure");
        }
      }
    }
    template <typename O, typename I>
    void set(const O& out, const I& in, const CP::SystematicSet& sys) {
      for (const auto& pair: m_copy_pairs) {
        pair->second.set(out, pair->first.get(in, sys), sys);
      }
    }
    template <typename O>
    void setDefault(const O& out, const CP::SystematicSet& sys) {
      for (const auto& pair: m_copy_pairs) {
        pair->second.set(out, defaults::get_default<T>(), sys);
      }
    }
  };

}

#endif
