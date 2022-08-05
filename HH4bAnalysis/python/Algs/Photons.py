from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PhotonAnalysisSequenceCfg(flags, dataType, inputContainerName, outputContainerName):
    cfg = ComponentAccumulator()
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import (
        makePhotonAnalysisSequence,
    )

    photonSequence = makePhotonAnalysisSequence(
        dataType,
        workingPoint="Loose.Undefined",
        postfix="loose",
        deepCopyOutput=False,
        shallowViewOutput=True,
        crackVeto=False,
        enableCleaning=True,
        cleaningAllowLate=False,
        recomputeIsEM=False,
        ptSelectionOutput=False,
        enableCutflow=False,
        enableKinematicHistograms=False,
    )
    photonSequence.configure(
        inputName=inputContainerName, outputName=outputContainerName
    )

    cfg.addSequence(CompFactory.AthSequencer(photonSequence.getName()))
    for alg in photonSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, photonSequence.getName())

    return cfg
