from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.output.ttree.tau_decor_config import tau_decor_cfg


def tau_sequence_cfg(flags):
    cfg = ComponentAccumulator()
    from TauAnalysisAlgorithms.TauAnalysisSequence import makeTauAnalysisSequence

    cfg.merge(tau_decor_cfg(flags))

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
        inputName=flags.Analysis.container_names.input.taus,
        outputName=flags.Analysis.container_names.output.taus,
    )

    cfg.addSequence(CompFactory.AthSequencer(tau_sequence.getName()))
    for alg in tau_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, tau_sequence.getName())

    return cfg
