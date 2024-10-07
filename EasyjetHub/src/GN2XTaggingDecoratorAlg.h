/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 Algorithm to decorate large-R jet with GN2X operating points and scale factors
 To be ultimately replaced by a CP algorithm in Athena
*/

#ifndef EASYJET_GN2XTAGGINGDECORATORALG
#define EASYJET_GN2XTAGGINGDECORATORALG

#include <AthenaBaseComps/AthAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <xAODJet/JetContainer.h>

#include "FTagAnalysisInterfaces/IBTaggingSelectionJsonTool.h"
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyJsonTool.h"

namespace Easyjet
{

  class GN2XTaggingDecoratorAlg final : public AthAlgorithm
  {
  public:
    GN2XTaggingDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;

  private:
    SG::ReadHandleKey<xAOD::JetContainer> m_jetsInKey
      {this, "jetsIn", "", "containerName to read"};

    ToolHandle<IBTaggingSelectionJsonTool> m_seltool
      {this, "SelectionTool", "", "BTagging Selection Json Tool"};
    ToolHandle<IBTaggingEfficiencyJsonTool> m_efftool
      {this, "EfficiencyTool", "", "BTagging Efficiency Json Tool"};

    StringProperty m_WPname{this, "WPname", "", "GN2X WP name for decoration"};
    BooleanProperty m_isMC{this, "isMC", true};
    BooleanProperty m_doSyst{this, "doSyst", false, "Enable systematics for SF"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_GN2XTagDecorKey;
    std::vector<SG::WriteDecorHandleKey<xAOD::JetContainer>> m_GN2XSFDecorKeys;
    
  };
}


#endif
