/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsMultileptonAlg.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"
#include "EasyjetHub/MT2_ROOT.h"

namespace MULTILEPTON
{
  BaselineVarsMultileptonAlg::BaselineVarsMultileptonAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsMultileptonAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("   BaselineVarsMultileptonAlg    \n");
    ATH_MSG_INFO("*********************************\n");

    // Read syst-aware input handles
    ATH_CHECK(m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK(m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK(m_metHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

    // Initialise syst-aware output decorators
    for(const std::string &var : m_floatVariables){
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK(m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for(const std::string &var : m_floatVectorVariables){
      CP::SysWriteDecorHandle<std::vector<float>> whandle{var+"_%SYS%", this};
      m_FVbranches.emplace(var, whandle);
      ATH_CHECK(m_FVbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for(const std::string &var : m_intVariables){
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    for(const std::string & var : m_charVectorVariables){
      CP::SysWriteDecorHandle<std::vector<char>> whandle(var+"_%SYS%", this);
      m_CVbranches.emplace(var, whandle);
      ATH_CHECK(m_CVbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    if (!m_isBtag.empty()){
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_met_sig.initialize(m_systematicsList, m_metHandle));

    if(m_isMC) ATH_CHECK(m_truthClassifier.retrieve());

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsMultileptonAlg::execute()
  {
    // Loop over all systematics
    for (const auto& sys : m_systematicsList.systematicsVector()){
      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -999., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -999, sys);
      }

      for (const auto& var: m_floatVectorVariables) {
        m_FVbranches.at(var).set(*event, {}, sys);
      }

      for (const auto& var: m_charVectorVariables) {
        m_CVbranches.at(var).set(*event, {}, sys);
      }

      int n_electrons = electrons->size();
      int n_muons = muons->size();
      int n_jets = jets->size();
      int nCentralJets = 0;

      // b-jet sector
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        // count central jets
        if (std::abs(jet->eta())<2.5) {
          nCentralJets++;
          if (WPgiven && m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }
      int n_bjets = bjets->size();

      m_Ibranches.at("nJets").set(*event, n_jets, sys);
      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
      m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);

      // selected leptons;
      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;
      const xAOD::Electron* ele2 = nullptr;
      const xAOD::Electron* ele3 = nullptr;

      for(const xAOD::Electron* electron : *electrons) {
        if(!ele0) ele0 = electron;
        else if(!ele1) ele1 = electron;
        else if(!ele2) ele2 = electron;
        else {
          ele3 = electron;
          break;
        }
      }

      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;
      const xAOD::Muon* mu2 = nullptr;
      const xAOD::Muon* mu3 = nullptr;

      for(const xAOD::Muon* muon : *muons) {
        if(!mu0) mu0 = muon;
        else if(!mu1) mu1 = muon;
        else if(!mu2) mu2 = muon;
        else {
          mu3 = muon;
          break;
        }
      }

      std::vector<std::pair<const xAOD::IParticle*, int>> leptons;
      if(ele0) leptons.emplace_back(ele0, -11*ele0->charge());
      if(mu0) leptons.emplace_back(mu0, -13*mu0->charge());
      if(ele1) leptons.emplace_back(ele1, -11*ele1->charge());
      if(mu1) leptons.emplace_back(mu1, -13*mu1->charge());
      if(ele2) leptons.emplace_back(ele2, -11*ele2->charge());
      if(mu2) leptons.emplace_back(mu2, -13*mu2->charge());
      if(ele3) leptons.emplace_back(ele3, -11*ele3->charge());
      if(mu3) leptons.emplace_back(mu3, -13*mu3->charge());

      int nLeptons = muons->size() + electrons->size();
      m_Ibranches.at("nLeptons").set(*event, nLeptons, sys);
      
      // Sort by pT; FIXME: for 3l, os lepton should be the 1st then the ss leptons sorted by pT
      std::sort(leptons.begin(), leptons.end(),
          [](const std::pair<const xAOD::IParticle*, int>& a,
              const std::pair<const xAOD::IParticle*, int>& b) {
            return a.first->pt() > b.first->pt(); });
      
      static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> truthParticleLinkAcc("truthParticleLink");
      static const SG::AuxElement::ConstAccessor<float> eleD0sigAcc("d0sig_"+m_eleWPName);
      static const SG::AuxElement::ConstAccessor<float> eleZ0sinthetaAcc("z0sintheta_"+m_eleWPName);
      static const SG::AuxElement::ConstAccessor<float> muD0sigAcc("d0sig_"+m_muWPName);
      static const SG::AuxElement::ConstAccessor<float> muZ0sinthetaAcc("z0sintheta_"+m_muWPName);


      for(unsigned int i=0; i < std::min((size_t)m_leptonAmount, leptons.size()); i++){
          std::string prefix = "Lepton"+std::to_string(i+1);
          TLorentzVector tlv = leptons[i].first->p4();
          m_Fbranches.at(prefix+"_pt").set(*event, tlv.Pt(), sys);
          m_Fbranches.at(prefix+"_eta").set(*event, tlv.Eta(), sys);
          m_Fbranches.at(prefix+"_phi").set(*event, tlv.Phi(), sys);
          m_Fbranches.at(prefix+"_E").set(*event, tlv.E(), sys);
          
          int lep_charge = leptons[i].second > 0 ? -1 : 1;
          int lep_pdgid = leptons[i].second;
          m_Ibranches.at(prefix+"_charge").set(*event, lep_charge, sys);
          m_Ibranches.at(prefix+"_pdgid").set(*event, lep_pdgid, sys);

          float lep_EtaBE2 = std::abs(lep_pdgid) == 11 ? 
              dynamic_cast<const xAOD::Electron*>(leptons[i].first)->caloCluster()->etaBE(2) : -99.0;
          m_Fbranches.at(prefix+"_EtaBE2").set(*event, lep_EtaBE2, sys);

          float lep_Z0SinTheta = std::abs(lep_pdgid) == 11 ? eleZ0sinthetaAcc(*(leptons[i].first)) : muZ0sinthetaAcc(*(leptons[i].first));
          m_Fbranches.at(prefix+"_Z0SinTheta").set(*event, lep_Z0SinTheta, sys);

          float lep_sigd0PV = std::abs(lep_pdgid) == 11 ? eleD0sigAcc(*(leptons[i].first)) : muD0sigAcc(*(leptons[i].first));
          m_Fbranches.at(prefix+"_sigd0PV").set(*event, lep_sigd0PV, sys);

          // leptons truth information
          if (m_isMC){
            auto [lep_truthOrigin, lep_truthType] = truthOrigin(leptons[i].first);
            m_Ibranches.at(prefix + "_truthOrigin").set(*event, lep_truthOrigin, sys);
            m_Ibranches.at(prefix + "_truthType").set(*event, lep_truthType, sys);
            int lep_isPrompt = 0;
            if (std::abs(lep_pdgid)==13){ // simplistic
              if (lep_truthType==6) lep_isPrompt=1; // isolated prompts
            } else if (std::abs(lep_pdgid)==11){
              if (lep_truthType==2) lep_isPrompt=1; // isolated prompts
            }
            m_Ibranches.at(prefix + "_isPrompt").set(*event, lep_isPrompt, sys);

            //record _isQMisID for leptons
            int isQMisID = -1;
            if (std::abs(lep_pdgid)==11){
              if ( !truthParticleLinkAcc.isAvailable( *(leptons[i].first) ) ) {
                ATH_MSG_WARNING("No link available to truth match for this reco electron. This shouldn't happen.");
              } else {
                auto link = truthParticleLinkAcc( *(leptons[i].first) );
                if(!link.isValid()) ATH_MSG_VERBOSE("Link to truth match for this reco electron is invalid. This shouldn't happen.");
                const xAOD::TruthParticle* truthMatch = link.isValid() ? *link : nullptr;
                if ( !truthMatch ) {
                  ATH_MSG_WARNING("No truth match available for this reco electron. This shouldn't happen.");
                } else if (truthMatch->isElectron()){
                  isQMisID = getTruthQMisID(leptons[i].first, truthMatch);
                }
              }
            } else if (std::abs(lep_pdgid)==13 && std::abs(leptons[i].first->eta())<2.5){
              const xAOD::TrackParticle* trk(nullptr);
              ElementLink< xAOD::TrackParticleContainer > trkLink = dynamic_cast<const xAOD::Muon*>(leptons[i].first)->inDetTrackParticleLink();

              if ( !trkLink.isValid() ) {
                ATH_MSG_WARNING("Link to ID track particle for this reco muon is invalid. This shouldn't happen.");
              } else {
                trk = *trkLink;

                // Get the truth particle matching the ID track
                //
                if ( !truthParticleLinkAcc.isAvailable( *trk ) ) {
                  ATH_MSG_WARNING("No link available to truth match for this reco muon's ID track. This shouldn't happen. Returning");
                } else {
                  auto link = truthParticleLinkAcc( *trk );
                  if(!link.isValid()) ATH_MSG_VERBOSE("Link to truth match for this reco muon's ID track is invalid. This shouldn't happen.");
                  const xAOD::TruthParticle* truthMatch = link.isValid() ? *link : nullptr;

                  // If there is no matching truth particle for the ID track, return and spit out a warning.
                  //
                  if ( !truthMatch ) {
                    ATH_MSG_WARNING("No truth match for this reco muon's ID track. This shouldn't happen. Returning");
                  } else if (truthMatch->isMuon()){
                    isQMisID = getTruthQMisID(leptons[i].first, truthMatch);
                  }
                }
              }
            }
            if( isQMisID != -1 ) m_Ibranches.at(prefix + "_isQMisID").set(*event, isQMisID, sys);

            // TODO: record mll_conv, mll_conv_at_ConvV for electrons.
            // TODO: record radius_conv, radius_convX, radius_convY for electrons
          }
        }


    }
    return StatusCode::SUCCESS;
  }

  template<typename ParticleType>
  std::pair<int, int> BaselineVarsMultileptonAlg::truthOrigin(const ParticleType* particle) {
    static const SG::AuxElement::ConstAccessor<int> lepTruthOrigin("truthOrigin");
    static const SG::AuxElement::ConstAccessor<int> lepTruthType("truthType");
    
    return {lepTruthOrigin(*particle), lepTruthType(*particle)};
  }


  int BaselineVarsMultileptonAlg::getTruthQMisID( const xAOD::IParticle* lep, const xAOD::TruthParticle* truthMatch ) 
  {
    int pdgId = truthMatch->pdgId();

    // A safety check: immediately return if input truth particle
    // is NOT an electron/muon
    //
    if ( !( truthMatch->isElectron() || truthMatch->isMuon() ) ) {
      ATH_MSG_WARNING("getTruthQMisID() :: Input truth particle is NOT an electron/muon (pdgId: " << pdgId << "). Will not check whether it's charge flip. Returning.");
      return -1;
    }

    int reco_charge = 0;
    if ( lep->type() == xAOD::Type::Electron ) {
      ATH_MSG_DEBUG("getTruthQMisID() :: This reco lepton is an electron" );
      reco_charge = static_cast<int>(dynamic_cast<const xAOD::Electron*>(lep)->charge());
    } else if ( lep->type() == xAOD::Type::Muon ) {
      ATH_MSG_DEBUG("getTruthQMisID() :: This reco lepton is a muon" );
      reco_charge = static_cast<int>(dynamic_cast<const xAOD::Muon*>(lep)->charge());
    }
    if ( !reco_charge ) {
      ATH_MSG_ERROR("getTruthQMisID() :: Reco particle has zero charge. This shouldn't happen. Aborting");
      return -1;
    }

    xAOD::TruthParticle* primitiveTruth(nullptr);
    static const SG::AuxElement::ConstAccessor<int> truthTypeAcc("truthType");

    if ( !truthTypeAcc.isAvailable( *lep ) ) {
      ATH_MSG_ERROR("getTruthQMisID() :: No accessor to truthType available for this reco lepton. This shouldn't happen. Aborting");
      return -1;
    }

    // case 1:
    //
    // Lepton (in most cases, an electron) is matched to a truth lepton which is part of a bremmmstrahlung shower.
    // In this case, we need to go back until we find the original lepton that radiated the photon.
    // The charge of this primitive lepton is the one to look at!
    //
    // look at 'Background'-type el/mu (see MCTruthClassifier.h)
    //

    bool isBkgLep(false);

    int pdgId_primitive(-999);

    if ( truthTypeAcc( *lep ) == 4 || truthTypeAcc( *lep ) == 8 ) {

      isBkgLep = true;

      ATH_MSG_DEBUG("getTruthQMisID() :: This reco lepton (charge: " << reco_charge << " ) is matched to a secondary truth lepton. Let's go back until we find the primitive");

      bool foundPrimitive(false);

      // Use this just to break the while loop in ill-fated cases
      //
      unsigned int iGeneration(0);

      // Check that the match has a parent
      //
      if ( !truthMatch->nParents() ) {
        ATH_MSG_WARNING("getTruthQMisID() :: This reco lepton's match has no parents. Will not check whether it's charge flip. Returning");
        return -1;
      }

      primitiveTruth = const_cast<xAOD::TruthParticle*>( truthMatch->parent(0) );

      while ( !( foundPrimitive || iGeneration > 20 ) ) {

        pdgId_primitive = primitiveTruth->pdgId();

        // Protection for truth particles w/o a prodVertex
        //
        if ( !primitiveTruth->hasProdVtx() ) {
          ATH_MSG_WARNING("getTruthQMisID() :: \t Found a truth particle w/o production vertex. Will not check whether this reco lepton is charge flip. Returning");
          return -1;
        }

        // Check if prod vtx is compatible to a secondary interaction. If that's the case, go back in the chain!
        //
        if ( primitiveTruth->prodVtx()->barcode() < -200000 ) {
          ATH_MSG_DEBUG("getTruthQMisID() :: \t Parent has pdgId: " << pdgId_primitive << ", prodVtx barcode: " << primitiveTruth->prodVtx()->barcode() << " - Need to go backwards in the decay chain");

          if ( !primitiveTruth->nParents() ) {
            ATH_MSG_WARNING("getTruthQMisID() :: \t This truth ancestor has no parents. Will not check whether reco lepton it's charge flip. Returning");
            return -1;
          }

          primitiveTruth = const_cast<xAOD::TruthParticle*>( primitiveTruth->parent(0) );

        } else {

  	      // Ok, we found the primitive!
          foundPrimitive = true;

          ATH_MSG_DEBUG("getTruthQMisID() :: \t We found the primitive! pdgId: " << pdgId_primitive << ", prodVtx barcode: " << primitiveTruth->prodVtx()->barcode() );

        }

        ++iGeneration;
      }

    } else {
      // case 2:
      //
      // Lepton is matched to a truth lepton which is not produced in a secondary interaction.
      // Will check the charge directly on the truth match.
      //
      primitiveTruth = const_cast<xAOD::TruthParticle*>( truthMatch );
      pdgId_primitive = primitiveTruth->pdgId();
    }

    // Use the MCTruthClassifier to get the type and origin of the primitive truth ancestor,
    // and save it as a decoration
    //
    std::pair<MCTruthPartClassifier::ParticleType,MCTruthPartClassifier::ParticleOrigin>  ancestor_info = m_truthClassifier->particleTruthClassifier( primitiveTruth );

    // Now check the charge!
    //
    int truth_charge = static_cast<int>(primitiveTruth->charge());

    // Flag a lepton as 'isQMisID' only if:
    //
    // -) the primitive ancestor is NOT a photon
    //   AND
    // -) the primitve ancestor origin is not ISR/FSR...
    //   AND
    // -) the charge has actually flipped in reco ;-)

    // NB in this scheme: primitive ancestor --> first non-GEANT-originated particle
    int isQMisID(0);

    if ( primitiveTruth->isLepton() && !primitiveTruth->isPhoton() ) {

      if ( !(ancestor_info.second == 39 || ancestor_info.second == 40) && ( reco_charge * truth_charge ) < 0 ) { isQMisID = 1; }

    }

    ATH_MSG_DEBUG( "getTruthQMisID() :: \n\nPrimitive TRUTH: \n" <<
  		"norm charge: " << (truth_charge>0)-(truth_charge<0) << "\n" <<
  		"pdgId: " << pdgId_primitive << "\n" <<
  		"prodVtxBarcode: " << primitiveTruth->prodVtx()->barcode() << "\n" <<
  		"status: " << primitiveTruth->status() << "\n" <<
  		"type: " << ancestor_info.first << "\n" <<
  		"origin: " << ancestor_info.second << "\n" <<
  		"-----------\n" <<
  		"isBkgLep? " << isBkgLep << "\n" <<
  		"-----------\nRECO: \n" <<
  		"norm charge: " << reco_charge << " \n" <<
  		"-----------\n" <<
  		"isQMisID? " << isQMisID << "\n\n");

    return isQMisID;

  }


}