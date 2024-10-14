//
// Created by Yulei on 2024/9/12.
//

#include "SubChannelClassify.h"

namespace MULTILEPTON
{

  using std::get;

  SubChannelClassify::SubChannelClassify(
      const xAOD::MuonContainer *muons,
      const xAOD::ElectronContainer *electrons,
      const xAOD::TauJetContainer *taus,
      const ConstDataVector<xAOD::JetContainer> *bjets,
      const xAOD::EventInfo *event)
  {
    m_event = event;
    m_muons = muons;
    m_electrons = electrons;
    m_taus = taus;
    m_bjets = bjets;

    m_n_bjets = m_bjets->size();
    m_n_electrons = m_electrons->size();
    m_n_muons = m_muons->size();
    m_n_leptons = m_n_electrons + m_n_muons;
    m_n_taus = m_taus->size();

    for (int i = 0; i < m_n_electrons; i++)
    {
      const xAOD::Electron *ele = electrons->at(i);
      m_leptons.emplace_back(i, -11 * ele->charge(), ele);
    }

    for (int i = 0; i < m_n_muons; i++)
    {
      const xAOD::Muon *mu = muons->at(i);
      m_leptons.emplace_back(i, -13 * mu->charge(), mu);
    }

    std::sort(m_leptons.begin(), m_leptons.end(),
              [](const std::tuple<int, int, const xAOD::IParticle *> &a,
                 const std::tuple<int, int, const xAOD::IParticle *> &b)
              { return get<2>(a)->pt() > get<2>(b)->pt(); });

    // Compute charge
    m_total_charge_lep = std::accumulate(
        m_leptons.begin(), m_leptons.end(), 0,
        [](int sum, const std::tuple<int, int, const xAOD::IParticle *> &lep)
        { return sum + -1 * ((get<1>(lep) > 0) - (get<1>(lep) < 0)); });
    m_total_charge_tau = std::accumulate(taus->begin(), taus->end(), 0,
                                         [](int sum, const xAOD::TauJet *tau)
                                         { return sum + tau->charge(); });

    m_sub_channel_id = classify_id();
    fill_lepton_pairs();

    if (m_sub_channel_id == CH_ID::hhbb4l)
    {
      m_sub_channel_flavor = static_cast<int>(classify_flavor_4L());
    }
    else if (m_sub_channel_id == CH_ID::hh3l || m_sub_channel_id == CH_ID::hh3l1tau)
    {
      reorder_3l();
      m_sub_channel_flavor = static_cast<int>(classify_flavor_3L());
    }
    else if (m_sub_channel_id == CH_ID::hh2lsc || m_sub_channel_id == CH_ID::hh2l2tau ||
             m_sub_channel_id == CH_ID::hh2lsc1tau)
    {
      m_sub_channel_flavor = static_cast<int>(classify_flavor_2L());
    }
    else if (m_sub_channel_id == CH_ID::hh1l2tau || m_sub_channel_id == CH_ID::hh1l3tau)
    {
      m_sub_channel_flavor = static_cast<int>(classify_flavor_1L());
    }
    else
    {
      m_sub_channel_flavor = -1;
    }

  }

  CH_ID SubChannelClassify::classify_id()
  {
    if (m_n_bjets >= 1 && m_n_bjets <= 3 && m_n_leptons == 4 && m_n_taus == 0 &&
        m_total_charge_lep == 0)
    {
      return CH_ID::hhbb4l;
    }
    else if (m_n_taus == 0)
    {
      if (m_n_leptons == 3 && std::abs(m_total_charge_lep) == 1)
        return CH_ID::hh3l;
      else if (m_n_leptons == 2 && std::abs(m_total_charge_lep) == 2)
        return CH_ID::hh2lsc;
    }
    else if (m_n_taus > 0)
    {
      if (m_n_leptons == 3 && std::abs(m_total_charge_lep) == 1 && m_n_taus == 1)
        return CH_ID::hh3l1tau;
      else if (m_n_leptons == 2 && m_n_taus == 2)
        return CH_ID::hh2l2tau;
      else if (m_n_leptons == 1 && m_n_taus == 2)
        return CH_ID::hh1l2tau;
      else if (m_n_leptons == 2 && std::abs(m_total_charge_lep) == 2 &&
               m_n_taus == 1)
        return CH_ID::hh2lsc1tau;
      else if (m_n_leptons == 1 && m_n_taus == 3)
        return CH_ID::hh1l3tau;
    }

    return CH_ID::unknown;
  }

  void SubChannelClassify::reorder_3l()
  {
    // For 3l and 3l1tau, reorder the leptons by charge and deltaR
    // lepton 0 is the opposite sign lepton.
    // lepton 1 is the same sign lepton closer to lepton 0.
    // lepton 2 is the remaining lepton.

    // find the opposite sign lepton and move it to index 0
    for (auto& lep : m_leptons)
    {
      if (get<1>(lep) * m_total_charge_lep < 0)
      {
        std::swap(m_leptons[0], lep);
        break;
      }
    }

    // find the same sign lepton closer to lepton 0
    if (get<2>(m_leptons[1])->p4().DeltaR(get<2>(m_leptons[0])->p4()) >
        get<2>(m_leptons[2])->p4().DeltaR(get<2>(m_leptons[0])->p4()))
    {
      std::swap(m_leptons[1], m_leptons[2]);
    }
  }

