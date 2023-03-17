from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Include, and then set up the electron analysis sequence:
from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import (
    makeElectronAnalysisSequence,
)


def electron_sequence_cfg(flags, datatype, incontainername, outcontainername):
    cfg = ComponentAccumulator()

    # with ConfigurableCABehavior(False):
    electronSequence = makeElectronAnalysisSequence(
        datatype,
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
    electronSequence.configure(inputName=incontainername, outputName=outcontainername)

    cfg.addSequence(CompFactory.AthSequencer(electronSequence.getName()))
    for alg in electronSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, electronSequence.getName())

    return cfg
