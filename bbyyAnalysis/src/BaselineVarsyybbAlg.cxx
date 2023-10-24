/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsyybbAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>

namespace HHBBYY
{
  BaselineVarsyybbAlg::BaselineVarsyybbAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("isMC", m_isMC);
  }

  StatusCode BaselineVarsyybbAlg::initialize()
  {
    ATH_CHECK(m_smallRJets_BTag_ContainerInKey.initialize());
    ATH_CHECK(m_smallRJets_ContainerInKey.initialize());
    ATH_CHECK(m_photonContainerInKey.initialize());
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

    static const SG::AuxElement::ConstAccessor<int>  HadronConeExclTruthLabelID("HadronConeExclTruthLabelID");

    for (const std::string &var : m_vars)
    {
      std::string deco_var = var; 
      m_decos.at(deco_var)(*eventInfo) = -99.; 
    };

    double Ht = 0; // scalar sum of jet pT

    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRJets_BTag(
       m_smallRJets_BTag_ContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRJets(
       m_smallRJets_ContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::PhotonContainer> > photons_(
        m_photonContainerInKey);

    ConstDataVector<xAOD::JetContainer> btag_jets = *smallRJets_BTag;
    ConstDataVector<xAOD::PhotonContainer> photons = *photons_;

    ATH_CHECK(smallRJets_BTag.isValid());
    ATH_CHECK(smallRJets.isValid());
    ATH_CHECK(photons_.isValid());

    TLorentzVector H_BB;
    TLorentzVector H_yy;
    TLorentzVector H_HH;

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
      m_decos.at("myy")(*eventInfo) = H_yy.M();
      m_decos.at("pTyy")(*eventInfo) = H_yy.Pt();
      m_decos.at("Etayy")(*eventInfo) = H_yy.Eta();
      m_decos.at("Phiyy")(*eventInfo) = H_yy.Phi();
      m_decos.at("dRyy")(*eventInfo) = (photons[0]->p4()).DeltaR(photons[1]->p4());
    } // end photon

    // b-jet sector
    if (btag_jets.size()>=1)
    {
      m_decos.at("Jet_pt_B1")(*eventInfo) = btag_jets[0]->pt();
      m_decos.at("Jet_eta_B1")(*eventInfo) = btag_jets[0]->eta();
      m_decos.at("Jet_phi_B1")(*eventInfo) = btag_jets[0]->phi();
      m_decos.at("Jet_E_B1")(*eventInfo) = btag_jets[0]->e();

      //Get Leading B-Tagged Jet's Flavor
      if(m_isMC)
        m_decos.at("Jet_HadronConeExclTruthLabelID_B1")(*eventInfo) = HadronConeExclTruthLabelID(*btag_jets[0]);
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

      //Get Subleading B-Tagged Jet's Flavor
      if(m_isMC)
        m_decos.at("Jet_HadronConeExclTruthLabelID_B2")(*eventInfo) = HadronConeExclTruthLabelID(*btag_jets[1]);
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

    // Applying central jet cuts.
    for (const xAOD::Jet *jet : *smallRJets) // Jets here can be every type of jet (No Working point selected)
    {
      Ht += jet->pt();
    }
    m_decos.at("Ht")(*eventInfo) = Ht;

    return StatusCode::SUCCESS;
  }
}
