# A bit repetitive but avoids extracting the list
# from a specific file format
objtypes = [
    "reco4Jet",
    "reco10TopoJet",
    "reco10UFOJet",
    "vrJet",
    "truth4Jet",
    "truth10TrimmedJet",
    "truth10SoftDropJet",
    "muons",
    "electrons",
    "photons",
    "truthBSMParticles",
    "truthSMParticles",
]

container_map = {
    "DAOD_PHYS": {
        "reco4Jet":           "AntiKt4EMPFlowJets",
        "reco10TopoJet":      "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
        "reco10UFOJet":       "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
        "vrJet":              "AntiKtVR30Rmax4Rmin02PV0TrackJets",
        "truth4Jet":          "AntiKt4TruthDressedWZJets",
        "truth10TrimmedJet":  "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
        "truth10SoftDropJet": "AntiKt10TruthSoftDropBeta100Zcut10Jets",
        "muons":              "Muons",
        "electrons":          "Electrons",
        "photons":            "Photons",
        "truthBSMParticles":  "TruthBSMWithDecayParticles",
        "truthSMParticles":   "TruthBosonsWithDecayParticles",
    },
    "DAOD_PHYSLITE": {
        "reco4Jet":           "AnalysisJets",
        "reco10TopoJet":      "",
        "reco10UFOJet":       "AnalysisLargeRJets",
        "vrJet":              "",
        "truth4Jet":          "AntiKt4TruthDressedWZJets",
        "truth10TrimmedJet":  "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
        "truth10SoftDropJet": "AntiKt10TruthSoftDropBeta100Zcut10Jets",
        "muons":              "AnalysisMuons",
        "electrons":          "AnalysisElectrons",
        "photons":            "AnalysisPhotons",
        "truthBSMParticles":  "TruthBSMWithDecayParticles",
        "truthSMParticles":   "TruthBosonsWithDecayParticles",
    },
}


def _get_container_name(objtype, daodphyslite=False):
    daod_format = "DAOD_PHYSLITE" if daodphyslite else "DAOD_PHYS"
    return container_map[daod_format][objtype]


def get_container_names(flags):
    inputs = {
        objtype: _get_container_name(objtype, flags.Input.isPHYSLITE)
        for objtype in objtypes
    }

    # If not running calibration algs in PHYSLITE, we can just skip the outputs
    if flags.Analysis.disable_calib:
        outputs = inputs
    else:
        outputs = dict(
            reco4Jet=f"Analysis{inputs['reco4Jet']}_%SYS%",
            reco10UFOJet=f"Analysis{inputs['reco10UFOJet']}_%SYS%",
            muons=f"Analysis{inputs['muons']}_%SYS%",
            electrons=f"Analysis{inputs['electrons']}_%SYS%",
            photons=f"Analysis{inputs['photons']}_%SYS%",
            #
            truth4Jet=inputs["truth4Jet"],
            truth10TrimmedJet=inputs["truth10TrimmedJet"],
            truth10SoftDropJet=inputs["truth10SoftDropJet"],
        )

    outputs["truthHHParticles"] = "TruthDiHiggsParticles"

    if flags.Input.isPHYSLITE:
        outputs["reco10TopoJet"] = ""
        outputs["vrJet"] = ""
    else:
        outputs["reco10TopoJet"] = f"Analysis{inputs['reco10TopoJet']}_%SYS%"
        outputs["vrJet"] = f"Analysis{inputs['vrJet']}_%SYS%"

    return {"inputs": inputs, "outputs": outputs}
