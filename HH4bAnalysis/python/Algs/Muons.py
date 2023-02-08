from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonAnalysisSequenceCfg(flags, dataType, inputContainerName, outputContainerName):
    cfg = ComponentAccumulator()
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence

    muonSequence = makeMuonAnalysisSequence(
        dataType,
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
    muonSequence.configure(inputName=inputContainerName, outputName=outputContainerName)
    # print(muonSequence)  # For debugging

    cfg.addSequence(CompFactory.AthSequencer(muonSequence.getName()))
    for alg in muonSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, muonSequence.getName())

    return cfg
