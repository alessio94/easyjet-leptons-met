from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def muon_sequence_cfg(flags, datatype, inname, outname):
    cfg = ComponentAccumulator()
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence

    muon_sequence = makeMuonAnalysisSequence(
        datatype,
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
    muon_sequence.configure(inputName=inname, outputName=outname)
    # print(muon_sequence)  # For debugging

    cfg.addSequence(CompFactory.AthSequencer(muon_sequence.getName()))
    for alg in muon_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, muon_sequence.getName())

    return cfg
