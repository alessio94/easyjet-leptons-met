///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2019-2020 CERN for the benefit of the ATLAS collaboration
*/

// VBFTagger.h
// header file for class VBFTagger
// Author: Antonio Giannini
// Email : antonio.giannini@cern.ch
///////////////////////////////////////////////////////////////////

#ifndef IVBFTagger_H
#define IVBFTagger_H 1

// VBFTagger includes
#include <string.h>

#include "AsgTools/AsgTool.h"
#include "AsgMessaging/AsgMessaging.h"
#include "xAODJet/JetContainer.h"

class IVBFTagger : public virtual asg::IAsgTool {

	ASG_TOOL_INTERFACE(IVBFTagger)

    public:

      virtual StatusCode initialize() = 0;
      virtual StatusCode execute() = 0;
      virtual StatusCode finalize() = 0;
    
      virtual StatusCode SelectInputJets(const xAOD::JetContainer& jets) = 0;
      virtual StatusCode SelectInputJets(std::vector<std::unique_ptr<const xAOD::Jet*>>& jets) = 0;
      virtual StatusCode SelectInputJets(std::vector<double> jets_pT, std::vector<double> jets_eta, std::vector<double> jets_phi, std::vector<double> jets_m) = 0;
      virtual StatusCode SetInputSequences(std::vector<double> pT, std::vector<double> eta, std::vector<double> phi, std::vector<double> E) = 0;

      virtual int GetNRNNJets() = 0;
      virtual float GetRNNScore() = 0;
      // ToDo: return RNN jets
      //virtual std::vector<std::unique_ptr<const xAOD::Jet*>> GetRNNJets() = 0;

}; 

#endif //> !IVBFTagger_H
