/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Victor Ruelas, Shudong Wang

//
// includes
//
#include "TruthParticleInformationAlg.h"
#include "TruthUtils.h"
#include <algorithm>

#include <AsgDataHandles/WriteDecorHandle.h>

#include "TruthUtils/HepMCHelpers.h"


//
// method implementations
//
namespace Easyjet
{
  namespace DecayMode {
    enum DecayMode : int8_t {
      bbbb=0, bbmm, bbtt, bbyy, bbWW, bbZZ,
      mmtt, eett, tttt,
      WWWW, WWZZ, WWtt,
      ZZtt, ZZZZ,
      bbbbtt,
      // Insert any new entries before this
      NMODES
    };
  }

  // Has to be kept updated in parallel with DecayMode
  // No good way to assert at compile time?
  const std::unordered_map<std::string, DecayMode::DecayMode> decayModes_str_to_enum{
    {"bbbb",    DecayMode::bbbb},
    {"bbmm",    DecayMode::bbmm},
    {"bbtt",    DecayMode::bbtt},
    {"bbyy",    DecayMode::bbyy},
    {"bbWW",    DecayMode::bbWW},
    {"bbZZ",    DecayMode::bbZZ},
    {"mmtt",    DecayMode::mmtt},
    {"eett",    DecayMode::eett},
    {"tttt",    DecayMode::tttt},
    {"WWWW",    DecayMode::WWWW},
    {"WWZZ",   DecayMode::WWZZ},
    {"WWtt",   DecayMode::WWtt},
    {"ZZtt",   DecayMode::ZZtt},
    {"ZZZZ",   DecayMode::ZZZZ},
    {"bbbbtt", DecayMode::bbbbtt},
  };

  // Using an array allows us to verify the size to check that all cases are covered
  const std::set<int> decayProducts_IDs[DecayMode::NMODES] {
    {MC::BQUARK, -MC::BQUARK}, // bbbb
    {MC::BQUARK, -MC::BQUARK, MC::MUON, -MC::MUON}, // bbmm
    {MC::BQUARK, -MC::BQUARK, MC::TAU, -MC::TAU}, //bbtt
    {MC::BQUARK, -MC::BQUARK, MC::PHOTON}, //bbyy
    {MC::BQUARK, -MC::BQUARK, MC::WPLUSBOSON, -MC::WPLUSBOSON}, // bbWW
    {MC::BQUARK, -MC::BQUARK, MC::Z0BOSON}, // bbZZ
    {MC::MUON, -MC::MUON, MC::TAU, -MC::TAU}, // mmtt
    {MC::ELECTRON, -MC::ELECTRON, MC::TAU, -MC::TAU}, //eett
    {MC::TAU, -MC::TAU}, //tttt
    {MC::WPLUSBOSON, -MC::WPLUSBOSON}, // WWWW
    {MC::WPLUSBOSON, -MC::WPLUSBOSON, MC::Z0BOSON}, // WWZZ
    {MC::WPLUSBOSON, -MC::WPLUSBOSON, MC::TAU, -MC::TAU}, // WWtt
    {MC::Z0BOSON, MC::TAU, -MC::TAU}, // ZZtt
    {MC::Z0BOSON}, // ZZZZ
    // The following one is not differentiable from bbtt
    {MC::BQUARK, -MC::BQUARK, MC::TAU, -MC::TAU}, // bbbbtt
  };

  class TruthScalar
  {
private:
    using P4 = ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>>;
    int m_pdgId;
    P4 m_p4;
    const xAOD::TruthParticle *m_source = nullptr;
    // final children
    std::vector<P4> m_children_p4;
    std::vector<P4> m_initial_children_p4;
    std::vector<std::vector<P4>> m_grandchildren_p4;
    std::vector<std::vector<P4>> m_initial_grandchildren_p4;
    std::vector<const xAOD::TruthParticle *> m_children;
    std::vector<const xAOD::TruthParticle *> m_initial_children;
    std::vector<std::vector<const xAOD::TruthParticle *>> m_grandchildren;
    std::vector<std::vector<const xAOD::TruthParticle *>> m_initial_grandchildren;

public:
    TruthScalar() : m_pdgId{0} {};
    explicit TruthScalar(const xAOD::TruthParticle *h): 
    m_pdgId{h->pdgId()}, m_source(h)
    {
      m_p4.SetCoordinates(h->pt(), h->eta(), h->phi(), h->m());
    } ;

