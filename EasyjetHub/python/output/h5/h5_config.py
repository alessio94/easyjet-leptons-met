from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def get_h5_cfg(flags):
    ca = ComponentAccumulator()
    output = CompFactory.H5FileSvc(path=flags.Analysis.h5_output.as_posix())
    ca.addService(output)
    ca.addEventAlgo(
        CompFactory.EventInfoWriterAlg(
            "infowriter",
            primitives=[
                "eventNumber",
                "lumiBlock",
                "averageInteractionsPerCrossing",
                "actualInteractionsPerCrossing",
            ],
            primitiveToType={
                "eventNumber": "UL2ULL",
                "mcEventNumber": "ULL",
                "lumiBlock": "UINT",
                "averageInteractionsPerCrossing": "HALF",
                "actualInteractionsPerCrossing": "HALF",
            },
            datasetName="event",
            output=output,
        )
    )
    jetcol = flags.Analysis.container_names.input[flags.Analysis.Small_R_jet.jet_type]
    types = {}
    primitives = []
    if flags.Analysis.h5.n_jets > 0:
        types |= {"valid": "CUSTOM"}
        primitives.append("valid")
    associations = {}
    kinematics = ["ptGeV", "eta", "phi", "massGeV"]
    types |= {x: "CUSTOM" for x in kinematics}
    btagging = [f"DL1dv01_p{x}" for x in "cub"]
    types |= {x: "HALF" for x in btagging}
    associations = {x: f"btaggingLink/{x}" for x in btagging}
    primitives += kinematics + btagging
    if flags.Input.isMC:
        mc_primatives, mc_types, mc_assoc = _get_truth_types(flags)
        primitives += mc_primatives
        types |= mc_types
        associations |= mc_assoc
    ca.addEventAlgo(
        CompFactory.IParticleWriterAlg(
            "jetwriter",
            primitives=primitives,
            primitiveToType=types,
            primitiveToAssociation=associations,
            datasetName="jets",
            maximumSize=flags.Analysis.h5.n_jets,
            container=jetcol.replace("_%SYS%", "_NOSYS"),
            output=output,
        )
    )
    return ca


def _get_truth_types(flags):
    ffloats = []
    fints = ["ID"]
    ftag_label = [f"HadronConeExclTruthLabel{x}" for x in ffloats + fints]
    bhalves = [
        "DRTruthParticle",
    ]
    bints = [
        "PdgId",
        "MatchingParticlePdgId",
        "MatchingParticleNChildren",
    ]
    bchar = [
        "Index",
    ]
    buchar = [
        "NMatchedChildren",
    ]
    bull = [
        "ParentsMask",
    ]

    types = {}
    types |= {f"HadronConeExclTruthLabel{x}": "FLOAT" for x in ffloats}
    types |= {f"HadronConeExclTruthLabel{x}": "INT2CHAR" for x in fints}
    boson_label = []
    associations = {}
    all_boson_suffix = bhalves + bints + bchar + bull

    physlite = flags.Input.isPHYSLITE
    all_parents = [] if physlite else ["Higgs", "Z", "Scalar", "Top"]
    for parent in all_parents:
        boson_label += [f"parent{parent}{x}" for x in all_boson_suffix]
        types |= {f"parent{parent}{x}": "HALF" for x in bhalves}
        types |= {f"parent{parent}{x}": "INT2SHORT" for x in bints}
        types |= {f"parent{parent}{x}": "CHAR" for x in bchar}
        types |= {f"parent{parent}{x}": "UCHAR" for x in buchar}
        types |= {f"parent{parent}{x}": "ULL" for x in bull}

        matchpt = f"parent{parent}MatchingParticlePtGeV"
        boson_label += [matchpt]
        associations[matchpt] = f"parent{parent}MatchingParticleLink/ptGeV"
        types[matchpt] = "CUSTOM"

        matchbar = f"parent{parent}Barcode"
        boson_label += [matchbar]
        associations[matchbar] = f"parent{parent}Link/barcode"
        types[matchbar] = "INT"

    cascade_types = [] if physlite else ["B", "W"]
    for cascade_type in cascade_types:
        key = f"nTopTo{cascade_type}Children"
        types[key] = "UCHAR"
        boson_label.append(key)

    return ftag_label + boson_label, types, associations
