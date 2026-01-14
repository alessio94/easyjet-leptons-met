/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  GNXLargeJetDecoratorAlg:
  An alg to add the discriminant value for the GNX taggers 
*/

// Always protect against multiple includes!
#ifndef EASYJET_GNXLARGEJETDECORATORALG
#define EASYJET_GNXLARGEJETDECORATORALG

#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODJet/JetContainer.h>

namespace Easyjet
{
  /// \brief An algorithm for counting containers
  class GNXLargeJetDecoratorAlg final : public AthReentrantAlgorithm {

    public:
      GNXLargeJetDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute(const EventContext& ctx) const override;
      /// We use default finalize() -- this is for cleanup, and we don't do any

    private:
     
      SG::ReadHandleKey<xAOD::JetContainer> m_jetsInKey {
        this, "jetsIn", "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",   "Large-R jet container" 
      };

      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN2XTauV00_phtautauhadDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN2XTauV00_phbbDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN2XTauV00_phccDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN2XTauV00_pqcdDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN2XTauV00_ptopDecorKey;
      std::map<std::string, SG::ReadDecorHandleKey<xAOD::JetContainer>> m_GN2XTauV00_decorKeys = {
          { "phtautauhad", m_GN2XTauV00_phtautauhadDecorKey},
          { "phbb",     m_GN2XTauV00_phbbDecorKey},
          { "phcc",     m_GN2XTauV00_phccDecorKey},
          { "pqcd",     m_GN2XTauV00_pqcdDecorKey},
          { "ptop",     m_GN2XTauV00_ptopDecorKey},
      };
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_GN2XTauV00_htt_scoreDecorKey;
    
      float compute_GN2XTauV00_htt_score(
        float phtautauhad_score, float phbb_score, float phcc_score, float ptop_score, float pqcd_score, 
        float f_Hbb=0.02, float f_Hcc=0.02, float f_top=0.15
      ) const {
        return std::log(phtautauhad_score / (f_Hbb * phbb_score + f_Hcc * phcc_score + f_top * ptop_score + (1.0f - f_top - f_Hcc - f_Hbb) * pqcd_score));
      }
  
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_phtautauhadDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_phbbDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_phccDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_pqcdbxDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_pqcdbbDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_pqcdcxDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_pqcdllDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_ptopDecorKey;
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_pWqqDecorKey;
      std::map<std::string, SG::ReadDecorHandleKey<xAOD::JetContainer>> m_GN3XPV01_decorKeys = {
          { "phtautauhad", m_GN3XPV01_phtautauhadDecorKey},
          {"phbb",   m_GN3XPV01_phbbDecorKey},
          {"phcc",   m_GN3XPV01_phccDecorKey},
          {"pqcdbx", m_GN3XPV01_pqcdbxDecorKey},
          {"pqcdbb", m_GN3XPV01_pqcdbbDecorKey},
          {"pqcdcx", m_GN3XPV01_pqcdcxDecorKey},
          {"pqcdll", m_GN3XPV01_pqcdllDecorKey},
          {"ptop",   m_GN3XPV01_ptopDecorKey},
          {"pWqq",   m_GN3XPV01_pWqqDecorKey}
      };
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_GN3XPV01_htt_scoreDecorKey;
    
      float compute_GN3XPV01_htt_score(
        float phtautauhad_score, 
        float phbb_score, float phcc_score, float ptop_score, float pWqq_score,
        float pqcdbx_score, float pqcdbb_score, float pqcdcx_score, float pqcdll_score,
        float f_Hbb=0.02, float f_Hcc=0.02, float f_Wqq=0.02, float f_top=0.15
      ) const {
        float f_qcd = (1.0f - f_top - f_Hcc - f_Hbb - f_Wqq);
        float pqcd_score =  (pqcdbx_score + pqcdbb_score + pqcdcx_score + pqcdll_score);
        return std::log(
          phtautauhad_score / (
            f_Hbb * phbb_score + f_Hcc * phcc_score + f_Wqq * pWqq_score + f_top * ptop_score + f_qcd * pqcd_score
          )
        );
      }

  };

}

#endif
