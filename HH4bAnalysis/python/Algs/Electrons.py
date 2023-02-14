from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Include, and then set up the electron analysis sequence:
from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import (
    makeElectronAnalysisSequence,
)


def ElectronAnalysisSequenceCfg(
    flags, dataType, inputContainerName, outputContainerName
):
    cfg = ComponentAccumulator()

    # with ConfigurableCABehavior(False):
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
        ptSelectionOutput=True,
        enableCutflow=False,
        enableKinematicHistograms=False,
    )
    electronSequence.configure(
        inputName=inputContainerName, outputName=outputContainerName
    )

    cfg.addSequence(CompFactory.AthSequencer(electronSequence.getName()))
    for alg in electronSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, electronSequence.getName())

    return cfg
