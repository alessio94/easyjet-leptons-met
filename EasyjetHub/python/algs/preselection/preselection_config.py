from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def event_selection_sequence_cfg(flags, grlfiles=[], loose=False):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.EventSelectionAnalysisSequence import (
        makeEventSelectionAnalysisSequence,
    )

    event_selection_sequence = makeEventSelectionAnalysisSequence(
        flags.Analysis.DataType, userGRLFiles=grlfiles, runEventCleaning=True
    )

    cfg.addSequence(CompFactory.AthSequencer(event_selection_sequence.getName()))
    for alg in event_selection_sequence.getGaudiConfig2Components():
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

        cfg.addEventAlgo(alg, event_selection_sequence.getName())

    return cfg


def trigger_sequence_cfg(flags,):
    cfg = ComponentAccumulator()
    from TriggerAnalysisAlgorithms.TriggerAnalysisSequence import (
        makeTriggerAnalysisSequence,
    )

    trigger_sequence = makeTriggerAnalysisSequence(
        flags.Analysis.DataType,
        triggerChains=flags.Analysis.TriggerChains,
        noFilter=False if flags.Analysis.do_trigger_filtering else True,
    )

    cfg.addSequence(CompFactory.AthSequencer(trigger_sequence.getName()))
    for alg in trigger_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, trigger_sequence.getName())

    return cfg
