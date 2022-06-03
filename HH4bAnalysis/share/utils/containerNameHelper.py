#!/bin/env python

# general container names used in this analysis
RECO_4_PFLOW_JETS = "Reco4PFlowJets"
RECO_10_PFLOW_JETS = "Reco10PFlowJets"
TRUTH_4_JETS = "Truth4Jets"
TRUTH_10_JETS = "Truth10Jets"
MUONS = "Muons"
ELECTRONS = "Electrons"

container_map = {
    "DAOD_PHYS": {
        RECO_4_PFLOW_JETS: "AntiKt4EMPFlowJets",
        RECO_10_PFLOW_JETS: "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
        TRUTH_4_JETS: "AntiKt4TruthDressedWZJets",
        TRUTH_10_JETS: "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
        MUONS: "Muons",
        ELECTRONS: "Electrons",
    },
    "DAOD_PHYSLITE": {
        RECO_4_PFLOW_JETS: "AnalysisJetsBTAG",
        RECO_10_PFLOW_JETS: "AnalysisLargeRRecoJets",
        TRUTH_4_JETS: "?",
        TRUTH_10_JETS: "?",
        MUONS: "AnalysisMuons",
        ELECTRONS: "AnalysisElectrons",
    },
}


def getContainerName(qualityontainerdesc, daodphyslite=False):
    format_key = "DAOD_PHYSLITE" if daodphyslite else "DAOD_PHYS"
    return container_map[format_key][qualityontainerdesc]
