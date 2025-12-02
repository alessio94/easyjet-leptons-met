#include "FourVectorOutBlock.h"
#include "DefaultOutputs.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODBase/IParticle.h>


namespace XBBCALIB {
  FourVectorOutBlock::FourVectorOutBlock(
    Gaudi::Algorithm* owner,
    const std::string& prefix,
    CP::SysListHandle& syst_list,
    CP::SysReadHandle<xAOD::EventInfo>& event_handle):
    m_pt{prefix + "pt_%SYS%", owner},
    m_eta{prefix + "eta_%SYS%", owner},
    m_phi{prefix + "phi_%SYS%", owner},
    m_m{prefix + "m_%SYS%", owner}
  {
    auto init = [&syst_list, &event_handle] (auto& handle) {
      auto sc = handle.initialize(syst_list, event_handle);
      if (sc.isFailure()) throw std::logic_error("initialization failure");
    };
    init(m_pt);
    init(m_eta);
    init(m_phi);
    init(m_m);
  }

  void FourVectorOutBlock::set(
    const xAOD::EventInfo& event,
    const xAOD::IParticle& part,
    const CP::SystematicSet& sys)
  {
    m_pt.set(event, part.pt(), sys);
    m_eta.set(event, part.eta(), sys);
    m_phi.set(event, part.phi(), sys);
    m_m.set(event, part.m(), sys);
  }

  void FourVectorOutBlock::set(const xAOD::EventInfo &event,
                               const TLorentzVector &lv,
                               const CP::SystematicSet &sys)
  {
    m_pt.set(event, lv.Pt(), sys);
    m_eta.set(event, lv.Eta(), sys);
    m_phi.set(event, lv.Phi(), sys);
    m_m.set(event, lv.M(), sys);
  }
  void FourVectorOutBlock::setDefault(const xAOD::EventInfo& event,
                                      const CP::SystematicSet& sys) {
    using namespace defaults;
    m_pt.set(event, def_float, sys);
    m_eta.set(event, def_float, sys);
    m_phi.set(event, def_float, sys);
    m_m.set(event, def_float, sys);
  }

}
