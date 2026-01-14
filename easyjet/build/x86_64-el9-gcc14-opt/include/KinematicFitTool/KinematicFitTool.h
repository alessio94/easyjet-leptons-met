///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// KinematicFitTool.h
// header file for class KinematicFitTool
// Author: BELFKIR Mohamed, STREBLER Thomas
// Email : mohamed.belfkir@cern.ch, thomas.strebler@cern.ch
///////////////////////////////////////////////////////////////////

#ifndef KinematicFitTool_H
#define KinematicFitTool_H

// KinematicFitTool includes
#include <string.h>
#include "AsgTools/AsgTool.h"
#include "AsgMessaging/AsgMessaging.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"

#include <AthenaKernel/Units.h>

#include "TMinuit.h"


class KinematicFitTool : public asg::AsgTool {

  ASG_TOOL_CLASS0(KinematicFitTool)

  public:

  /// Constructor with parameter name: 
  KinematicFitTool(const std::string& name = "KinematicFitTool");

  /// Destructor: 
  ~KinematicFitTool(){s_instance = nullptr;};
  StatusCode initialize() override;

  StatusCode applyKF(const xAOD::PhotonContainer& photons,
		     xAOD::JetContainer& jets,
		     double& KF_Mbb);

  StatusCode applyKF_bb4l(const xAOD::ElectronContainer &electrons,
           const xAOD::MuonContainer &muons,
           xAOD::JetContainer &jets, double &KF_Mbb);

  StringProperty m_mode{this, "mode", "bbyy", "corresponding final state"};

  const std::vector<xAOD::JetFourMom_t>& getIter1Jets() const { return m_iter1_jets; }
  const std::vector<xAOD::JetFourMom_t>& getIter2Jets() const { return m_iter2_jets; }

private:

  StringProperty m_JetAlgo{this, "JetCollection", "AntiKt4EMPFlow"};
  StringProperty m_BtaggingWP
    {this, "bTagWPDecorName", "ftag_select_GN2v00LegacyWP_FixedCutBEff_77"};
  DoubleProperty m_Jet_Min_Pt{this, "JetMinPt", 20.*Athena::Units::GeV};
  BooleanProperty m_isFixAngles{this, "FixAnglesFit", true};
  BooleanProperty m_isRun3{this, "isRun3", true};

  StatusCode runKinematicFit(xAOD::Jet* bjet1, xAOD::Jet* bjet2, std::vector<xAOD::Jet*>& extra_jets, double& KF_Mbb);
  DoubleProperty lambda {this, "LambdaMomConstraint", 0.0, "Lambda parameter for momentum constraint in kinematic fit"};
  DoubleProperty lambda_m {this, "LambdaMassConstraint", 0.0, "Lambda parameter for mass constraint in kinematic fit"};
  double MomConstraint_bb4l(double x, int nAddJets, bool PxOrPy);
  double MomConstraint_bbyy(double FitJetp, int nAddJets, bool PxOrPy);
  void calculateHHMomentum(const TLorentzVector &FitBJet1, const TLorentzVector &FitBJet2, double &PxHH, double &PyHH) const;

  std::vector<double> m_par_tensor_E[4][6];
  std::vector<double> m_par_tensor_pT[4][6];
  std::vector<double> m_par_tensor_pXconstr[4];
  std::vector<double> m_par_tensor_pYconstr[4];

  // const std::vector<float> m_logPT = {2.0,3.7,4.0,4.5,5.0,5.3,6.0};

  std::vector<xAOD::JetFourMom_t> m_iter1_jets;
  std::vector<xAOD::JetFourMom_t> m_iter2_jets;

  StringProperty m_EnRespFilename{this, "EnRespFilename", "", "corresponding final name"};
  StringProperty m_PtRespFilename{this, "PtRespFilename", "", "corresponding final name"};
  StringProperty m_pXconstrFilename{this, "pXconstrFilename", "", "corresponding final name"};
  StringProperty m_pYconstrFilename{this, "pYconstrFilename", "", "corresponding final name"};

  // for bbyy
  FloatArrayProperty m_logpT{this, "log_pt_binning", {}};

  FloatArrayProperty m_logE_Barrel{this, "Barrel_E_binning", {}};
  FloatArrayProperty m_logE_Crack{this, "Crack_E_binning", {}};
  FloatArrayProperty m_logE_Endcap{this, "Endcap_E_binning", {}};
  FloatArrayProperty m_logE_NoTrack{this, "No_Track_E_binning", {}};

  FloatArrayProperty m_logPT_Barrel{this, "Barrel_pT_binning", {}};
  FloatArrayProperty m_logPT_Crack{this, "Crack_pT_binning", {}};
  FloatArrayProperty m_logPT_Endcap{this, "Endcap_pT_binning", {}};
  FloatArrayProperty m_logPT_NoTrack{this, "No_Track_pT_binning", {}};
  
  struct Event {
    const xAOD::Photon* photon1;
    const xAOD::Photon* photon2;
    const xAOD::IParticle* lepton1;
    const xAOD::IParticle* lepton2;
    const xAOD::IParticle *lepton3;
    const xAOD::IParticle *lepton4;
    xAOD::Jet* bjet1;
    xAOD::Jet* bjet2;
    std::vector<xAOD::Jet*> extra_jets;
    int iteration;
  };

  Event m_Event;
  int m_minuit_ierflg;
  static KinematicFitTool* s_instance;

  double TaOgataResponse(const TLorentzVector& FitJet,
			 const xAOD::Jet* Jet, bool EorPT);

  double MomConstraint(double FitJetp, int nAddJets, bool PxOrPy);

  static void FCN(int& /*npar*/, double* /*grad*/, double& LH, double* par, int /*flag*/);

  void FCNImpl(double& LH, double* par);

  void SetParameterNameAndRange(TMinuit* m,
				const xAOD::Jet* bjet1, const xAOD::Jet* bjet2,
				const std::vector<xAOD::Jet*>& extra_jets);
  void ShareFit(const TMinuit* minuit, xAOD::Jet* bjet1, xAOD::Jet* bjet2,
		std::vector<xAOD::Jet*>& extra_jets,
		double& KF_Mbb, int iteration);

}; 

#endif //> !KinematicFitTool_H