  void SubChannelClassify::fill_lepton_pairs()
  {
    if (m_n_leptons < 2)
      return;
    for (int i = 0; i < m_n_leptons; i++)
    {
      for (int j = i + 1; j < m_n_leptons; j++)
      {
        auto same_flavor =
            std::abs(get<1>(m_leptons[i])) == std::abs(get<1>(m_leptons[j]));
        auto same_charge = get<1>(m_leptons[i]) * get<1>(m_leptons[j]) > 0;

        if (same_flavor && !same_charge)
          m_lep_SFOC.emplace_back(get<2>(m_leptons[i]), get<2>(m_leptons[j]));
        else if (same_flavor && same_charge)
          m_lep_SFSC.emplace_back(get<2>(m_leptons[i]), get<2>(m_leptons[j]));
        else if (!same_flavor)
          m_lep_DF.emplace_back(get<2>(m_leptons[i]), get<2>(m_leptons[j]));
      }
    }
  }

  bool SubChannelClassify::check_lep_pT(float min_pT) const
  {
    return std::all_of(
        m_leptons.begin(), m_leptons.end(),
        [min_pT](const std::tuple<int, int, const xAOD::IParticle *> &lep)
        { return get<2>(lep)->pt() > min_pT; });
  }
  bool SubChannelClassify::check_low_mass(float min_m) const
  {

    bool SFOC =
        m_lep_SFOC.empty() ||
        std::all_of(
            m_lep_SFOC.begin(), m_lep_SFOC.end(),
            [min_m](const std::pair<const xAOD::IParticle *,
                                    const xAOD::IParticle *> &pair)
            { return (pair.first->p4() + pair.second->p4()).M() > min_m; });

    bool SFSC =
        m_lep_SFSC.empty() ||
        std::all_of(
            m_lep_SFSC.begin(), m_lep_SFSC.end(),
            [min_m](const std::pair<const xAOD::IParticle *,
                                    const xAOD::IParticle *> &pair)
            { return (pair.first->p4() + pair.second->p4()).M() > min_m; });

    bool DF =
        m_lep_DF.empty() ||
        std::all_of(
            m_lep_DF.begin(), m_lep_DF.end(),
            [min_m](const std::pair<const xAOD::IParticle *,
                                    const xAOD::IParticle *> &pair)
            { return (pair.first->p4() + pair.second->p4()).M() > min_m; });

    return SFOC && SFSC && DF;
  }

  FLAVOR_1L SubChannelClassify::classify_flavor_1L() {
    if (m_n_electrons == 1)
      return FLAVOR_1L::e;
    else if (m_n_muons == 1)
      return FLAVOR_1L::m;
    else
      return FLAVOR_1L::unknown;
  }

  FlAVOR_2L SubChannelClassify::classify_flavor_2L() {
    if (m_n_electrons == 2)
      return FlAVOR_2L::ee;
    else if (m_n_muons == 2)
      return FlAVOR_2L::mm;
    else if (m_n_electrons == 1 && m_n_muons == 1)
      return m_electrons->at(0)->pt() > m_muons->at(0)->pt() ? FlAVOR_2L::em : FlAVOR_2L::me;
    else
      return FlAVOR_2L::unknown;
  }

  FLAVOR_3L SubChannelClassify::classify_flavor_3L() {

    auto flavor_l0 = std::abs(get<1>(m_leptons[0]));
    auto flavor_l1 = std::abs(get<1>(m_leptons[1]));

    if (m_n_electrons == 3)
      return FLAVOR_3L::eee;
    else if (m_n_muons == 3)
      return FLAVOR_3L::mmm;
    else if (m_n_electrons == 2 && flavor_l0 == 11 && flavor_l1 == 11 )
      return FLAVOR_3L::eem;
    else if (m_n_electrons == 2 && flavor_l0 == 11 && flavor_l1 == 13 )
      return FLAVOR_3L::eme;
    else if (m_n_electrons == 2 && flavor_l0 == 13)
      return FLAVOR_3L::mee;
    else if (m_n_muons == 2 && flavor_l0 == 13 && flavor_l1 == 13)
      return FLAVOR_3L::mme;
    else if (m_n_muons == 2 && flavor_l0 == 13 && flavor_l1 == 11)
      return FLAVOR_3L::mem;
    else if (m_n_muons == 2 && flavor_l0 == 11)
      return FLAVOR_3L::emm;
    else
      return FLAVOR_3L::unknown;
  }

  FLAVOR_4L SubChannelClassify::classify_flavor_4L() {
    if (m_n_electrons == 4)
      return FLAVOR_4L::eeee;
    else if (m_n_muons == 4)
      return FLAVOR_4L::mmmm;
    else if (m_n_electrons == 2 && m_n_muons == 2)
      return m_electrons->at(0)->pt() > m_muons->at(0)->pt() ? FLAVOR_4L::eemm : FLAVOR_4L::mmee;
    else
      return FLAVOR_4L::unknown;
  }

} // namespace MULTILEPTON