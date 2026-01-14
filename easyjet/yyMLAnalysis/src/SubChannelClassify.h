
#ifndef YYMLANALYSIS_SUBCHANNELCLASSIFY_H
#define YYMLANALYSIS_SUBCHANNELCLASSIFY_H

#include <xAODEventInfo/EventInfo.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODTau/TauJetContainer.h>

#include <AthContainers/ConstDataVector.h>

#include "yymlEnum.h"

namespace HHYYML {
  class SubChannelClassify {
public:
    SubChannelClassify(const xAOD::ElectronContainer *,
                       const xAOD::MuonContainer *,
                       const xAOD::TauJetContainer *,
                       const xAOD::EventInfo * event=nullptr);

    CH_ID classify_id() const;

    FLAVOR_1L classify_flavor_1L() const;
    FLAVOR_2L classify_flavor_2L() const;

    // void fill_lepton_pairs();

    // bool check_lep_pT(float min_pT) const;
    // bool check_low_mass(float min_m) const;

    // const std::vector<std::tuple<int, int, const xAOD::IParticle *>> &
    // getLeptons() const
    // {
    //   return m_leptons;
    // }
    int getNElectrons() const { return m_n_electrons; }
    int getNMuons() const { return m_n_muons; }
    int getNLeptons() const { return m_n_leptons; }
    int getNTaus() const { return m_n_taus; }
    CH_ID getSubChannelId() const { return m_sub_channel_id; }
    int getSubChannelFlavor() const { return m_sub_channel_flavor; }

private:
    const xAOD::ElectronContainer *m_electrons;
    const xAOD::MuonContainer *m_muons;
    const xAOD::TauJetContainer *m_taus;
    const xAOD::EventInfo *m_event;
    // std::vector<std::tuple<int, int, const xAOD::IParticle *>> m_leptons; // std::tuple<index, PDGID, pointer>

    int m_n_electrons;
    int m_n_muons;
    int m_n_leptons;
    int m_n_taus;
    
    CH_ID m_sub_channel_id;
    int m_sub_channel_flavor;
  };
} // namespace HHYYML

#endif // YYMLANALYSIS_SUBCHANNELCLASSIFY_H
