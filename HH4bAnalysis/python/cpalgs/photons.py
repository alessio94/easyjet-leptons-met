from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def photon_sequence_cfg(flags, containers):
    cfg = ComponentAccumulator()
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import (
        makePhotonAnalysisSequence,
    )

    photon_sequence = makePhotonAnalysisSequence(
        flags.Analysis.DataType,
        workingPoint="Loose.NonIso",
        postfix="loose",
        deepCopyOutput=False,
        shallowViewOutput=True,
        crackVeto=False,
        enableCleaning=True,
        cleaningAllowLate=False,
        recomputeIsEM=False,
        ptSelectionOutput=True,
        enableCutflow=False,
        enableKinematicHistograms=False,
    )
    photon_sequence.configure(
        inputName=containers["inputs"]["photons"],
        outputName=containers["outputs"]["photons"],
    )

    cfg.addSequence(CompFactory.AthSequencer(photon_sequence.getName()))
    for alg in photon_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, photon_sequence.getName())

    return cfg
