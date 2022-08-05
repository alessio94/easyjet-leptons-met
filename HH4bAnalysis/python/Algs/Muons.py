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
        ptSelectionOutput=False,
        qualitySelectionOutput=True,
        enableCutflow=False,
        enableKinematicHistograms=False,
    )
    muonSequence.configure(inputName=inputContainerName, outputName=outputContainerName)
    # print(muonSequence)  # For debugging

    cfg.addSequence(CompFactory.AthSequencer(muonSequence.getName()))
    for alg in muonSequence.getGaudiConfig2Components():
        if "MuonSelectionAlg" in alg.getName():
            alg.selectionTool.IsRun3Geo = min(flags.Input.RunNumber) > 400000
        cfg.addEventAlgo(alg, muonSequence.getName())

    return cfg
