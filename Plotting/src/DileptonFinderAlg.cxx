///////////////////////// -*- C++ -*- /////////////////////////////
/// @author TJ Khoo

//
// includes
//

// Class definition
#include "DileptonFinderAlg.h"
#include <memory>    // for unique_ptr
#include <algorithm> // for sorting

//
// method implementations
//

namespace MSA
{
  DileptonFinderAlg ::
      DileptonFinderAlg(const std::string &name, ISvcLocator *pSvcLocator)
      : AthAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode DileptonFinderAlg ::
      initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (m_leptonsInKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided!");
      return StatusCode::FAILURE;
    }
    if (m_leptonsOutKey.empty())
    {
      ATH_MSG_ERROR("No output collection provided!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(m_leptonsInKey.initialize());
    ATH_CHECK(m_leptonsOutKey.initialize());

    ATH_MSG_INFO("Will search \"" << m_leptonsInKey.key() << "\" for lepton pair");
    ATH_MSG_INFO("Will write dileptons to \"" << m_leptonsOutKey.key() << "\"");

    return StatusCode::SUCCESS;
  }

  StatusCode DileptonFinderAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    // We don't actually have to know the final type,
    // but can just retrieve the base type.
    SG::ReadHandle<xAOD::IParticleContainer> leptons(m_leptonsInKey);
    ATH_CHECK(leptons.isValid());

    if (leptons->size() < 2)
    {
      setFilterPassed(false);
    }
    else
    {
      setFilterPassed(true);
      ATH_CHECK(recordLeadingLeptonPair(*leptons));
    }

    return StatusCode::SUCCESS;
  }

  StatusCode DileptonFinderAlg ::
      recordLeadingLeptonPair(const xAOD::IParticleContainer &leptons)
  {
    ATH_MSG_DEBUG("Saving leading lepton pair as \"" << m_leptonsOutKey.key() << "\".");

    // Typedef for readability later
    // All xAOD::BlahContainers inherit from DataVector, which you
    // can think of as a vector of pointers that owns the pointers
    // (and will delete them when it is destructed).
    // DataVector only receives a non-const object, so we have a
    // different container type to which we pass const objects,
    // ConstDataVector (templated to the owning container type).
    typedef ConstDataVector<xAOD::IParticleContainer> CDV_IPart;

    // We want the leading two leptons in pt, so we'll partially
    // sort the container (this is faster than a full sort),
    // but to do so we need a non-const container (can't sort the
    // input container that we retrieved).
    CDV_IPart sortedLeptons(leptons.begin(), leptons.end(), SG::VIEW_ELEMENTS);
    // Use C++ Standard Template Libraries (STL) for optimised
    // algorithms that implement frequently-used procedures
    // Defining a lambda function for comparisons
    std::partial_sort(sortedLeptons.begin(),     // Iterator from which to start sorting
                      sortedLeptons.begin() + 2, // Use begin + N to sort first N
                      sortedLeptons.end(),       // Iterator marking the end of the range to sort
                      [](const xAOD::IParticle *left, const xAOD::IParticle *right)
                      { return left->pt() > right->pt(); });

    // Verify that we got the sort right
    // Explicitly exclude code from execution unless
    // message level is requested to skip the loop
    if (msgLvl(MSG::VERBOSE))
    {
      ATH_MSG_VERBOSE("Printing lepton pts before sorting.");
      for (const xAOD::IParticle *lep : leptons)
      {
        ATH_MSG_VERBOSE("  " << lep->pt());
      }
    } //
    ATH_MSG_DEBUG("First lepton pt: " << sortedLeptons[0]->pt());
    ATH_MSG_DEBUG("Second lepton pt: " << sortedLeptons[1]->pt());

    // Now we can make another container to hold just the items we care about
    //
    // Previously, we always made stuff on the stack, or retrieved a
    // pointer held by somebody else. Here, we need to hand the memory
    // over to the store, so we need to create a new object on the heap.
    // Use std::unique_ptr to avoid memory leaks!
    //
    // Passing SG::VIEW_ELEMENTS to the constructor means that this
    // container will not own its contents (and therefore need to
    // manage the corresponding memory). The default is OWN_ELEMENTS.
    auto p_leadingpair = std::make_unique<CDV_IPart>(SG::VIEW_ELEMENTS);

    p_leadingpair->push_back(leptons[0]);
    p_leadingpair->push_back(leptons[1]);

    // Just as we can retrieve from the evtStore(), we can record to it.
    // There's no separation like with TEvent/TStore -- everything goes
    // into StoreGate, then we configure what should be written to file.
    // We need to release the memory from the ownership of the uniqe_ptr here,
    // as the store takes over memory management.
    SG::WriteHandle<ConstDataVector<xAOD::IParticleContainer>> leadingpair(m_leptonsOutKey);
    ATH_CHECK(leadingpair.record(std::move(p_leadingpair)));
    // When we retrieve this container, we can actually get it back as
    // a const IParticleContainer -- the ConstDataVector stuff is just
    // to fill it; there's a method on the class called asDataVector().

    return StatusCode::SUCCESS;
  }

}