    bool valid() const {return m_source!=nullptr;}

    const xAOD::TruthParticle *source() { return m_source; };

    int pdgId() const { return m_pdgId; };

    float p4(int coordIdx)
    {
      std::array<float, 4> coords;
      m_p4.GetCoordinates(coords.begin());
      return coords[coordIdx];
    }

    void children(std::vector<const xAOD::TruthParticle *> children)
    {
      for (const xAOD::TruthParticle *child : children)
      {
        P4 child_p4{child->pt(), child->eta(), child->phi(), child->m()};
        m_children_p4.push_back(child_p4);
      }
      m_children = std::move(children);
    }

    void initial_children(std::vector<const xAOD::TruthParticle *> initial_children)
    {
      for (const xAOD::TruthParticle *initial_child : initial_children)
      {
        P4 initial_child_p4{initial_child->pt(), initial_child->eta(), initial_child->phi(), initial_child->m()};
        m_initial_children_p4.push_back(initial_child_p4);
      }
      m_initial_children = std::move(initial_children);
    }

    void grandchildren(
      std::vector<std::vector<const xAOD::TruthParticle *>> grandchildren)
    {
      for (const std::vector<const xAOD::TruthParticle *>& 
          partial_grandchildren : grandchildren)
      { 
        std::vector<P4> partial_grandchildren_p4;
        for (const xAOD::TruthParticle *grandchild : partial_grandchildren)
        {
          P4 grandchild_p4{grandchild->pt(), grandchild->eta(), 
                            grandchild->phi(), grandchild->m()};
          partial_grandchildren_p4.push_back(grandchild_p4);
        }
        m_grandchildren_p4.push_back(partial_grandchildren_p4);
      }
      m_grandchildren = std::move(grandchildren);
    }

    void initial_grandchildren(
      std::vector<std::vector<const xAOD::TruthParticle *>> initial_grandchildren)
    {
      for (const std::vector<const xAOD::TruthParticle *>& 
        partial_initial_grandchildren : initial_grandchildren)
      { 
        std::vector<P4> partial_initial_grandchildren_p4;
        for (const xAOD::TruthParticle 
          *initial_grandchild : partial_initial_grandchildren)
        {
          P4 initial_grandchild_p4{initial_grandchild->pt(), initial_grandchild->eta(),
                                    initial_grandchild->phi(), initial_grandchild->m()};
          partial_initial_grandchildren_p4.push_back(initial_grandchild_p4);
        }
        m_initial_grandchildren_p4.push_back(partial_initial_grandchildren_p4);
      }
      m_initial_grandchildren = std::move(initial_grandchildren);
    }

    std::vector<int> children_pdgId(){
      std::vector<int> pdgId_pair;
      for (const xAOD::TruthParticle *child : m_children)
      {
        pdgId_pair.push_back(child->pdgId());
      }
      return pdgId_pair;
    }

    std::vector<int> initial_children_pdgId(){
      std::vector<int> pdgId_pair;
      for (const xAOD::TruthParticle *initial_child : m_initial_children)
      {
        pdgId_pair.push_back(initial_child->pdgId());
      }
      return pdgId_pair;
    }

    std::vector<std::vector<int>> grandchildren_pdgId(){
      std::vector<std::vector<int>> pdgIds;
      for (const std::vector<const xAOD::TruthParticle *>& 
        partial_grandchildren : m_grandchildren)
      { 
        std::vector<int> partial_grandchildren_pdgId;
        for (const xAOD::TruthParticle *grandchild : partial_grandchildren)
        {
          partial_grandchildren_pdgId.push_back(grandchild->pdgId());
        }
        pdgIds.push_back(partial_grandchildren_pdgId);
      }
      return pdgIds;
    }

    std::vector<std::vector<int>> initial_grandchildren_pdgId(){
      std::vector<std::vector<int>> pdgIds;
      for (const std::vector<const xAOD::TruthParticle *>& 
        partial_initial_grandchildren : m_initial_grandchildren)
      { 
        std::vector<int> partial_initial_grandchildren_pdgId;
        for (const xAOD::TruthParticle *grandchild : partial_initial_grandchildren)
        {
          partial_initial_grandchildren_pdgId.push_back(grandchild->pdgId());
        }
        pdgIds.push_back(partial_initial_grandchildren_pdgId);
      }
      return pdgIds;
    }

