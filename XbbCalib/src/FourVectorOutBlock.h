#ifndef FOUR_VECTOR_OUT_BLOCK_H
#define FOUR_VECTOR_OUT_BLOCK_H

#include <SystematicsHandles/SysWriteDecorHandle.h>

#include "TLorentzVector.h"
#include <string>
#include <memory>

namespace Gaudi {
  class Algorithm;
}
namespace CP {
  template <typename T> class SysReadHandle;
  class SystematicSet;
}
namespace xAOD {
  class EventInfo_v1;
  using EventInfo = EventInfo_v1;
  class IParticle;
}


namespace XBBCALIB
{
  class FourVectorOutBlock
  {
  public:
    FourVectorOutBlock(
      Gaudi::Algorithm* owner,
      const std::string& prefix,
      CP::SysListHandle& syst_list,
      CP::SysReadHandle<xAOD::EventInfo>& event_handle);
    void set(const xAOD::EventInfo& event, const xAOD::IParticle& part,
             const CP::SystematicSet& sys);
     void set(const xAOD::EventInfo& event, const TLorentzVector &lv,
             const CP::SystematicSet& sys);
    void setDefault(const xAOD::EventInfo& event,
                    const CP::SystematicSet& sys);
  private:
    CP::SysWriteDecorHandle<float> m_pt;
    CP::SysWriteDecorHandle<float> m_eta;
    CP::SysWriteDecorHandle<float> m_phi;
    CP::SysWriteDecorHandle<float> m_m;
  };
}

#endif
