from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from HH4bAnalysis.Config.Base import SampleTypes
from HH4bAnalysis.utils.logHelper import log


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

    triggerSequence = makeTriggerAnalysisSequence(
        dataType,
        triggerChains=triggerChains,
        noFilter=flags.Analysis.disable_trigger_filtering,
    )

    cfg.addSequence(CompFactory.AthSequencer(triggerSequence.getName()))
    for alg in triggerSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, triggerSequence.getName())

    return cfg


def PileupAnalysisSequenceCfg(flags, dataType, prwFiles, lumicalcFiles):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence

    tags = flags.Input.AMITag
    pileupSequence = makePileupAnalysisSequence(
        dataType,
        files=flags.Input.Files,
        useDefaultConfig=SampleTypes.mc21a.value in tags,
    )
    pileupSequence.configure(inputName={}, outputName={})

    cfg.addSequence(CompFactory.AthSequencer(pileupSequence.getName()))
    for alg in pileupSequence.getGaudiConfig2Components():
        # Workaround for mc21 courtesy of
        # https://its.cern.ch/jira/browse/ATLASG-1628?focusedCommentId=4297949&page=com.atlassian.jira.plugin.system.issuetabpanels%3Acomment-tabpanel#comment-4297949
        if SampleTypes.mc21a.value in tags and "PileupReweightingAlg" in alg.getName():
            alg.pileupReweightingTool.PeriodAssignments = []
            alg.pileupReweightingTool.DataScaleFactor = 1
        cfg.addEventAlgo(alg, pileupSequence.getName())

    return cfg


def GeneratorAnalysisSequenceCfg(flags, dataType):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.GeneratorAnalysisSequence import (
        makeGeneratorAnalysisSequence,
    )

    tags = flags.Input.AMITag.split("_")
    ptag = ""
    for tag in reversed(tags):
        if tag.startswith("p"):
            ptag = tag
            break
    if not ptag:
        log.warning(f"Did not find p-tag in AMI tags: {flags.Input.AMITag}")

    doCBK = ptag not in ["p5226", "p5278", "p5334"]
    generatorSequence = makeGeneratorAnalysisSequence(
        dataType,
        saveCutBookkeepers=doCBK,
        runNumber=flags.Input.RunNumber[0],
        cutBookkeepersSystematics=doCBK,
    )

    cfg.addSequence(CompFactory.AthSequencer(generatorSequence.getName()))
    for alg in generatorSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, generatorSequence.getName())

    return cfg
