from AthenaConfiguration.ComponentFactory import CompFactory
from utils.convertOldConfigHelper import convertSequenceAndGetAlgs


def makeAndAddTriggerAnalysisAlgs(compAcc, flags, triggerChains):
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg

    tdt = compAcc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    trigSelAlg = CompFactory.CP.TrigEventSelectionAlg(
        tool=tdt, triggers=triggerChains, selectionDecoration="trigPassed"
    )
    compAcc.addEventAlgo(trigSelAlg)

    return trigSelAlg


def makeAndAddPileupAnalysisSequence(compAcc, dataType, prwFiles, lumicalcFiles):
    # Include, and then set up the pileup analysis sequence:
    from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence

    pileupSequence = makePileupAnalysisSequence(
        dataType,
        userPileupConfigs=prwFiles,
        userLumicalcFiles=lumicalcFiles,
        autoConfig=False,
    )
    pileupSequence.configure(inputName={}, outputName={})
    # print(pileupSequence)  # For debugging

    # Convert to new configurables
    pileupSequenceCnv, algsCnv = convertSequenceAndGetAlgs(CompFactory, pileupSequence)
    compAcc.addSequence(pileupSequenceCnv)
    for alg in algsCnv:
        compAcc.addEventAlgo(alg, pileupSequenceCnv.getName())

    return pileupSequenceCnv, algsCnv
