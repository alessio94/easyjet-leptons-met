# Baseline HH analysis in the `easyjet-ntupler`

The resolved and boosted analyses can be turned on/off from the [bbbbAnalysis/share/RunConfig.yaml](https://gitlab.cern.ch/easyjet/easyjet/-/tree/main/bbbbAnalysis/share/RunConfig.yaml). The Configuration and the stacking of the algorithms can be found in [bbbbAnalysis/python/config/dihiggs.py](https://gitlab.cern.ch/easyjet/easyjet/-/tree/main/bbbbAnalysis/python/config/dihiggs.py). The flow diagrams depict what is happening:

### resolved

```mermaid

flowchart LR
A[xAOD::JetContainer\n smallR jets] --> JetSelectorAlg
subgraph JetSelectorAlg
   bTagWP\nminPt:20_000\nmaxEta:2.5\ntruncateAtAmount:4\nminimumAmount:4\npTsort:True
end
JetSelectorAlg --> B[xAOD::JetContainer]
B[xAOD::JetContainer]-->JetPairingAlg
subgraph JetPairingAlg
   pairingStrategy:minDeltaR
end
JetPairingAlg --> C[xAOD::JetContainer]
C[xAOD::JetContainer] --> BaselineVarsResolvedAlg
subgraph BaselineVarsResolvedAlg
   smallRContainerInKey\nbTagWP
end
 
```

### boosted

```mermaid

flowchart TB
A[xAOD::JetContainer\n largeR jets] --> JetSelectorAlg
subgraph JetSelectorAlg
   bTagWP:empty_string\nminPt:250_000\nmaxEta:2.0\ntruncateAtAmount:2\nminimumAmount:2\npTsort:True
end

JetSelectorAlg --> B[xAOD::JetContainer]
B[xAOD::JetContainer]-->GhostAssocVRJetGetterAlg1
subgraph GhostAssocVRJetGetterAlg1
   whichJet:0
end

GhostAssocVRJetGetterAlg1 --> C1[xAOD::JetContainer]
C1 --> JetSelectorAlg1
subgraph JetSelectorAlg1
   bTagWP:btag_wp\nminPt:10000\nmaxEta:2.5\ntruncateAtAmount:3\nminimumAmount:2\npTsort:True\nremoveRelativeDeltaRToVRJet:True 
end
JetSelectorAlg1 --> D1[xAOD::JetContainer]
D1-->BaselineVarsBoostedAlg

B[xAOD::JetContainer]-->GhostAssocVRJetGetterAlg2
subgraph GhostAssocVRJetGetterAlg2
   whichJet:1
end
GhostAssocVRJetGetterAlg2 --> C2[xAOD::JetContainer]
C2 --> JetSelectorAlg2
subgraph JetSelectorAlg2
   BTagWP:btag_wp\nminPt:10000\nmaxEta:2.5\ntruncateAtAmount:3\nminimumAmount:2\npTsort:True\nremoveRelativeDeltaRToVRJet:True 
end
JetSelectorAlg2 --> D2[xAOD::JetContainer]
D2-->BaselineVarsBoostedAlg
B-->BaselineVarsBoostedAlg
subgraph BaselineVarsBoostedAlg
   largeRContainerInKey\nleadingLargeR_GA_VRJets\nsubLeadingLargeR_GA_VRJets\nbTagWP
end
```
