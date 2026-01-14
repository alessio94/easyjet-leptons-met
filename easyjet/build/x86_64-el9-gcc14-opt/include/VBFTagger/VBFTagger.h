///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2019-2024 CERN for the benefit of the ATLAS collaboration
*/

// VBFTagger.h
// header file for class VBFTagger
// Author: Antonio Giannini
// Email : antonio.giannini@cern.ch
///////////////////////////////////////////////////////////////////

#ifndef VBFTagger_H
#define VBFTagger_H 1

// VBFTagger includes
#include "VBFTagger/IVBFTagger.h"

#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include "AsgMessaging/AsgMessaging.h"
#include "xAODJet/JetContainer.h"

#include <string.h>
#include <fstream>
#include <numeric>

// First include the class that does the computation
#include "lwtnn/LightweightGraph.hh"
#include "lwtnn/LightweightNeuralNetwork.hh"
// Then include the json parsing functions
#include "lwtnn/parse_json.hh"
#include "lwtnn/Stack.hh"

class VBFTagger : public virtual IVBFTagger, public asg::AsgTool {

	ASG_TOOL_CLASS1(VBFTagger, IVBFTagger)

    public:

      /// Constructor with parameter name: 
      VBFTagger(const std::string& name = "VBFTagger");

      /// Destructor: 
      virtual ~VBFTagger(); 

      // init related methods
      virtual StatusCode initialize();
      StatusCode ReadRNNModel();
      StatusCode ReadScalingValues();

      // execute related methods
      virtual StatusCode execute();
      StatusCode LoadRNN();      
      lwt::LightweightGraph::SeqNodeMap get_sequences(const std::vector<lwt::InputNodeConfig>& config);
      lwt::VectorMap get_JetVars(const std::vector<lwt::Input>& inputs);
      lwt::LightweightGraph::NodeMap get_flat_inputs(const std::vector<lwt::InputNodeConfig>& config);
      void SetFlatInputs(std::map<std::string, double> &in_vals);

      virtual StatusCode SelectInputJets(const xAOD::JetContainer& jets);
      virtual StatusCode SelectInputJets(std::vector<std::unique_ptr<const xAOD::Jet*>>& jets);
      virtual StatusCode SelectInputJets(std::vector<double> jets_pT, std::vector<double> jets_eta, std::vector<double> jets_phi, std::vector<double> jets_m);
      virtual StatusCode SetInputSequences(std::vector<double> pT, std::vector<double> eta, std::vector<double> phi, std::vector<double> E);

      // finalise
      virtual StatusCode finalize();
    
      // utils methods
      virtual int GetNRNNJets();
      virtual float GetRNNScore();
      // ToDo: return RNN jets
      //virtual std::vector<std::unique_ptr<const xAOD::Jet*>> GetRNNJets();

      StatusCode CloseBosonMassJets(std::vector<std::unique_ptr<const xAOD::Jet*>>& jets, int mBoson);

    private:

      // configuration  
      Gaudi::Property<bool> m_IsDebug{this, "IsDebug", false};
      Gaudi::Property<bool> m_IsPureRNN{this, "IsPureRNN", true};
      bool m_UseNTracks;
      Gaudi::Property<std::string> m_PreProcJetsMethod{this, "PreProcJetsMethod", ""};
      Gaudi::Property<double> m_pTCut{this, "pTCut", 30.e3};

      // model related
      Gaudi::Property<std::string> m_modelTag{this, "modelTag", "", "VBF RNN model version"};
      lwt::GraphConfig m_config;
      std::map<std::string, float> m_fscaler_mean, m_fscaler_std;

      // physics vars
      Gaudi::Property<int> m_nMaxJets{this, "nMaxJets", 2};
      int m_nRNNJets = 0;
      std::vector<double> m_jets_pT;
      std::vector<double> m_jets_eta;
      std::vector<double> m_jets_phi;
      std::vector<double> m_jets_E;
      std::vector<double> m_jets_nTracks;
      std::vector<std::unique_ptr<const xAOD::Jet*>> m_RNNJets;

      double m_RNNScore;

      const double m_mW = 80.38;
      const double m_mZ = 91.19;
      const double m_mh = 125.25;

      // collection of pointers to jets to remove from inputs
      std::vector<std::unique_ptr<const xAOD::Jet*>> m_JetsToDiscard;

}; 

#endif //> !VBFTagger_H
