from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from EasyjetHub.steering.sample_metadata import get_grl_files


def event_selection_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    configSeq += makeConfig('EventCleaning')
    configSeq.setOptionValue('.runPrimaryVertexSelection',
                             flags.Analysis.do_event_cleaning)
    configSeq.setOptionValue('.runEventCleaning', flags.Analysis.do_event_cleaning)
    configSeq.setOptionValue('.userGRLFiles', get_grl_files(flags))

    if not flags.Analysis.loose_jet_cleaning:
        configSeq.setOptionValue(
            '.selectionFlags',
            [
                'DFCommonJets_eventClean_LooseBad',
                'DFCommonJets_isBadBatman',
            ],
        )
        configSeq.setOptionValue(
            '.invertFlags',
            [
                False,
                True,
            ],
        )

    return configSeq


def trigger_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # TODO: We could pass trigger chains per year to also apply scale factors
    # However, for that the sequence needs to be pushed to after the calibration,
    # while we want to have the earliest possible rejection. So we may need to
    # split this and/or see if the config blocks need reworking
    configSeq += makeConfig('Trigger')
    configSeq.setOptionValue(
        '.triggerChainsForSelection',
        list(flags.Analysis.TriggerChains),
    )
    configSeq.setOptionValue('.noFilter', not flags.Analysis.do_trigger_filtering)

    return configSeq
