/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Teng Jian Khoo

#include "BTaggingDecoratorAlg.h"

namespace Easyjet
{
  BTaggingDecoratorAlg ::BTaggingDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator),
        m_btagLinkAcc(m_btagLinkName)
  {
  
  }

  StatusCode BTaggingDecoratorAlg ::initialize()
  {
    ATH_CHECK (m_jetsInKey.initialize());
    m_btagLinkAcc = SG::AuxElement::ConstAccessor<ElementLink<xAOD::BTaggingContainer> >(m_btagLinkName);

    // Build ConstAccessor/Decorator pairs for each
    // variable to be copied
    for(const std::string& var : m_floatVars) {
      m_floatHandlers.emplace_back(var,var);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode BTaggingDecoratorAlg ::execute(const EventContext& ctx) const
  {

    SG::ReadHandle<xAOD::JetContainer> jetsIn(m_jetsInKey,ctx);
    ATH_CHECK (jetsIn.isValid());

    for(const xAOD::Jet* jet : *jetsIn) {
      const xAOD::BTagging* btag = *m_btagLinkAcc(*jet);

      // Read the variable from the b-tag object and decorate on the jet
      for(const auto&[cacc,dec] : m_floatHandlers) {
          dec(*jet) = cacc(*btag);
      }

      // loop  over the floatVars_exp and intVars_exp and decorate the jet
      if (m_doExp) {
        SG::AuxElement::Decorator<float> DbExp("GN2v00_Db");
        SG::AuxElement::Decorator<int> pcbtExp("GN2v00_pcbtExp");

        // evaluate Db score of GN2v00
        float Db = evaluate_Db(*jet);

        // assign an experimental pcbt score
        int pcbt = evaluate_pcbtExp(Db);

        // decorate the jet
        DbExp(*jet) = Db;
        pcbtExp(*jet) = pcbt;
      }
    }

    return StatusCode::SUCCESS;
  }

  float BTaggingDecoratorAlg ::evaluate_Db(const xAOD::Jet& jet) const {
    static const SG::AuxElement::ConstAccessor<float> pu("GN2v00_pu");
    static const SG::AuxElement::ConstAccessor<float> pc("GN2v00_pc");
    static const SG::AuxElement::ConstAccessor<float> pb("GN2v00_pb");
    float fc = 0.1; // HARDCODED fc value for GN2

    return log( (pb(jet) ) / ( fc * pc(jet) + (1-fc) * pu(jet) ) );
  }

  int BTaggingDecoratorAlg ::evaluate_pcbtExp(float Db) const {
    int PCBT = 0;         // 100-89% -> 0
    if (Db > 0.783) PCBT++; // 89-85%  -> 1
    if (Db > 1.638) PCBT++; // 85-82%  -> 2
    if (Db > 2.145) PCBT++; // 82-77%  -> 3
    if (Db > 2.893) PCBT++; // 77-75%  -> 4
    if (Db > 3.034) PCBT++; // 75-70%  -> 5
    if (Db > 3.875) PCBT++; // 70-68%  -> 6
    if (Db > 4.157) PCBT++; // 68-60%  -> 7
    if (Db > 5.394) PCBT++; // 60-0%   -> 8

    return PCBT;
  }
}
