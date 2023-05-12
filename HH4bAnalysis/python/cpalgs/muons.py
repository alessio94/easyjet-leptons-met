from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def muon_sequence_cfg(flags, containers):
    cfg = ComponentAccumulator()
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence

    muon_sequence = makeMuonAnalysisSequence(
        flags.Analysis.DataType,
        workingPoint="Loose.NonIso",
        postfix="loose",
        deepCopyOutput=False,
        shallowViewOutput=True,
        ptSelectionOutput=True,
        qualitySelectionOutput=True,
        enableCutflow=False,
        enableKinematicHistograms=False,
        isRun3Geo=(flags.Analysis.Run == 3),
    )
    muon_sequence.configure(
        inputName=containers["inputs"]["muons"],
        outputName=containers["outputs"]["muons"],
    )

    cfg.addSequence(CompFactory.AthSequencer(muon_sequence.getName()))
    for alg in muon_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, muon_sequence.getName())

    return cfg