    std::vector<float> children_p4(int coordIdx)
    {
      std::vector<float> coords_pair;
      for (P4 child_p4 : m_children_p4)
      {
        std::array<float, 4> coords;
        child_p4.GetCoordinates(coords.begin());
        coords_pair.push_back(coords[coordIdx]);
      }
      return coords_pair;
    }

    std::vector<float> initial_children_p4(int coordIdx)
    {
      std::vector<float> coords_pair;
      for (P4 initial_child_p4 : m_initial_children_p4)
      {
        std::array<float, 4> coords;
        initial_child_p4.GetCoordinates(coords.begin());
        coords_pair.push_back(coords[coordIdx]);
      }
      return coords_pair;
    }

    std::vector<std::vector<float>> grandchildren_p4(int coordIdx)
    {
      std::vector<std::vector<float>> coords;
      for (std::vector<P4>& partial_grandchildren_p4 : m_grandchildren_p4)
      {
        std::vector<float> partial_grandchildren_coords;
        for ( P4 grandchild_p4 : partial_grandchildren_p4)
        {
          std::array<float, 4> grandchild_coords;
          grandchild_p4.GetCoordinates(grandchild_coords.begin());
          partial_grandchildren_coords.push_back(grandchild_coords[coordIdx]);
        }
        coords.push_back(partial_grandchildren_coords);
      }
      return coords;
    }

    std::vector<std::vector<float>> initial_grandchildren_p4(int coordIdx)
    {
      std::vector<std::vector<float>> coords;
      for (std::vector<P4>& partial_initial_grandchildren_p4 : m_initial_grandchildren_p4)
      {
        std::vector<float> partial_initial_grandchildren_coords;
        for ( P4 initial_grandchild_p4 : partial_initial_grandchildren_p4)
        {
          std::array<float, 4> initial_grandchild_coords;
          initial_grandchild_p4.GetCoordinates(initial_grandchild_coords.begin());
          partial_initial_grandchildren_coords.push_back(initial_grandchild_coords[coordIdx]);
        }
        coords.push_back(partial_initial_grandchildren_coords);
      }
      return coords;
    }
  };

  TruthParticleInformationAlg ::TruthParticleInformationAlg(
    const std::string &name, ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) { }

  StatusCode TruthParticleInformationAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (!m_truthParticleBSMInKey.empty())
      ATH_CHECK(m_truthParticleBSMInKey.initialize());

    if (!m_truthParticleSMInKey.empty())
      ATH_CHECK(m_truthParticleSMInKey.initialize());

    if (!m_truthParticleInfoOutKey.empty())
      ATH_CHECK(m_truthParticleInfoOutKey.initialize());

    ATH_CHECK(m_EventInfoKey.initialize());

    m_truthHiggsesKinDecorKeys.resize(m_nHiggses);
    m_truthChildrenKinFromHiggsesDecorKeys.resize(m_nHiggses);
    m_truthInitialChildrenKinFromHiggsesDecorKeys.resize(m_nHiggses);
    for (unsigned int h = 0; h < m_nHiggses; h++)
    {
      // decorator will show up as "truth_Hx_pdgId", where x is the x higgs
      m_truthHiggsesPdgIdDecorKeys.emplace_back
      (m_EventInfoKey,
        "truth_H" + std::to_string(h + 1) + "_" + "pdgId");
      m_truthChildrenPdgIdFromHiggsesDecorKeys.emplace_back
      (m_EventInfoKey,
        "truth_children_fromH" + std::to_string(h + 1) + "_" + "pdgId");
      m_truthInitialChildrenPdgIdFromHiggsesDecorKeys.emplace_back
      (m_EventInfoKey,
        "truth_initial_children_fromH" + std::to_string(h + 1) + "_" + "pdgId");

      for (const std::string &var : m_kinVars)
      {
        m_truthHiggsesKinDecorKeys[h].emplace_back
        (m_EventInfoKey,
          "truth_H" + std::to_string(h + 1) + "_" + var);
        m_truthChildrenKinFromHiggsesDecorKeys[h].emplace_back
        (m_EventInfoKey,
          "truth_children_fromH" + std::to_string(h + 1) + "_" + var);
        m_truthInitialChildrenKinFromHiggsesDecorKeys[h].emplace_back
        (m_EventInfoKey, 
          "truth_initial_children_fromH" + std::to_string(h + 1) + "_" + var);
      }
      ATH_CHECK(m_truthHiggsesKinDecorKeys[h].initialize());
      ATH_CHECK(m_truthChildrenKinFromHiggsesDecorKeys[h].initialize());
      ATH_CHECK(m_truthInitialChildrenKinFromHiggsesDecorKeys[h].initialize());
    }

