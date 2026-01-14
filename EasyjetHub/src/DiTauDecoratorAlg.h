/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_DITAUDECORATORALG
#define EASYJET_DITAUDECORATORALG

#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODTau/DiTauJetContainer.h>

#include <DiTauRecTools/DiTauIDVarCalculator.h>
#include <DiTauRecTools/DiTauOnnxDiscriminantTool.h>


namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class DiTauDecoratorAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
  public:
    DiTauDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:

    // Taus
    SG::ReadHandleKey<xAOD::DiTauJetContainer> m_diTausInKey{
      this, "diTausIn", "", "containerName to read"
    };

    SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_scoreDecorKey;

    ToolHandle<DiTauRecTools::DiTauIDVarCalculator> m_diTauIDVarCalculator{
      this, "DiTauIDVarCalculator", "", "DiTau ID variable calculator tool"};

    ToolHandle<DiTauRecTools::DiTauOnnxDiscriminantTool> m_diTauOnnxDiscriminantTool{
      this, "DiTauOnnxDiscriminantTool", "", "DiTau ONNX discriminant  tool"};
    
  };

}

#endif
