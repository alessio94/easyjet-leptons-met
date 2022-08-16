from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def EventSelectionAnalysisSequenceCfg(flags, dataType, grlFiles=[], loose=False):
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
            selectionFlags = ["DFCommonJets_eventClean_LooseBad"]
            invertFlags = [False]
            if not loose:
                selectionFlags += ["DFCommonJets_isBadBatman"]
                invertFlags += [True]
            alg.FilterDescription = (
                f"selecting events passing {', '.join(selectionFlags)}"
            )
            alg.selectionFlags = [f"{flag},as_char" for flag in selectionFlags]
            alg.invertFlags = invertFlags

        cfg.addEventAlgo(alg, eventSelectionSequence.getName())

    return cfg


def TriggerAnalysisSequenceCfg(flags, dataType, triggerChains):
    cfg = ComponentAccumulator()
    from TriggerAnalysisAlgorithms.TriggerAnalysisSequence import (
        makeTriggerAnalysisSequence,
    )

    triggerSequence = makeTriggerAnalysisSequence(dataType, triggerChains=triggerChains)

    cfg.addSequence(CompFactory.AthSequencer(triggerSequence.getName()))
    for alg in triggerSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, triggerSequence.getName())

    return cfg


def PileupAnalysisSequenceCfg(flags, dataType, prwFiles, lumicalcFiles):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence

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


def GeneratorAnalysisSequenceCfg(flags, dataType):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.GeneratorAnalysisSequence import (
        makeGeneratorAnalysisSequence,
    )

    generatorSequence = makeGeneratorAnalysisSequence(
        dataType,
        runNumber=flags.Input.RunNumber[0],
    )

    cfg.addSequence(CompFactory.AthSequencer(generatorSequence.getName()))
    for alg in generatorSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, generatorSequence.getName())

    return cfg
