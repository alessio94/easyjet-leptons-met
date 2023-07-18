from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def tau_sequence_cfg(flags, containers):
    cfg = ComponentAccumulator()
    from TauAnalysisAlgorithms.TauAnalysisSequence import makeTauAnalysisSequence

    tau_sequence = makeTauAnalysisSequence(
        flags.Analysis.DataType,
        workingPoint="Loose",
        postfix="loose",
        deepCopyOutput=False,
        shallowViewOutput=True,
        enableCutflow=False,
        enableKinematicHistograms=False,
        # isRun3Geo=(flags.Analysis.Run == 3),
    )
    tau_sequence.configure(
        inputName=containers["inputs"]["taus"],
        outputName=containers["outputs"]["taus"],
    )

    cfg.addSequence(CompFactory.AthSequencer(tau_sequence.getName()))
    for alg in tau_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, tau_sequence.getName())

    return cfg
