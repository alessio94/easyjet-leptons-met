from HH4bAnalysis.utils.inputsHelper import is_physlite

# custom container names used in this framework
RECO_4_PFLOW_JETS_KEY = "Reco4PFlowJets"
RECO_10_PFLOW_JETS_KEY = "Reco10PFlowJets"
RECO_10_UFO_JETS_KEY = "Reco10UFOJets"
VR_JETS_KEY = "VRJets"
TRUTH_4_JETS_KEY = "Truth4Jets"
TRUTH_10_JETS_KEY = "Truth10Jets"
TRUTH_10_UFO_JETS_KEY = "Truth10UFOJets"
MUONS_KEY = "Muons"
ELECTRONS_KEY = "Electrons"
PHOTONS_KEY = "Photons"
TRUTH_PARTICLE_INFO_KEY = "TruthParticles"


container_map = {
    "DAOD_PHYS": {
        RECO_4_PFLOW_JETS_KEY: "AntiKt4EMPFlowJets",
        RECO_10_PFLOW_JETS_KEY: "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
        RECO_10_UFO_JETS_KEY: "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
        VR_JETS_KEY: "AntiKtVR30Rmax4Rmin02PV0TrackJets",
        TRUTH_4_JETS_KEY: "AntiKt4TruthDressedWZJets",
        TRUTH_10_JETS_KEY: "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
        TRUTH_10_UFO_JETS_KEY: "AntiKt10TruthSoftDropBeta100Zcut10Jets",
        MUONS_KEY: "Muons",
        ELECTRONS_KEY: "Electrons",
        PHOTONS_KEY: "Photons",
        TRUTH_PARTICLE_INFO_KEY: "TruthBSMWithDecayParticles",
    },
    "DAOD_PHYSLITE": {
        RECO_4_PFLOW_JETS_KEY: "AnalysisJets",
        RECO_10_PFLOW_JETS_KEY: "",
        RECO_10_UFO_JETS_KEY: "",
        VR_JETS_KEY: "",
        TRUTH_4_JETS_KEY: "AntiKt4TruthDressedWZJets",
        TRUTH_10_JETS_KEY: "",
        TRUTH_10_UFO_JETS_KEY: "",
        MUONS_KEY: "AnalysisMuons",
        ELECTRONS_KEY: "AnalysisElectrons",
        PHOTONS_KEY: "AnalysisPhotons",
        TRUTH_PARTICLE_INFO_KEY: "TruthBSMWithDecayParticles",
    },
}


def _get_container_name(qualitycontainerdesc, daodphyslite=False):
    format_key = "DAOD_PHYSLITE" if daodphyslite else "DAOD_PHYS"
    return container_map[format_key][qualitycontainerdesc]


def get_container_names(flags):
    is_daod_physlite = is_physlite(flags)
    inputs = dict(
        reco4Jet=_get_container_name("Reco4PFlowJets", is_daod_physlite),
        truth4Jet=_get_container_name("Truth4Jets", is_daod_physlite),
        reco10Jet=_get_container_name("Reco10PFlowJets", is_daod_physlite),
        reco10UFOJet=_get_container_name("Reco10UFOJets", is_daod_physlite),
        truth10Jet=_get_container_name("Truth10Jets", is_daod_physlite),
        truth10UFOJet=_get_container_name("Truth10UFOJets", is_daod_physlite),
        vrJet=_get_container_name("VRJets", is_daod_physlite),
        muons=_get_container_name("Muons", is_daod_physlite),
        electrons=_get_container_name("Electrons", is_daod_physlite),
        photons=_get_container_name("Photons", is_daod_physlite),
    )
    # If not running calibration algs in PHYSLITE, we can just skip the outputs
    if flags.Analysis.disable_calib:
        return {"inputs": inputs, "outputs": inputs}

    outputs = dict(
        reco4Jet=f"Analysis{inputs['reco4Jet']}_%SYS%",
        truth4Jet=inputs["truth4Jet"],
        truth10Jet=inputs["truth10Jet"],
        truth10UFOJet=inputs["truth10UFOJet"],
        muons=f"Analysis{inputs['muons']}_%SYS%",
        electrons=f"Analysis{inputs['electrons']}_%SYS%",
        photons=f"Analysis{inputs['photons']}_%SYS%",
    )

    if not is_daod_physlite:
        outputs["reco10Jet"] = f"Analysis{inputs['reco10Jet']}_%SYS%"
        # outputs["reco10UFOJet"] = f"{inputs['reco10UFOJet']}_%SYS%"
        outputs["reco10UFOJet"] = f"Analysis{inputs['reco10UFOJet']}_%SYS%"
        outputs["vrJet"] = f"Analysis{inputs['vrJet']}_%SYS%"
    else:
        outputs["reco10Jet"] = ""
        outputs["reco10UFOJet"] = ""
        outputs["vrJet"] = ""

    if flags.Input.isMC:
        inputs["truthParticles"] = _get_container_name(
            "TruthParticles", is_daod_physlite
        )
        outputs["truthParticles"] = "TruthParticles"

    return {"inputs": inputs, "outputs": outputs}
