from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def EventSelectionAnalysisSequenceCfg(flags, dataType, grlFiles=[]):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.EventSelectionAnalysisSequence import (
        makeEventSelectionAnalysisSequence,
    )

    eventSelectionSequence = makeEventSelectionAnalysisSequence(
        dataType, userGRLFiles=grlFiles, runEventCleaning=True
    )

    cfg.addSequence(CompFactory.AthSequencer(eventSelectionSequence.getName()))
    for alg in eventSelectionSequence.getGaudiConfig2Components():
        if "PrimaryVertexSelectorAlg" in alg.getName():
            alg.MinTracks = 2
        if "EventFlagSelectorAlg" in alg.getName():
            alg.selectionFlags = [
                "DFCommonJets_eventClean_LooseBad,as_char",
                "DFCommonJets_isBadBatman,as_char",
            ]
        cfg.addEventAlgo(alg, eventSelectionSequence.getName())

    return cfg


def TriggerAnalysisAlgsCfg(flags, triggerChains):
    cfg = ComponentAccumulator()
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg

    tdt = cfg.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    trigSelAlg = CompFactory.CP.TrigEventSelectionAlg(
        tool=tdt,
        triggers=triggerChains,
        selectionDecoration="trigPassed",
    )
    cfg.addEventAlgo(trigSelAlg)

    return cfg


def PileupAnalysisSequenceCfg(flags, dataType, prwFiles, lumicalcFiles):
    cfg = ComponentAccumulator()
    # Include, and then set up the pileup analysis sequence:
    from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence

    # with ConfigurableRun3Behavior(False):
    pileupSequence = makePileupAnalysisSequence(
        dataType,
        userPileupConfigs=prwFiles,
        userLumicalcFiles=lumicalcFiles,
        autoConfig=False,
    )
    pileupSequence.configure(inputName={}, outputName={})

    cfg.addSequence(CompFactory.AthSequencer(pileupSequence.getName()))
    for alg in pileupSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, pileupSequence.getName())

    return cfg
