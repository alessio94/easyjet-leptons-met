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

namespace HH4B
{
  BaselineVarsyybbAlg::BaselineVarsyybbAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("bTagWP", m_bTagWP);
  }

  StatusCode BaselineVarsyybbAlg::initialize()
  {
    ATH_CHECK(m_smallRContainerInKey.initialize());
    ATH_CHECK(m_photonContainerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());

    // make decorators
    for (const std::string &var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
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

    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRjets(
       m_smallRContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::PhotonContainer> > photons_(
        m_photonContainerInKey);

    //static const SG::AuxElement::Accessor<char>  DFCommonPhotonsIsEMLoose ("DFCommonPhotonsIsEMLoose");
    static const SG::AuxElement::Accessor<char>  DFCommonPhotonsIsEMTight ("DFCommonPhotonsIsEMTight");
    static const SG::AuxElement::Accessor<char>  DFCommonPhotonsCleaning ("DFCommonPhotonsCleaning");

    //AsgPhotonIsEMSelector PID("AsgPhotonIsEMSelector");

    ATH_CHECK(smallRjets.isValid());
    ATH_CHECK(photons_.isValid());

    ConstDataVector<xAOD::JetContainer> jets = *smallRjets;
    ConstDataVector<xAOD::PhotonContainer> photons = *photons_;

    int TWO_TIGHTID_PHOTONS = 0;
    int TWO_ISO_PHOTONS = 0;
    int PASS_RELPT_CUT = 0;
    int MASSCUT = 0;
    int isPassed = 0;
    //Adding cut : < 6 central jets with pt<25GeV
    int LESS_THAN_SIX_CENTRAL_JETS_CUT=0;
    std::vector<float> PassTightIDs;
    std::vector<float> PassIsos;
    std::vector<float> ptOverMasses;
    std::vector<float> CentralJetsEta;



    bool PassIso = 0;
    double myy = -99;

    if(photons.size() >= 2){
      //auto h2 = (photons[0]->p4() + photons[1]->p4());
      myy = (photons[0]->p4() + photons[1]->p4()).M();
      //myy = h2.M();

      for (const xAOD::Photon *photon : *photons_)
      {
        PassIso = (bool)( (photon->isolation(xAOD::Iso::topoetcone20)/photon->pt()) < 0.065 &&  (photon->isolation(xAOD::Iso::ptcone20)/photon->pt()) < 0.05 );
        PassTightIDs.push_back(DFCommonPhotonsIsEMTight(*photon));
        PassIsos.push_back(PassIso);
        ptOverMasses.push_back(photon->pt()/myy);
      }

      if(PassTightIDs[0] == 1 && PassTightIDs[1] == 1) TWO_TIGHTID_PHOTONS = 1;
      if(PassIsos[0] == 1 && PassIsos[1] == 1) TWO_ISO_PHOTONS = 1;
      if(ptOverMasses[0] > 0.35 && ptOverMasses[1] > 0.25) PASS_RELPT_CUT = 1;
      if(myy >= 105000. && myy < 160000.) MASSCUT = 1;

    }


    //Applying central jets cut. 

    for (const xAOD::Jet *jet : *smallRjets)
    {
        if(std::abs(jet->eta())<2.5)
        {
          CentralJetsEta.push_back(jet->eta());
        }

        // else
        //    std::cout << "Jet eta : " << jet->eta() << std::endl;

    }

    if (CentralJetsEta.size()<6)
    {
      LESS_THAN_SIX_CENTRAL_JETS_CUT=1;
    }

    if((photons.size() >= 2)
       && TWO_TIGHTID_PHOTONS==1
       && TWO_ISO_PHOTONS==1
       && PASS_RELPT_CUT==1
       && MASSCUT==1
       && LESS_THAN_SIX_CENTRAL_JETS_CUT==1
    ) isPassed = 1;





    // Save cutflow booleans
    //m_decos.at("N_photons")(*eventInfo) = photons.size();  
    m_decos.at("N_LOOSE_PHOTONS")(*eventInfo) = photons.size();
    m_decos.at("TWO_LOOSE_PHOTONS")(*eventInfo) = photons.size() >= 2; 
    m_decos.at("TWO_TIGHTID_PHOTONS")(*eventInfo) = TWO_TIGHTID_PHOTONS;  
    m_decos.at("TWO_ISO_PHOTONS")(*eventInfo) = TWO_ISO_PHOTONS;
    m_decos.at("PASS_RELPT_CUT")(*eventInfo) = PASS_RELPT_CUT;
    m_decos.at("MASSCUT")(*eventInfo) = MASSCUT;
    m_decos.at("LESS_THAN_SIX_CENTRAL_JETS_CUT")(*eventInfo) = LESS_THAN_SIX_CENTRAL_JETS_CUT;
    m_decos.at("isPassed")(*eventInfo) = isPassed;


    if(photons.size() == 1){
      // Leading photon
      m_decos.at("Leading_Photon_pt")(*eventInfo) = photons[0]->pt();
      m_decos.at("Leading_Photon_eta")(*eventInfo) = photons[0]->eta();
      m_decos.at("Leading_Photon_phi")(*eventInfo) = photons[0]->phi();
      m_decos.at("Leading_Photon_E")(*eventInfo) = photons[0]->e();

    }

    if(photons.size() >= 2){
      // // Leading photon
      m_decos.at("Leading_Photon_pt")(*eventInfo) = photons[0]->pt();
      m_decos.at("Leading_Photon_eta")(*eventInfo) = photons[0]->eta();
      m_decos.at("Leading_Photon_phi")(*eventInfo) = photons[0]->phi();
      m_decos.at("Leading_Photon_E")(*eventInfo) = photons[0]->e();

      // Subleading photon
      m_decos.at("Subleading_Photon_pt")(*eventInfo) = photons[1]->pt();
      m_decos.at("Subleading_Photon_eta")(*eventInfo) = photons[1]->eta();
      m_decos.at("Subleading_Photon_phi")(*eventInfo) = photons[1]->phi();
      m_decos.at("Subleading_Photon_E")(*eventInfo) = photons[1]->e(); 
      m_decos.at("myy")(*eventInfo) = myy;
    }

    if (jets.size()==1)
    {
      m_decos.at("Leading_Jet_pt")(*eventInfo) = jets[0]->pt();
      m_decos.at("Leading_Jet_eta")(*eventInfo) = jets[0]->eta();
      m_decos.at("Leading_Jet_phi")(*eventInfo) = jets[0]->phi();
      m_decos.at("Leading_Jet_E")(*eventInfo) = jets[0]->e();

    }

    if (jets.size()>=2)
    {
      m_decos.at("Leading_Jet_pt")(*eventInfo) = jets[0]->pt();
      m_decos.at("Leading_Jet_eta")(*eventInfo) = jets[0]->eta();
      m_decos.at("Leading_Jet_phi")(*eventInfo) = jets[0]->phi();
      m_decos.at("Leading_Jet_E")(*eventInfo) = jets[0]->e();


      m_decos.at("Subleading_Jet_pt")(*eventInfo) = jets[1]->pt();
      m_decos.at("Subleading_Jet_eta")(*eventInfo) = jets[1]->eta();
      m_decos.at("Subleading_Jet_phi")(*eventInfo) = jets[1]->phi();
      m_decos.at("Subleading_Jet_E")(*eventInfo) = jets[1]->e();

    }

    return StatusCode::SUCCESS;
  }
}