    ATH_CHECK(m_truthHiggsesPdgIdDecorKeys.initialize());
    ATH_CHECK(m_truthChildrenPdgIdFromHiggsesDecorKeys.initialize());
    ATH_CHECK(m_truthInitialChildrenPdgIdFromHiggsesDecorKeys.initialize());

    if (m_recordGrandchildren)
    {
      m_truthGrandchildrenKinFromHiggsesDecorKeys.resize(m_nHiggses);
      m_truthInitialGrandchildrenKinFromHiggsesDecorKeys.resize(m_nHiggses);
      for (unsigned int h = 0; h < m_nHiggses; h++)
      {
        m_truthGrandchildrenPdgIdFromHiggsesDecorKeys.emplace_back
        (m_EventInfoKey,
          "truth_grandchildren_fromH" + std::to_string(h + 1) + "_" + "pdgId");
        m_truthInitialGrandchildrenPdgIdFromHiggsesDecorKeys.emplace_back
        (m_EventInfoKey,
          "truth_initial_grandchildren_fromH" + std::to_string(h + 1) + "_" + "pdgId");

        for (const std::string &var : m_kinVars)
        {
          m_truthGrandchildrenKinFromHiggsesDecorKeys[h].emplace_back
          (m_EventInfoKey,
            "truth_grandchildren_fromH" + std::to_string(h + 1) + "_" + var);
          m_truthInitialGrandchildrenKinFromHiggsesDecorKeys[h].emplace_back
          (m_EventInfoKey,
            "truth_initial_grandchildren_fromH" + std::to_string(h + 1) + "_" + var);
        }
        ATH_CHECK(m_truthGrandchildrenKinFromHiggsesDecorKeys[h].initialize());
        ATH_CHECK(m_truthInitialGrandchildrenKinFromHiggsesDecorKeys[h].initialize());
      }
    }
    ATH_CHECK(m_truthGrandchildrenPdgIdFromHiggsesDecorKeys.initialize(m_recordGrandchildren));
    ATH_CHECK(m_truthInitialGrandchildrenPdgIdFromHiggsesDecorKeys.initialize(m_recordGrandchildren));


    for (const auto& decayMode : m_decayModes)
    {
      if(decayModes_str_to_enum.find(decayMode)==decayModes_str_to_enum.end())
        ATH_MSG_ERROR("Decay mode "<<decayMode<<" is not supported");
      const auto& childParticles = decayProducts_IDs[decayModes_str_to_enum.at(decayMode)];
      m_targetPdgIDs.insert(childParticles.begin(),childParticles.end());
    }

    for (const std::string &var : m_kinVars)
    {
      m_truthHHKinDecorKeys.emplace_back(m_EventInfoKey, "truth_HH_" + var);
    }
    ATH_CHECK(m_truthHHKinDecorKeys.initialize());

    for (const std::string &average_var : m_kinAverageVars)
    {
      m_truthHHAverageKinDecorKeys.emplace_back(m_EventInfoKey, "truth_HH_" + average_var);
    }
    ATH_CHECK(m_truthHHAverageKinDecorKeys.initialize());

    m_absCosThetaStarDecorKey = m_EventInfoKey.key()+".truth_HH_" + m_absCosThetaStar;
    ATH_CHECK(m_absCosThetaStarDecorKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TruthParticleInformationAlg ::execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::EventInfo> 
      eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());
    
    SG::ReadHandle<xAOD::TruthParticleContainer> 
      truthSMParticles(m_truthParticleSMInKey);
    ATH_CHECK(truthSMParticles.isValid());

    SG::ReadHandle<xAOD::TruthParticleContainer> 
      truthBSMParticles(m_truthParticleBSMInKey);
    ATH_CHECK(truthBSMParticles.isValid());

