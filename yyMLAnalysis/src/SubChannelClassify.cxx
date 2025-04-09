
#include "SubChannelClassify.h"
#include <numeric>

namespace HHYYML {

  SubChannelClassify::SubChannelClassify(
    const xAOD::ElectronContainer *electrons,
    const xAOD::MuonContainer *muons,
    const xAOD::TauJetContainer *taus,
    const xAOD::EventInfo *event):
    m_electrons(electrons),
    m_muons(muons),
    m_taus(taus),
    m_event(event),
    m_n_electrons(m_electrons->size()),
    m_n_muons(m_muons->size()),
    m_n_leptons(m_n_electrons + m_n_muons),
    m_n_taus(m_taus->size()),
    m_sub_channel_id(classify_id()) {

    // for (int i = 0; i < m_n_electrons; i++) {
    //   const xAOD::Electron *ele = electrons->at(i);
    //   m_leptons.emplace_back(i, -11 * ele->charge(), ele);
    // }

    // for (int i = 0; i < m_n_muons; i++) {
    //   const xAOD::Muon *mu = muons->at(i);
    //   m_leptons.emplace_back(i, -13 * mu->charge(), mu);
    // }

    // // Sort leptons in descending order in pt
    // std::sort(m_leptons.begin(), m_leptons.end(),
    //     [](const std::tuple<int, int, const xAOD::IParticle *> &a,
    //        const std::tuple<int, int, const xAOD::IParticle *> &b)
    //     { return get<2>(a)->pt() > get<2>(b)->pt(); });

    if (m_sub_channel_id == CH_ID::hh1l0tau || m_sub_channel_id == CH_ID::hh1l1tau) {
      m_sub_channel_flavor = static_cast<int>(classify_flavor_1L());
    }
    else if (m_sub_channel_id == CH_ID::hh2l0tau) {
      m_sub_channel_flavor = static_cast<int>(classify_flavor_2L());
    }
    else {
      m_sub_channel_flavor = -1;
    }

  }

  CH_ID SubChannelClassify::classify_id() const {
    if (m_n_leptons == 1 && m_n_taus == 0) {
      return CH_ID::hh1l0tau;
    }
    else if (m_n_leptons == 0 && m_n_taus == 1) {
      return CH_ID::hh0l1tau;
    }
    else if (m_n_leptons == 2 && m_n_taus == 0) {
      return CH_ID::hh2l0tau;
    }
    else if (m_n_leptons == 1 && m_n_taus == 1) {
      return CH_ID::hh1l1tau;
    }
    else if (m_n_leptons == 0 && m_n_taus == 2) {
      return CH_ID::hh0l2tau;
    }
    else {
      return CH_ID::unknown;
    }
  }

  FLAVOR_1L SubChannelClassify::classify_flavor_1L() const {
    if (m_n_electrons == 1)
      return FLAVOR_1L::e;
    else if (m_n_muons == 1)
      return FLAVOR_1L::m;
    else
      return FLAVOR_1L::unknown;
  }

  FLAVOR_2L SubChannelClassify::classify_flavor_2L() const {
    if (m_n_electrons == 2)
      return FLAVOR_2L::ee;
    else if (m_n_muons == 2)
      return FLAVOR_2L::mm;
    else if (m_n_electrons == 1 && m_n_muons == 1)
      return m_electrons->at(0)->pt() > m_muons->at(0)->pt() ? FLAVOR_2L::em : FLAVOR_2L::me;
    else
      return FLAVOR_2L::unknown;
  }

} // namespace HHYYML
