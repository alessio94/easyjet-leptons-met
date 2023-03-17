from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from HH4bAnalysis.config.container_names import get_container_names


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
                "eventNumber": "ULL",
                "mcEventNumber": "ULL",
                "lumiBlock": "UINT",
                "averageInteractionsPerCrossing": "HALF",
                "actualInteractionsPerCrossing": "HALF",
            },
            datasetName="event",
            output=output,
        )
    )
    jetcol = get_container_names(flags)["outputs"]["reco4Jet"]
    types = {}
    primitives = []
    if flags.Analysis.n_h5_jets > 0:
        types |= {"valid": "CUSTOM"}
        primitives.append("valid")
    associations = {}
    kinematics = ["ptGeV", "eta", "phi", "massGeV"]
    types |= {x: "CUSTOM" for x in kinematics}
    btagging = [f"DL1dv00_p{x}" for x in "cub"]
    types |= {x: "HALF" for x in btagging}
    associations = {x: "btaggingLink" for x in btagging}
    primitives += kinematics + btagging
    if flags.Input.isMC:
        mc_primatives, mc_types = _get_truth_types()
        primitives += mc_primatives
        types |= mc_types
    ca.addEventAlgo(
        CompFactory.IParticleWriterAlg(
            "jetwriter",
            primitives=primitives,
            primitiveToType=types,
            primitiveToAssociation=associations,
            datasetName="jets",
            maximumSize=flags.Analysis.n_h5_jets,
            container=jetcol.replace("_%SYS%", "_NOSYS"),
            output=output,
        )
    )
    return ca


def _get_truth_types():
    ffloats = []
    fints = ["ID"]
    ftag_label = [f"HadronConeExclTruthLabel{x}" for x in ffloats + fints]
    bhalves = ["DRTruthParticle"]
    bints = [
        "PdgId",
        "Barcode",
        "MatchingParticlePdgId",
        "MatchingParticleNChildren",
        "MatchingParticleBarcode",
    ]

    types = {}
    types |= {f"HadronConeExclTruthLabel{x}": "FLOAT" for x in ffloats}
    types |= {f"HadronConeExclTruthLabel{x}": "INT" for x in fints}
    boson_label = []
    for boson in "Higgs", "Scalar", "Top":
        boson_label += [f"parent{boson}{x}" for x in bhalves + bints]
        types |= {f"parent{boson}{x}": "HALF" for x in bhalves}
        types |= {f"parent{boson}{x}": "INT" for x in bints}

    return ftag_label + boson_label, types
