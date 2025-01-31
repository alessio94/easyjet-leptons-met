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

    # Run GRL decoration optionally, already available in PHYSLITE
    if flags.Analysis.GRL.store_decoration and not flags.Input.isPHYSLITE:
        from GoodRunsLists.GoodRunsListsDictionary import getGoodRunsLists
        configSeq += makeConfig('EventCleaning')
        configSeq.setOptionValue('.noFilter', True)
        configSeq.setOptionValue('.useRandomRunNumber', flags.Input.isMC)
        configSeq.setOptionValue('.runPrimaryVertexSelection', False)
        configSeq.setOptionValue('.GRLDict', getGoodRunsLists())

    return configSeq


def trigger_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    configSeq += makeConfig('Trigger')
    configSeq.setOptionValue(
        '.triggerChainsForSelection',
        list(flags.Analysis.TriggerChains),
    )
    configSeq.setOptionValue('.noFilter',
                             not flags.Analysis.do_trigger_filtering)

    return configSeq
