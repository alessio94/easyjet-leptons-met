from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Include, and then set up the electron analysis sequence:
from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import (
    makeElectronAnalysisSequence,
)


def electron_sequence_cfg(flags, containers):
    cfg = ComponentAccumulator()

    # with ConfigurableCABehavior(False):
    electronSequence = makeElectronAnalysisSequence(
        flags.Analysis.DataType,
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
        inputName=containers["inputs"]["electrons"],
        outputName=containers["outputs"]["electrons"],
    )

    cfg.addSequence(CompFactory.AthSequencer(electronSequence.getName()))
    for alg in electronSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, electronSequence.getName())

    return cfg
