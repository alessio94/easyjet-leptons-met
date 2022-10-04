
## Meaning of the variables in the DiHiggsAnalysisAlgorithm

See issue#21 for an algorithm description, it runs per btagging working point, meaning that each variable at the end has the working point attached in the ntuples. Each variable is defaulted to -1. So if you find -1 in the ntuples it means that the algorithm did not execute to the end/decoration. This can happen for example if there are no 4 btagged jets available for the pairing in the event.

# resolved: "analysis with small R jets", requires 4 btagged small R jets

| Variable                              | Description                                                                                                                                                                         |
| ------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| resolved_nCentralJets                 | Number of central jets, Central jets: ( if (jet->pt() > 25000. && std::abs(jet->eta()) < 2.5))                                                                                      |
| resolved_nBtaggedCentralJets          | Number of btagged central jets                                                                                                                                                      |
| resolved_jet1_pt                      | leading jet pt                                                                                                                                                                      |
| resolved_jet2_pt                      | subleading jet pt                                                                                                                                                                   |
| resolved_jet3_pt                      | subsubleading jet pt                                                                                                                                                                |
| resolved_jet4_pt                      | sub^3leading jet pt                                                                                                                                                                 |
| resolved_DeltaR12                     | DeltaR between upper jets                                                                                                                                                           |
| resolved_DeltaR13                     | DeltaR between upper jets                                                                                                                                                           |
| resolved_DeltaR14                     | DeltaR between upper jets                                                                                                                                                           |
| resolved_DeltaR23                     | DeltaR between upper jets                                                                                                                                                           |
| resolved_DeltaR24                     | DeltaR between upper jets                                                                                                                                                           |
| resolved_DeltaR34                     | DeltaR between upper jets                                                                                                                                                           |
| resolved_h1_m                         | Leading Higgs Candidate mass                                                                                                                                                        |
| resolved_h2_m                         | subLeading Higgs Candidate mass                                                                                                                                                     |
| resolved_h1_dR_jets                   | dR between jets used to reconstruct Leading Higgs Candidate                                                                                                                         |
| resolved_h2_dR_jets                   | dR between jets used to reconstruct subLeading Higgs Candidate                                                                                                                      |
| resolved_hh_m                         | reconstructed Dihiggs mass                                                                                                                                                          |
| resolved_h1_fromSameInitialParticle   | bool: if closest truth b of leading jet for leading Higgs candidate has the same parent (H or S) as the subleading jet (only becomes meaningful with a dR criterion futher down)    |
| resolved_h2_fromSameInitialParticle   | bool: if closest truth b of leading jet for subleading Higgs candidate has the same parent (H or S) as the subleading jet (only becomes meaningful with a dR criterion futher down) |
| resolved_h1_dR_leadingJet_closestB    | dR between the leading jet used for the leading Higgs candidate and the closest truth b                                                                                             |
| resolved_h2_dR_leadingJet_closestB    | dR between the leading jet used for the subleading Higgs candidate and the closest truth b                                                                                          |
| resolved_h1_dR_subleadingJet_closestB | dR between the subleading jet used for the leading Higgs candidate and the closest truth b                                                                                          |
| resolved_h2_dR_subleadingJet_closestB | dR between the subleading jet used for the subleading Higgs candidate and the closest truth b                                                                                       |

# boosted: "analysis with large R jets", requires two large R jets


| Variable                                 | Description                                                          |
| ---------------------------------------- | -------------------------------------------------------------------- |
| boosted_nLargeJets                       | Number of large R jets                                               |
| boosted_h1_m                             | leading Higgs Candidate mass                                         |
| boosted_h1_jet1_pt                       | leading jet pt of leading Higgs Candidate                            |
| boosted_h1_jet2_pt                       | subleading jet pt of leading Higgs Candidate                         |
| boosted_h1_dR_jets                       | dR between jets used to reconstruct leading Higgs Candidate          |
| boosted_h2_m                             | subleading Higgs Candidate mass                                      |
| boosted_h2_jet1_pt                       | leading jet pt of leading Higgs Candidate                            |
| boosted_h2_jet2_pt                       | subleading jet pt of leading Higgs Candidate                         |
| boosted_h2_dR_jets                       | dR between jets used to reconstruct leading Higgs Candidate          |
| boosted_hh_m                             | reconstructed Dihiggs mass                                           |
| boosted_h1_nGhostAssocVrJets             | nr of ghost associated vr jets of leading Higgs Candidate            |
| boosted_h1_nBtaggedGhostAssocVrTrackJets | nr of btagged ghost associated vr jets of leading Higgs Candidate    |
| boosted_h2_nGhostAssocVrJets             | nr of ghost associated vr jets of subleading Higgs Candidate         |
| boosted_h2_nBtaggedGhostAssocVrTrackJets | nr of btagged ghost associated vr jets of subleading Higgs Candidate |
