/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author

#include "BaselineVarsZbbyCalibAlg.h"

namespace {
  // To do: make this somethig more standard like NAN
  const float def_float = -99;
  // To do: make this something more standard like -1
  const int def_int = -99;
}

namespace XBBCALIB
{

  class FourVectorOutBlock
  {
  private:
    CP::SysWriteDecorHandle<float> m_pt;
    CP::SysWriteDecorHandle<float> m_eta;
    CP::SysWriteDecorHandle<float> m_phi;
    CP::SysWriteDecorHandle<float> m_m;
  public:
    template <typename T>
    FourVectorOutBlock(
      T* owner,
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
    void set(const xAOD::EventInfo& event, const xAOD::IParticle& part,
             const CP::SystematicSet& sys) {
      m_pt.set(event, part.pt(), sys);
      m_eta.set(event, part.eta(), sys);
      m_phi.set(event, part.phi(), sys);
      m_m.set(event, part.m(), sys);
    }
    void setDefault(const xAOD::EventInfo& event,
                    const CP::SystematicSet& sys) {
      m_pt.set(event, def_float, sys);
      m_eta.set(event, def_float, sys);
      m_phi.set(event, def_float, sys);
      m_m.set(event, def_float, sys);
    }
  };


  BaselineVarsZbbyCalibAlg::BaselineVarsZbbyCalibAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  { }

  BaselineVarsZbbyCalibAlg::~BaselineVarsZbbyCalibAlg() = default;

  StatusCode BaselineVarsZbbyCalibAlg::initialize()
  {
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));

    ATH_CHECK(m_nLRJetsHandle.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_nPhotonsHandle.initialize(m_systematicsList, m_eventHandle));

    m_photon_4vec = std::make_unique<FourVectorOutBlock>(
      this, "photon_", m_systematicsList, m_eventHandle);
    m_z_candidate_4vec = std::make_unique<FourVectorOutBlock>(
      this, "Zcand_", m_systematicsList, m_eventHandle);

    // set up the generic float copying
    for (const auto& var: m_floats_to_copy) {
      auto& rwpair = m_float_copy_pairs.emplace_back(
        std::make_unique<rw_pair_t<float>>(
          SRDH_t<float>{var, this},
          SWDH_t<float>{m_copied_variable_prefix + var + "_%SYS%", this})
        );
      ATH_CHECK(rwpair->first.initialize(m_systematicsList, m_lrjetHandle));
      ATH_CHECK(rwpair->second.initialize(m_systematicsList, m_eventHandle));
    }
    // set up the generic int copying
    for (const auto& var: m_ints_to_copy) {
      auto& rwpair = m_int_copy_pairs.emplace_back(
        std::make_unique<rw_pair_t<int>>(
          SRDH_t<int>{var, this},
          SWDH_t<int>{m_copied_variable_prefix + var + "_%SYS%", this})
        );
      ATH_CHECK(rwpair->first.initialize(m_systematicsList, m_lrjetHandle));
      ATH_CHECK(rwpair->second.initialize(m_systematicsList, m_eventHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsZbbyCalibAlg::execute()
  {
    //Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve(photons, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      // selected Probe Jet ;
      if (lrjets->size() >= 1)
      {
        const xAOD::Jet* largeJet = lrjets->at(0);
        m_z_candidate_4vec->set(*event, *largeJet, sys);
        // copy configurable variables
        for (const auto& pair: m_float_copy_pairs) {
          pair->second.set(*event, pair->first.get(*largeJet, sys), sys);
        }
        for (const auto& pair: m_int_copy_pairs) {
          pair->second.set(*event, pair->first.get(*largeJet, sys), sys);
        }
      } else {
        m_z_candidate_4vec->setDefault(*event, sys);
        for (const auto& pair: m_float_copy_pairs) {
          pair->second.set(*event, def_float, sys);
        }
        for (const auto& pair: m_int_copy_pairs) {
          pair->second.set(*event, def_int, sys);
        }
      }
      m_nLRJetsHandle.set(*event, lrjets->size(), sys);
      m_nPhotonsHandle.set(*event, photons->size(), sys);
      // This should be ok, as exactly one photon req.
      if (photons->size() >= 1) {
        m_photon_4vec->set(*event, *photons->at(0), sys);
      } else {
        m_photon_4vec->setDefault(*event, sys);
      }

    }
    return StatusCode::SUCCESS;
  }

}



