/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Evil Teng Jian Khoo

#include "JetDeepCopyAlg.h"

namespace Easyjet
{
  JetDeepCopyAlg ::JetDeepCopyAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode JetDeepCopyAlg ::initialize()
  {
    ATH_CHECK(m_jetsInKey.initialize());
    ATH_CHECK(m_jetsOutKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode JetDeepCopyAlg ::execute(const EventContext& ctx) const
  {

    SG::ReadHandle jetsIn(m_jetsInKey,ctx);
    ATH_CHECK (jetsIn.isValid());
    SG::WriteHandle jetsOut(m_jetsOutKey, ctx);
    ATH_CHECK(jetsOut.record(
                std::make_unique<Cont>(),std::make_unique<ContAux>()));

    for(const Item* jet : *jetsIn) {
      *jetsOut->emplace_back(new Item()) = *jet;
    }

    return StatusCode::SUCCESS;
  }
}
