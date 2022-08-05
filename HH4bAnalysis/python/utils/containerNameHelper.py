# custom container names used in this framework
RECO_4_PFLOW_JETS_KEY = "Reco4PFlowJets"
RECO_10_PFLOW_JETS_KEY = "Reco10PFlowJets"
VR_JETS_KEY = "VRJets"
TRUTH_4_JETS_KEY = "Truth4Jets"
TRUTH_10_JETS_KEY = "Truth10Jets"
MUONS_KEY = "Muons"
ELECTRONS_KEY = "Electrons"
PHOTONS_KEY = "Photons"

container_map = {
    "DAOD_PHYS": {
        RECO_4_PFLOW_JETS_KEY: "AntiKt4EMPFlowJets",
        RECO_10_PFLOW_JETS_KEY: "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
        VR_JETS_KEY: "AntiKtVR30Rmax4Rmin02PV0TrackJets",
        TRUTH_4_JETS_KEY: "AntiKt4TruthDressedWZJets",
        TRUTH_10_JETS_KEY: "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
        MUONS_KEY: "Muons",
        ELECTRONS_KEY: "Electrons",
        PHOTONS_KEY: "Photons",
    },
    "DAOD_PHYSLITE": {
        RECO_4_PFLOW_JETS_KEY: "AnalysisJets",
        RECO_10_PFLOW_JETS_KEY: "",
        VR_JETS_KEY: "",
        TRUTH_4_JETS_KEY: "AntiKt4TruthDressedWZJets",
        TRUTH_10_JETS_KEY: "",
        MUONS_KEY: "AnalysisMuons",
        ELECTRONS_KEY: "AnalysisElectrons",
        PHOTONS_KEY: "AnalysisPhotons",
    },
}


def getContainerName(qualitycontainerdesc, daodphyslite=False):
    format_key = "DAOD_PHYSLITE" if daodphyslite else "DAOD_PHYS"
    return container_map[format_key][qualitycontainerdesc]
