from AthenaConfiguration.ComponentFactory import CompFactory
from utils.convertOldConfigHelper import convertSequenceAndGetAlgs

# Include, and then set up the photon analysis sequence:
from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import (
    makePhotonAnalysisSequence,
)


def makeAndAddPhotonAnalysisSequence(
    compAcc, dataType, inputContainerName, outputContainerName
):
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

    # Convert to new configurables
    photonSequenceCnv, photonAlgsCnv = convertSequenceAndGetAlgs(
        CompFactory, photonSequence
    )
    compAcc.addSequence(photonSequenceCnv)
    for photonAlg in photonAlgsCnv:
        compAcc.addEventAlgo(photonAlg, photonSequenceCnv.getName())

    return photonSequenceCnv, photonAlgsCnv
