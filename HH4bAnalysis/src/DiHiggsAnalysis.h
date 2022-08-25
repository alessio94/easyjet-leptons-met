#ifndef HH4B_DIHIGGSANALYSIS
#define HH4B_DIHIGGSANALYSIS

#include "xAODJet/JetContainer.h"
#include <Math/Vector4D.h>
#include <xAODEventInfo/EventInfo.h>

namespace HH4B
{

  // make Higgs vars names list with attached btagging working points
  std::vector<std::string>
  getHiggsVarsNames(std::vector<std::string> &btag_wps,
                    std::vector<std::string> &vr_btag_wps);

  struct HiggsCandidate
  {
    // b-jets for Higgs candidate
    const xAOD::Jet *m_leadingJet = nullptr;
    const xAOD::Jet *m_subleadingJet = nullptr;
    // four vector of Higgs candidate, (this is faster than TLorentzVector)
    ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<float>> m_fourVector;
    // dR between leading and subleading jet
    float m_dRjets = -1;

    // for boosted analysis
    const xAOD::Jet *m_largeRJet = nullptr;
    // Nr of ghost associated Vr Jets
    int m_nGhostAssocVrJets = -1;
    // Nr of btagged ghost associated Vr Jets
    int m_nBtaggedGhostAssocVrJets = -1;
    // don't decorate if requirements are not met
    bool m_doDecorate = true;
  };

  class DiHiggsAnalysis
  {
public:
    // initialize vars in m_higgsVarsMap
    void initHiggsVarsMap(std::vector<std::string> &higgsVars);
    // do resolved Analysis
    void makeResolvedAnalysis(const xAOD::JetContainer &smallRjets,
                              std::string wp);
    // do boosted Analysis
    void makeBoostedAnalysis(const xAOD::JetContainer &largeRjets,
                             std::string wp);
    // returns m_higgsVarsMap for other purposes
    std::unordered_map<std::string, float> getHiggsVarsMap();
    // map holding the final vars (like dict in python)
    // access/write like : m_diHiggs_vars[var];
    // unordered_map is faster than map as it does the lookup with a hash and
    // not by string comparison
    std::unordered_map<std::string, float> m_higgsVarsMap;
  };
}
#endif
