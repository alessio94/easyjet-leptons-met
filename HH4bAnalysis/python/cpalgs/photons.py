from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def photon_sequence_cfg(flags, datatype, incontainername, outcontainername):
    cfg = ComponentAccumulator()
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import (
        makePhotonAnalysisSequence,
    )

    photon_sequence = makePhotonAnalysisSequence(
        datatype,
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
    photon_sequence.configure(inputName=incontainername, outputName=outcontainername)

    cfg.addSequence(CompFactory.AthSequencer(photon_sequence.getName()))
    for alg in photon_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, photon_sequence.getName())

    return cfg
