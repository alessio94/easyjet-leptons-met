from AthenaConfiguration.ComponentFactory import CompFactory
from utils.convertOldConfigHelper import convertSequenceAndGetAlgs

# Include, and then set up the electron analysis sequence:
from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import (
    makeElectronAnalysisSequence,
)


def makeAndAddElectronAnalysisSequence(
    compAcc, dataType, inputContainerName, outputContainerName
):
    electronSequence = makeElectronAnalysisSequence(
        dataType,
        workingPoint="LooseLHElectron.NonIso",
        postfix="loose",
        deepCopyOutput=False,
        shallowViewOutput=True,
        recomputeLikelihood=False,
        chargeIDSelection=False,
        isolationCorrection=False,
        crackVeto=False,
        ptSelectionOutput=False,
        enableCutflow=False,
        enableKinematicHistograms=False,
    )
    electronSequence.configure(
        inputName=inputContainerName, outputName=outputContainerName
    )

    # Convert to new configurables
    electronSequenceCnv, electronAlgsCnv = convertSequenceAndGetAlgs(
        CompFactory, electronSequence
    )
    compAcc.addSequence(electronSequenceCnv)
    for electronAlg in electronAlgsCnv:
        compAcc.addEventAlgo(electronAlg, electronSequenceCnv.getName())

    return electronSequenceCnv, electronAlgsCnv
