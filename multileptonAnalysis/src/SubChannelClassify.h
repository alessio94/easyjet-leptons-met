//
// Created by Yulei on 2024/9/12.
//

#ifndef EASYJET_SUB_CHANNEL_CLASSIFY_H
#define EASYJET_SUB_CHANNEL_CLASSIFY_H

#include <xAODEgamma/ElectronContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODTau/TauJetContainer.h>

#include <AthContainers/ConstDataVector.h>

#include "MultileptonEnum.h"

namespace MULTILEPTON
{
  class SubChannelClassify
  {
public:
    SubChannelClassify(const xAOD::MuonContainer *,
                       const xAOD::ElectronContainer *,
                       const xAOD::TauJetContainer *,
                       const ConstDataVector<xAOD::JetContainer> *,
                       const xAOD::EventInfo * event=nullptr);

    CH_ID classify_id();

    FLAVOR_1L classify_flavor_1L();
    FlAVOR_2L classify_flavor_2L();
    FLAVOR_3L classify_flavor_3L();
    FLAVOR_4L classify_flavor_4L();

    void reorder_3l();
    void fill_lepton_pairs();

    bool check_lep_pT(float min_pT) const;
    bool check_low_mass(float min_m) const;

    const std::vector<std::tuple<int, int, const xAOD::IParticle *>> &
    getLeptons() const
    {
      return m_leptons;
    }
    const std::vector<
        std::pair<const xAOD::IParticle *, const xAOD::IParticle *>> &
    getLepSFOC() const
    {
      return m_lep_SFOC;
    }
    const std::vector<
        std::pair<const xAOD::IParticle *, const xAOD::IParticle *>> &
    getLepSFSC() const
    {
      return m_lep_SFSC;
    }
    const std::vector<
        std::pair<const xAOD::IParticle *, const xAOD::IParticle *>> &
    getLepDF() const
    {
      return m_lep_DF;
    }
    int getNBjets() const { return m_n_bjets; }
    int getNElectrons() const { return m_n_electrons; }
    int getNMuons() const { return m_n_muons; }
    int getNLeptons() const { return m_n_leptons; }
    int getNTaus() const { return m_n_taus; }
    int getTotalChargeLep() const { return m_total_charge_lep; }
    int getTotalChargeTau() const { return m_total_charge_tau; }
    CH_ID getSubChannelId() const { return m_sub_channel_id; }
    int getSubChannelFlavor() const { return m_sub_channel_flavor; }

private:
    const xAOD::EventInfo *m_event;
    const xAOD::MuonContainer *m_muons;
    const xAOD::ElectronContainer *m_electrons;
    const xAOD::TauJetContainer *m_taus;
    const ConstDataVector<xAOD::JetContainer> *m_bjets;
    std::vector<std::tuple<int, int, const xAOD::IParticle *>> m_leptons;

    // Same-Flavor Opposite-Charge
    std::vector<std::pair<const xAOD::IParticle *, const xAOD::IParticle *>>
        m_lep_SFOC;
    // Same-Flavor Same-Charge
    std::vector<std::pair<const xAOD::IParticle *, const xAOD::IParticle *>>
        m_lep_SFSC;
    // Different-Flavor
    std::vector<std::pair<const xAOD::IParticle *, const xAOD::IParticle *>>
        m_lep_DF;

    int m_n_bjets;
    int m_n_electrons;
    int m_n_muons;
    int m_n_leptons;
    int m_n_taus;
    int m_total_charge_lep;
    int m_total_charge_tau;

    CH_ID m_sub_channel_id;
    int m_sub_channel_flavor;
  };
} // namespace MULTILEPTON

#endif // EASYJET_SUB_CHANNEL_CLASSIFY_H
