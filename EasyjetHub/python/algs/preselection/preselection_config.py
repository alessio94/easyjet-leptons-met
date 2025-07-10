from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from EasyjetHub.steering.sample_metadata import get_grl_files


def event_selection_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    configSeq += makeConfig('EventCleaning')
    configSeq.setOptionValue('.runPrimaryVertexSelection',
                             flags.Analysis.do_event_cleaning)
    configSeq.setOptionValue('.runEventCleaning', flags.Analysis.do_event_cleaning)

    selectionFlags = ['DFCommonJets_eventClean_LooseBad']
    if flags.Analysis.do_tight_jet_cleaning:
        selectionFlags = ['DFCommonJets_eventClean_TightBad']
    invertFlags = [False]

    if flags.Analysis.add_BadBatman_jet_cleaning:
        selectionFlags += ['DFCommonJets_isBadBatman']
        invertFlags += [True]

    configSeq.setOptionValue('.selectionFlags', selectionFlags)
    configSeq.setOptionValue('.invertFlags', invertFlags)

    configSeq.setOptionValue('.runGRL', flags.Analysis.GRL.runGRL)

    if flags.Analysis.GRL.runGRL:
        configSeq.setOptionValue('.userGRLFiles', get_grl_files(flags))

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
    configSeq.setOptionValue(
        '.triggerChainsForDecoration',
        list(flags.Analysis.TriggerChainsDeco),
    )
    configSeq.setOptionValue('.noFilter',
                             not flags.Analysis.do_trigger_filtering)

    return configSeq


def overlap_VGammaOR(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    configSeq += makeConfig('VGammaOR')
    configSeq.setOptionValue('noFilter', flags.Analysis.bypass_VGammaOR)
    if flags.Analysis.VGammaOR_ypT_cuts:
        configSeq.setOptionValue('photon_pT_cuts',
                                 list(flags.Analysis.VGammaOR_ypT_cuts))
    if flags.Analysis.VGammaOR_dR_lepton_photon_cuts:
        configSeq.setOptionValue('dR_lepton_photon_cuts',
                                 list(flags.Analysis.VGammaOR_dR_lepton_photon_cuts))
    # Add option later if any analysis needs to modify the default list
    # configSeq.setOptionValue('keepInOverlap',
    # [700011, 700012, 700013, 700014, 700015, 700016, 700017])
    # Added DSIDs correpsonding to the needs of XbbCalib, VBSHiggs analysis
    configSeq.setOptionValue('removeInOverlap',
                             list(flags.Analysis.Truth.DSID_vgammaOR))

    return configSeq


def secondary_vertex_filter_cfg(flags):
    cfg = ComponentAccumulator()

    LLP1VrtSecInclusiveSuffix = flags.Analysis.VSI_container_suffix
    vertex_collection = "VrtSecInclusive_SecondaryVertices" + LLP1VrtSecInclusiveSuffix
    cfg.addEventAlgo(
        CompFactory.Easyjet.SecVtxFilterAlg(
            "SecVtxFilterAlg", vertexIn=vertex_collection
        )
    )

    return cfg
