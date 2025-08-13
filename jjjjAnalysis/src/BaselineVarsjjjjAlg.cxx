/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner ---> Antonio Giannini

#include "AthContainers/AuxElement.h"
#include "BaselineVarsjjjjAlg.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include "getThrust.h"

namespace jjjj
{
  BaselineVarsjjjjAlg ::BaselineVarsjjjjAlg(const std::string &name,
                                                    ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    //declareProperty("bTagWP", m_bTagWP);
  }

  StatusCode BaselineVarsjjjjAlg ::initialize()
  {
    ATH_CHECK (m_SmallRJetsHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
  // make decorators
    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &var : m_intVariables){
      ATH_MSG_DEBUG("initializing integer variable: " << var);
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };
    
    if (m_isMC){
      ATH_CHECK (m_PTLID.initialize(m_systematicsList, m_SmallRJetsHandle));
    }
    
      ATH_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;

  }

  StatusCode BaselineVarsjjjjAlg ::execute()
  {
    for (const auto& sys : m_systematicsList.systematicsVector()){
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));
      const xAOD::JetContainer *SmallRJets = nullptr;
      ANA_CHECK (m_SmallRJetsHandle.retrieve (SmallRJets, sys));

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }
      int n_Jets = SmallRJets -> size();
      if (n_Jets < 4) continue;       
      m_Ibranches.at("nJets").set(*event, n_Jets, sys);

      TLVs jetP4s;
      // jet index in pT order
      int index = 0;
      for ( auto jet : *SmallRJets ){
        index++;
        if (index > 4){
          break; // we want only four jets
        }
        jetP4s.push_back(jet->p4());
        m_Fbranches.at("Jet" + std::to_string(index) + "_pt").set(*event, jet->pt(), sys);
        m_Fbranches.at("Jet" + std::to_string(index) + "_eta").set(*event, jet->eta(), sys);
        m_Fbranches.at("Jet" + std::to_string(index) + "_phi").set(*event, jet->phi(), sys);
        m_Fbranches.at("Jet" + std::to_string(index) + "_E").set(*event, jet->e(), sys); 
        if (m_isMC){
          int PTLID = -99;
          PTLID = m_PTLID.get(*jet, sys);
          m_Ibranches.at("jjjj_Jet" + std::to_string(index) + "_PTLID").set(*event, PTLID, sys);
        }
      }

      // Compute eigenvalues of normalized jet momentum tensor
      // construct normalized momentum tensor
      auto M = get3DMomentumTensor(jetP4s);
      
      // get eigenvalues of 3D momentum tensor
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigenSolver(M);
      Eigen::Vector3d eigenValues = eigenSolver.eigenvalues();
      std::vector<double> eigenVals = {eigenValues[0], eigenValues[1], eigenValues[2]};
      std::sort(eigenVals.begin(), eigenVals.end());
      
      std::vector<std::vector<double>> thrust = calcThrust(jetP4s);
      double transverseThrust = thrust[0][0];
      double thrustMinor = thrust[0][1];
      
      m_Fbranches.at("transverseThrust").set(*event, 1 - transverseThrust,sys);
      m_Fbranches.at("thrustMinor").set(*event,thrustMinor,sys);
      
      // get eigenvalues of 2D momentum tensor
      auto Mxy = get2DMomentumTensor(jetP4s);
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d> eigenSolverxy(Mxy);
      Eigen::Vector2d eigenValuesxy = eigenSolverxy.eigenvalues();
      std::vector<double> mus = {eigenValuesxy[0], eigenValuesxy[1]};
      std::sort(mus.begin(), mus.end());
      double transverseSphericity = 2.0*mus[0]/(mus[0] + mus[1]);
      
      m_Fbranches.at("transverseSphericity").set(*event,transverseSphericity,sys);
      
      // assign values
      m_Fbranches.at("Q1").set(*event,eigenVals[0],sys);
      m_Fbranches.at("Q2").set(*event,eigenVals[1],sys);
      m_Fbranches.at("Q3").set(*event,eigenVals[2],sys);
    }

    return StatusCode::SUCCESS;
  }

  Eigen::Matrix3d BaselineVarsjjjjAlg::get3DMomentumTensor (TLVs const &jetP4s){
    Eigen::Matrix3d M = Eigen::Matrix3d::Zero();
    double pmagTotal = 0.;
    for (auto jetP4:jetP4s){
      double px = jetP4.Px(), py = jetP4.Py(), pz = jetP4.Pz();
      double pmag = jetP4.P();
      pmagTotal += pmag;
      M(0, 0) += px*px/pmag; M(0, 1) += px*py/pmag; M(0, 2) += px*pz/pmag;
      M(1, 1) += py*py/pmag; M(1, 2) += py*pz/pmag;
      M(2, 2) += pz*pz/pmag;
    }
    M(0, 0) /= pmagTotal; M(0, 1) /= pmagTotal; M(0, 2) /= pmagTotal;
    M(1, 1) /= pmagTotal; M(1, 2) /= pmagTotal;
    M(2, 2) /= pmagTotal;
    M(1, 0) = M(0, 1); M(2, 0) = M(0, 2); M(2, 1) = M(1, 2);
    return M;
  }

  Eigen::Matrix2d BaselineVarsjjjjAlg::get2DMomentumTensor (TLVs const &jetP4s){
    Eigen::Matrix2d Mxy = Eigen::Matrix2d::Zero();
    double pmagTotal = 0.;
    for (auto jetP4:jetP4s){
      double px = jetP4.Px(), py = jetP4.Py();
      double pmag = jetP4.P();
      pmagTotal += pmag;
      Mxy(0, 0) += px*px/pmag; Mxy(0, 1) += px*py/pmag;
      Mxy(1, 1) += py*py/pmag; 
    }
      Mxy(0, 0) /= pmagTotal; Mxy(0, 1) /= pmagTotal;
      Mxy(1, 1) /= pmagTotal;
      Mxy(1, 0) = Mxy(0, 1);
    return Mxy;
  }
}