    bool isMC = eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION);
    if (!isMC)
    {
      ATH_MSG_ERROR(
        "No truth particle information available in data, cannot build "
        << std::string(m_nHiggses, 'H') << " decay path!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(recordTruthParticleInformation(*truthBSMParticles,
                                             *truthSMParticles, *eventInfo));
    return StatusCode::SUCCESS;
  }

  StatusCode TruthParticleInformationAlg ::recordTruthParticleInformation(
    const xAOD::TruthParticleContainer &truthBSMParticles,
    const xAOD::TruthParticleContainer &truthSMParticles,
    const xAOD::EventInfo &eventInfo) const
  {
    ATH_MSG_DEBUG("Saving truth particles as \""
                  << m_truthParticleInfoOutKey.key() << "\".");

    // Check that BSM container is not empty, otherwise use SM container
    auto truthParticlesContainer =
      !truthBSMParticles.empty() ? truthBSMParticles : truthSMParticles;

    /*
      Find and record truth particle information
    */
    std::vector<TruthScalar> higgses =
      getFinalHiggses(truthParticlesContainer);
    /*
      Decorate truth particle information on EventInfo with defaults if higgses
      empty
    */
    decorateTruthParticleInformation(eventInfo, higgses);
    /*
      Write container with truth particles that will be available in other
      algorithms
    */
    auto higgsesTruthParticles =
      std::make_unique<ConstDataVector<xAOD::TruthParticleContainer>>(
        SG::VIEW_ELEMENTS);
    for (TruthScalar h : higgses)
    {
      if(!h.valid()) continue;
      debugPrintParticleKinematics(h.source());
      higgsesTruthParticles->push_back(h.source());
    }
    SG::WriteHandle<ConstDataVector<xAOD::TruthParticleContainer>> 
      writeHandle(m_truthParticleInfoOutKey);
    ATH_CHECK(writeHandle.record(std::move(higgsesTruthParticles)));
    return StatusCode::SUCCESS;
  }

  void TruthParticleInformationAlg ::decorateTruthParticleInformation(
    const xAOD::EventInfo &eventInfo, std::vector<TruthScalar>& higgses) const
  {
    if (higgses.size() < m_nHiggses)
    {
      // Default values
      higgses.resize(m_nHiggses, TruthScalar());
    }
    for (unsigned int h = 0; h < m_nHiggses; h++)
    {
      SG::WriteDecorHandle<xAOD::EventInfo, int> 
        truthHiggsesPdgIdDecorHandle(m_truthHiggsesPdgIdDecorKeys[h]);
      truthHiggsesPdgIdDecorHandle(eventInfo) = higgses[h].pdgId();

      SG::WriteDecorHandle<xAOD::EventInfo, std::vector<int>>
        truthChildrenPdgIdFromHiggsesDecorHandle(
          m_truthChildrenPdgIdFromHiggsesDecorKeys[h]);
      truthChildrenPdgIdFromHiggsesDecorHandle(eventInfo) = 
        higgses[h].children_pdgId();

      SG::WriteDecorHandle<xAOD::EventInfo, std::vector<int>>
        truthInitialChildrenPdgIdFromHiggsesDecorHandle(
          m_truthInitialChildrenPdgIdFromHiggsesDecorKeys[h]);
      truthInitialChildrenPdgIdFromHiggsesDecorHandle(eventInfo) =
        higgses[h].initial_children_pdgId();

      for (size_t i = 0; i < m_kinVars.size(); i++)
      {
        SG::WriteDecorHandle<xAOD::EventInfo, float> 
          truthHiggsesKinDecorHandle(m_truthHiggsesKinDecorKeys[h][i]);
        truthHiggsesKinDecorHandle(eventInfo) = higgses[h].p4(i);

        SG::WriteDecorHandle<xAOD::EventInfo, std::vector<float>>
          truthChildrenKinFromHiggsesDecorHandle(
            m_truthChildrenKinFromHiggsesDecorKeys[h][i]);
        truthChildrenKinFromHiggsesDecorHandle(eventInfo) =
          higgses[h].children_p4(i);

        SG::WriteDecorHandle<xAOD::EventInfo, std::vector<float>>
          truthInitialChildrenKinFromHiggsesDecorHandle(
            m_truthInitialChildrenKinFromHiggsesDecorKeys[h][i]);
        truthInitialChildrenKinFromHiggsesDecorHandle(eventInfo) =
          higgses[h].initial_children_p4(i);
      }
    }

    if (m_recordGrandchildren)
    {
      for (unsigned int h = 0; h < m_nHiggses; h++)
      {
        SG::WriteDecorHandle<xAOD::EventInfo, std::vector<std::vector<int>>>
          truthGrandchildrenPdgIdFromHiggsesDecorHandle(
            m_truthGrandchildrenPdgIdFromHiggsesDecorKeys[h]);
        truthGrandchildrenPdgIdFromHiggsesDecorHandle(eventInfo) =
          higgses[h].grandchildren_pdgId();
      
        SG::WriteDecorHandle<xAOD::EventInfo, std::vector<std::vector<int>>>
          truthInitialGrandchildrenPdgIdFromHiggsesDecorHandle(
            m_truthInitialGrandchildrenPdgIdFromHiggsesDecorKeys[h]);
        truthInitialGrandchildrenPdgIdFromHiggsesDecorHandle(eventInfo) =
          higgses[h].initial_grandchildren_pdgId();

          for (size_t i = 0; i < m_kinVars.size(); i++)
          {
            SG::WriteDecorHandle<xAOD::EventInfo, std::vector<std::vector<float>>>
              truthGrandchildrenKinFromHiggsesDecorHandle(
                m_truthGrandchildrenKinFromHiggsesDecorKeys[h][i]);
            truthGrandchildrenKinFromHiggsesDecorHandle(eventInfo) =
            higgses[h].grandchildren_p4(i);

            SG::WriteDecorHandle<xAOD::EventInfo, std::vector<std::vector<float>>>
              truthInitialGrandchildrenKinFromHiggsesDecorHandle(
                m_truthInitialGrandchildrenKinFromHiggsesDecorKeys[h][i]);
            truthInitialGrandchildrenKinFromHiggsesDecorHandle(eventInfo) =
              higgses[h].initial_grandchildren_p4(i);
          }
      }
    }

    // Reconstruct the HH system
    // Assume exactly two Higgs for now & check for nullptrs

    std::array<float, 4> coords = {-999., -999., -999., -999.};
    std::array<float, 2> ave_coords = {-999., -999.};
    float abs_cos_theta_star = -999.;
    if(higgses.size()>=2 && higgses[0].valid() && higgses[1].valid())
    {
        coords = calcHHKinematics(higgses[0].source(), higgses[1].source());
        ave_coords = calcHHAverageKinematics(higgses[0].source(), higgses[1].source());
        abs_cos_theta_star = calcHHCosThetaStar(higgses[0].source(), higgses[1].source());
    }
    ATH_MSG_DEBUG("got abs_cos_theta_star" << abs_cos_theta_star);

    for (size_t i = 0; i < m_kinVars.size(); i++)
    {
      SG::WriteDecorHandle<xAOD::EventInfo, float> 
        truthHHKinDecorHandle(m_truthHHKinDecorKeys[i]);
      truthHHKinDecorHandle(eventInfo) = coords[i];
    }

    for (size_t i = 0; i < m_kinAverageVars.size(); i++)
    {
      SG::WriteDecorHandle<xAOD::EventInfo, float> 
        truthHHAverageKinDecorHandle(m_truthHHAverageKinDecorKeys[i]);
      truthHHAverageKinDecorHandle(eventInfo) = ave_coords[i];
    }

    SG::WriteDecorHandle<xAOD::EventInfo, float> 
      absCosThetaStarDecorHandle(m_absCosThetaStarDecorKey);
    absCosThetaStarDecorHandle(eventInfo) = abs_cos_theta_star;

  }
  
  void TruthParticleInformationAlg::verbosePrintParticleAndChildren(
    const xAOD::TruthParticle *p, int counter = 1) const
  {
    // Failsafe to prevent infinite recursion if children go indefinitely
    if (counter == 100)
      return;
    ATH_MSG_VERBOSE("Particle " << p->index() << " pdgID " << p->pdgId()
                                << ", uniqueID " << p->uid()
                                << ", children " << p->nChildren());
    for (size_t i = 0; i < p->nChildren(); i++)
    {
      verbosePrintParticleAndChildren(p->child(i), counter + 1);
    };
  }

  void TruthParticleInformationAlg::debugPrintParticleKinematics(
    const xAOD::TruthParticle *p) const
  {
    ATH_MSG_DEBUG("Particle " << p->pdgId() << ", pt " << p->pt() << ", phi "
                              << p->phi() << ", eta " << p->eta() << ", mass "
                              << p->m());
  }

  std::vector<const xAOD::TruthParticle *>
  TruthParticleInformationAlg ::getFinalChildren(const xAOD::TruthParticle *p) const
  {
    std::vector<const xAOD::TruthParticle *> children;
    const xAOD::TruthParticle *tmp(nullptr);
    for (size_t i = 0; i < p->nChildren(); i++)
    {
      if (msgLvl(MSG::VERBOSE))
      {
        verbosePrintParticleAndChildren(p->child(i));
      }

      std::unordered_set<int> childrenPdgIds;
      int child_pdg = p->child(i)->pdgId();
      if(m_targetPdgIDs.contains(child_pdg))
        {
          childrenPdgIds.emplace(child_pdg);
        }

      const xAOD::TruthParticle *final_child =
        getFinalParticleOfType(p->child(i), childrenPdgIds);
      if (!tmp || (final_child->uid() != tmp->uid()))
      {
        tmp = final_child;
        children.push_back(final_child);
      }
    }
    return children;
  }

  std::vector<const xAOD::TruthParticle *>
  TruthParticleInformationAlg ::getInitialChildren(const xAOD::TruthParticle *p) const
  {
    std::vector<const xAOD::TruthParticle *> initial_children;
    for (size_t i = 0; i < p->nChildren(); i++)
    {
      if (msgLvl(MSG::VERBOSE))
      {
        verbosePrintParticleAndChildren(p->child(i));
      }

      initial_children.push_back(p->child(i));
    }
    return initial_children;

  }

  std::vector<std::vector<const xAOD::TruthParticle *>>
  TruthParticleInformationAlg ::getFinalGrandchildren(
    const std::vector<const xAOD::TruthParticle *>& children) const
  { 
    std::vector<std::vector<const xAOD::TruthParticle *>> grandchildren;
    std::vector<const xAOD::TruthParticle *> tmp_grandchildren;
    const xAOD::TruthParticle *tmp_grandchild(nullptr);

    // Hardcoding here, should we use the pdgid of the grandchild only?
    // But we don't want to differentiate decay modes of higgs children
    std::unordered_set<int> grandChildrenPdgIds = {
      MC::UQUARK, -MC::UQUARK, MC::DQUARK, -MC::DQUARK, MC::SQUARK, -MC::SQUARK, 
      MC::CQUARK, -MC::CQUARK, MC::BQUARK, -MC::BQUARK, 
      MC::ELECTRON, -MC::ELECTRON, MC::MUON, -MC::MUON, MC::TAU, -MC::TAU, 
      MC::NU_E, -MC::NU_E, MC::NU_MU, -MC::NU_MU, MC::NU_TAU, -MC::NU_TAU};

    for (const xAOD::TruthParticle *child : children)
    {
      tmp_grandchildren.clear();
      tmp_grandchild = nullptr;
      for (size_t i = 0; i < child->nChildren(); i++)
      {
        if (msgLvl(MSG::VERBOSE))
        {
          verbosePrintParticleAndChildren(child->child(i));
        }
        
        const xAOD::TruthParticle *final_grandchild =
          getFinalParticleOfType(child->child(i), grandChildrenPdgIds);
        if (!tmp_grandchild || (final_grandchild->uid() != tmp_grandchild->uid()))
        {
          tmp_grandchild = final_grandchild;
          tmp_grandchildren.push_back(final_grandchild);
        }
      }
      grandchildren.push_back(tmp_grandchildren);
    }

    return grandchildren;
  }

  std::vector<std::vector<const xAOD::TruthParticle *>>
  TruthParticleInformationAlg ::getInitialGrandchildren(
    const std::vector<const xAOD::TruthParticle *>& children) const
  {
    std::vector<std::vector<const xAOD::TruthParticle *>> initial_grandchildren;
    std::vector<const xAOD::TruthParticle *> tmp_initial_grandchildren;

    for (const xAOD::TruthParticle *child : children)
    {
      tmp_initial_grandchildren.clear();

        for (size_t i = 0; i < child->nChildren(); i++)
        {
          if (msgLvl(MSG::VERBOSE))
          {
            verbosePrintParticleAndChildren(child->child(i));
          }
          
          tmp_initial_grandchildren.push_back(child->child(i));
        }
        initial_grandchildren.push_back(tmp_initial_grandchildren);
    }
    
    return initial_grandchildren;
  }

  std::vector<TruthScalar> TruthParticleInformationAlg ::getFinalHiggses(
    const xAOD::TruthParticleContainer &truthParticlesContainer) const
  {
    std::vector<TruthScalar> higgses;
    const xAOD::TruthParticle *tmp(nullptr);
    for (const xAOD::TruthParticle *tp : truthParticlesContainer)
    {
      if (msgLvl(MSG::VERBOSE))
      {
        verbosePrintParticleAndChildren(tp);
      }
      if ((tp->pdgId() == MC::HIGGSBOSON || tp->pdgId() == MC::SBOSONBSM || tp->pdgId() == MC::ABOSONBSM))
      {
        const xAOD::TruthParticle *final_h =
          getFinalParticleOfType(tp, {MC::HIGGSBOSON, MC::SBOSONBSM, MC::ABOSONBSM});
        if (!tmp || (final_h->uid() != tmp->uid()))
        {
          TruthScalar h{final_h};
          if (m_recordGrandchildren)
          {
            auto children = getFinalChildren(final_h);
            h.grandchildren(getFinalGrandchildren(children));
            h.initial_grandchildren(getInitialGrandchildren(children));
            h.children(children);
            h.initial_children(getInitialChildren(final_h));
          }
          else 
          {
            h.children(getFinalChildren(final_h));
            h.initial_children(getInitialChildren(final_h));
          }
          tmp = final_h;
          // avoid SBOSONBSM if SBOSONBSM -> ABOSONBSM ABOSONBSM or ABOSONBSM HIGGSBOSON 
          if(final_h->pdgId() == MC::SBOSONBSM && final_h->nChildren() == 2 &&
            (h.initial_children_pdgId()[0] == MC::ABOSONBSM || 
              h.initial_children_pdgId()[1] == MC::ABOSONBSM)) continue;
          higgses.push_back(h);
        }
      }
    }

      std::sort(higgses.begin(), higgses.end(), [](TruthScalar &a, TruthScalar &b) {
        return a.p4(0) > b.p4(0); // biggest pT will be first in array
      });

    return higgses;
  }


  std::array<float, 4> TruthParticleInformationAlg::calcHHKinematics(
    const xAOD::TruthParticle *p1, const xAOD::TruthParticle *p2) const
  {
    ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>> h1;
    ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>> h2;
    h1.SetCoordinates(p1->pt(), p1->eta(), p1->phi(), p1->m());
    h2.SetCoordinates(p2->pt(), p2->eta(), p2->phi(), p2->m());
    auto hh = h1 + h2;

    ATH_MSG_DEBUG("Particle " << p1->pdgId() << ", pt " << p1->pt() << ", phi "
                                << p1->phi() << ", eta " << p1->eta() << ", mass "
                                << p1->m());
    ATH_MSG_DEBUG("Particle " << p2->pdgId() << ", pt " << p2->pt() << ", phi "
                                << p2->phi() << ", eta " << p2->eta() << ", mass "
                                << p2->m());
    ATH_MSG_DEBUG("Particle " << " pt " << hh.pt() << ", phi "
                                << hh.phi() << ", eta " << hh.eta() << ", mass "
                                << hh.M());

    std::array<float, 4> coords;
    hh.GetCoordinates(coords.begin());

    return coords;
  }


  std::array<float, 2> TruthParticleInformationAlg::calcHHAverageKinematics(
    const xAOD::TruthParticle *p1, const xAOD::TruthParticle *p2) const
  {
    float ave_pt = (p1->pt()+p2->pt())/2;
    float ave_eta =  (p1->eta()+p2->eta())/2;
    std::array<float, 2> ave_coords{ave_pt, ave_eta};

    ATH_MSG_DEBUG("Average pt " << ave_pt << "and eta" << ave_eta);

    return ave_coords;
  }

  float TruthParticleInformationAlg::calcHHCosThetaStar(
    const xAOD::TruthParticle *h1_in, const xAOD::TruthParticle *h2_in) const
  {
    TLorentzVector h1 = h1_in->p4();
    TLorentzVector h2 = h2_in->p4();
    TLorentzVector CM = 0.5*(h1+h2);

    TLorentzVector h1_star = h1-CM;
    TLorentzVector h2_star = h2-CM;

    ATH_MSG_DEBUG("check if two higges are opposite in CM");
    ATH_MSG_DEBUG("get x* h1,h2" << h1_star.X() << " , " << h2_star.X()
                  << " y* h1,h2 " << h1_star.Y() << " , " << h2_star.Y()
                  << " z* h1,h2 " << h1_star.Z() << " , " << h2_star.Z()
                  );

	  // take whatever out of h1,h2 thetas as abs(cos) anyway
    return std::abs(cos(h1_star.Theta()));
  }

}
