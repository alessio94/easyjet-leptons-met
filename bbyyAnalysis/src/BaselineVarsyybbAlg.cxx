/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "AthContainers/AuxElement.h"
#include "BaselineVarsyybbAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>

namespace HHBBYY
{
  BaselineVarsyybbAlg::BaselineVarsyybbAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsyybbAlg::initialize()
  {
    ATH_CHECK(m_smallRJets_BTag_ContainerInKey.initialize());
    ATH_CHECK(m_smallRJets_ContainerInKey.initialize());
    ATH_CHECK(m_photonContainerInKey.initialize());
    ATH_CHECK(m_muonContainerInKey.initialize());
    ATH_CHECK(m_electronContainerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());

    for (const std::string &var : m_vars)
    {
      std::string deco_var = var; 
      SG::AuxElement::Decorator<float> deco(deco_var);
      m_decos.emplace(deco_var, deco);
    };

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsyybbAlg::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    for (const std::string &var : m_vars)
    {
      std::string deco_var = var; 
      m_decos.at(deco_var)(*eventInfo) = -99.; 
    };

    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRJets_BTag(
       m_smallRJets_BTag_ContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRJets(
       m_smallRJets_ContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::PhotonContainer> > photons_(
        m_photonContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::MuonContainer> > muons_(
        m_muonContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::ElectronContainer> > electrons_(
        m_electronContainerInKey);

    static const SG::AuxElement::Accessor<char>  DFCommonPhotonsIsEMTight ("DFCommonPhotonsIsEMTight");
    static const SG::AuxElement::Accessor<char>  DFCommonPhotonsCleaning ("DFCommonPhotonsCleaning");
    static const SG::AuxElement::Accessor<char>  DFCommonElectronsLHMedium ("DFCommonElectronsLHMedium");
    static const SG::AuxElement::Accessor<char>  DFCommonMuonPassIDCuts ("DFCommonMuonPassIDCuts");
    static const SG::AuxElement::Accessor<char>  DFCommonMuonPassPreselection ("DFCommonMuonPassPreselection");

    ATH_CHECK(smallRJets_BTag.isValid());
    ATH_CHECK(smallRJets.isValid());
    ATH_CHECK(photons_.isValid());
    ATH_CHECK(muons_.isValid());
    ATH_CHECK(electrons_.isValid());
    ConstDataVector<xAOD::JetContainer> btag_jets = *smallRJets_BTag;
    ConstDataVector<xAOD::PhotonContainer> photons = *photons_;

    int TWO_TIGHTID_PHOTONS = 0;
    int TWO_ISO_PHOTONS = 0;
    int PASS_RELPT_CUT = 0;
    int MASSCUT = 0;
    int isPassed = 0;
    int N_LEPTONS_CUT = 0;
    int LESS_THAN_SIX_CENTRAL_JETS = 0;
    int EXACTLY_TWO_B_JETS = 0;
    std::vector<float> PassTightIDs;
    std::vector<float> PassIsos;
    std::vector<float> ptOverMasses;
    std::vector<float> CentralJetsEta;
    int n_leptons=0;

    bool PassIso = 0;
    double myy = -99;

    TLorentzVector H_BB;
    TLorentzVector H_yy;
    TLorentzVector H_HH;
    
    double Ht = 0; // scalar sum of jet pT

    // Photon sector
    if (photons.size() >= 1)
    {
      // Leading photon
      m_decos.at("Leading_Photon_pt")(*eventInfo) = photons[0]->pt();
      m_decos.at("Leading_Photon_eta")(*eventInfo) = photons[0]->eta();
      m_decos.at("Leading_Photon_phi")(*eventInfo) = photons[0]->phi();
      m_decos.at("Leading_Photon_E")(*eventInfo) = photons[0]->e();
    }
    if (photons.size() >= 2)
    {
      // Subleading photon
      m_decos.at("Subleading_Photon_pt")(*eventInfo) = photons[1]->pt();
      m_decos.at("Subleading_Photon_eta")(*eventInfo) = photons[1]->eta();
      m_decos.at("Subleading_Photon_phi")(*eventInfo) = photons[1]->phi();
      m_decos.at("Subleading_Photon_E")(*eventInfo) = photons[1]->e(); 
      
      // build the H(yy) candidate
      H_yy = photons[0]->p4() + photons[1]->p4();
      myy = H_yy.M();
      m_decos.at("myy")(*eventInfo) = myy;
      m_decos.at("pTyy")(*eventInfo) = H_yy.Pt();
      m_decos.at("Etayy")(*eventInfo) = H_yy.Eta();
      m_decos.at("Phiyy")(*eventInfo) = H_yy.Phi();
      m_decos.at("dRyy")(*eventInfo) = (photons[0]->p4()).DeltaR(photons[1]->p4());

      // photon isolation and selection pT/myy
      for (const xAOD::Photon *photon : *photons_)
      {
        PassIso = (photon->isolation(xAOD::Iso::topoetcone20)/photon->pt()) < 0.065 &&  (photon->isolation(xAOD::Iso::ptcone20)/photon->pt()) < 0.05 ;
        PassTightIDs.push_back(DFCommonPhotonsIsEMTight(*photon));
        PassIsos.push_back(PassIso);
        ptOverMasses.push_back(photon->pt()/myy);
      }

      if(PassTightIDs[0] == 1 && PassTightIDs[1] == 1) TWO_TIGHTID_PHOTONS = 1;
      if(PassIsos[0] == 1 && PassIsos[1] == 1) TWO_ISO_PHOTONS = 1;
      if(ptOverMasses[0] > 0.35 && ptOverMasses[1] > 0.25) PASS_RELPT_CUT = 1;
      if(myy >= 105e3 && myy < 160e3) MASSCUT = 1;

    } // end photon

    // b-jet sector
    if (btag_jets.size()>=1)
    {
      m_decos.at("Jet_pt_B1")(*eventInfo) = btag_jets[0]->pt();
      m_decos.at("Jet_eta_B1")(*eventInfo) = btag_jets[0]->eta();
      m_decos.at("Jet_phi_B1")(*eventInfo) = btag_jets[0]->phi();
      m_decos.at("Jet_E_B1")(*eventInfo) = btag_jets[0]->e();
    }
    if (btag_jets.size()>=2)
    {
      m_decos.at("Jet_pt_B2")(*eventInfo) = btag_jets[1]->pt();
      m_decos.at("Jet_eta_B2")(*eventInfo) = btag_jets[1]->eta();
      m_decos.at("Jet_phi_B2")(*eventInfo) = btag_jets[1]->phi();
      m_decos.at("Jet_E_B2")(*eventInfo) = btag_jets[1]->e();

      // build the H(BB) candidate
      H_BB = btag_jets[0]->p4()+btag_jets[1]->p4();
      m_decos.at("mBB")(*eventInfo) = H_BB.M();
      m_decos.at("pTBB")(*eventInfo) = H_BB.Pt();
      m_decos.at("EtaBB")(*eventInfo) = H_BB.Eta();
      m_decos.at("PhiBB")(*eventInfo) = H_BB.Phi();
      m_decos.at("dRBB")(*eventInfo) = (btag_jets[0]->p4()).DeltaR(btag_jets[1]->p4());
    }

    // build the HH candidate
    if (photons.size() >= 2 && btag_jets.size()>=2)
    {
      H_HH = H_yy + H_BB;
      m_decos.at("mBByy")(*eventInfo) = H_HH.M();
      m_decos.at("pTBByy")(*eventInfo) = H_HH.Pt();
      m_decos.at("EtaBByy")(*eventInfo) = H_HH.Eta();
      m_decos.at("PhiBByy")(*eventInfo) = H_HH.Phi();
      m_decos.at("dRBByy")(*eventInfo) = H_yy.DeltaR(H_BB);
      m_decos.at("mBByy_star")(*eventInfo) = H_HH.M() - (H_BB.M()-125e3) - (H_yy.M()-125e3);
    }

    // electron isolation
    for (const xAOD::Electron *electron : *electrons_)
    {
      bool PassElectronIso = 0;
      bool PassElectronMedium = 0;
      // or ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000
      PassElectronIso = (electron->isolation(xAOD::Iso::topoetcone20)/electron->pt()) < 0.20 &&  (electron->isolation(xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt500)/electron->pt()) < 0.15 ;
      PassElectronMedium = DFCommonElectronsLHMedium(*electron);
      if (PassElectronIso && PassElectronMedium)
        n_leptons+=1;
    }

    // muon isolation
    for (const xAOD::Muon *muon : *muons_)
    {
      bool PassMuonIso = 0;
      bool PassMuonMedium = 0;
      PassMuonIso = (muon->isolation(xAOD::Iso::topoetcone20)/muon->pt()) < 0.30 &&  (muon->isolation(xAOD::Iso::ptcone20)/muon->pt()) < 0.15 ;
      PassMuonMedium = DFCommonMuonPassIDCuts(*muon) && DFCommonMuonPassPreselection(*muon);
      if (PassMuonIso && PassMuonMedium)
        n_leptons+=1;
    }

    // No medium+isolated electrons and muons.
    if (n_leptons==0)
    {
      N_LEPTONS_CUT = 1;
    }

    // exactly 2 b-jets
    if (smallRJets_BTag->size()==2)
    {
      EXACTLY_TWO_B_JETS=1;
    }


    // Applying central jet cuts.
    for (const xAOD::Jet *jet : *smallRJets) // Jets here can be every type of jet (No Working point selected)
    {
      if(std::abs(jet->eta())<2.5) // check if jet is central 
      {
        CentralJetsEta.push_back(jet->eta()); // saving only eta
      }
      Ht += jet->pt();
    }
    m_decos.at("Ht")(*eventInfo) = Ht;

    if (CentralJetsEta.size()<6)
    {
      LESS_THAN_SIX_CENTRAL_JETS=1;
    }


    if((photons.size() >= 2)
       && TWO_TIGHTID_PHOTONS==1
       && TWO_ISO_PHOTONS==1
       && PASS_RELPT_CUT==1
       && MASSCUT==1
       && N_LEPTONS_CUT==1
       && LESS_THAN_SIX_CENTRAL_JETS==1
       && EXACTLY_TWO_B_JETS==1
    ) isPassed = 1;


    // Save cutflow booleans
    //m_decos.at("N_photons")(*eventInfo) = photons.size();  
    m_decos.at("N_LOOSE_PHOTONS")(*eventInfo) = photons.size();
    m_decos.at("TWO_LOOSE_PHOTONS")(*eventInfo) = photons.size() >= 2; 
    m_decos.at("TWO_TIGHTID_PHOTONS")(*eventInfo) = TWO_TIGHTID_PHOTONS;  
    m_decos.at("TWO_ISO_PHOTONS")(*eventInfo) = TWO_ISO_PHOTONS;
    m_decos.at("PASS_RELPT_CUT")(*eventInfo) = PASS_RELPT_CUT;
    m_decos.at("MASSCUT")(*eventInfo) = MASSCUT;
    m_decos.at("N_LEPTONS_CUT")(*eventInfo) = N_LEPTONS_CUT;
    m_decos.at("LESS_THAN_SIX_CENTRAL_JETS")(*eventInfo) = LESS_THAN_SIX_CENTRAL_JETS;
    m_decos.at("EXACTLY_TWO_B_JETS")(*eventInfo) = EXACTLY_TWO_B_JETS;
    m_decos.at("isPassed")(*eventInfo) = isPassed;

    return StatusCode::SUCCESS;
  }
}
