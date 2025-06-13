# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from AthenaConfiguration.Enums import LHCPeriod

from EasyjetHub.steering.utils.name_helper import drop_sys
from EasyjetHub.steering.analysis_configuration import get_trigger_chains_scale_factor


def electron_sequence(flags, configAcc):

    wps = [(flags.Analysis.Electron.ID, flags.Analysis.Electron.Iso)]
    if 'extra_wps' in flags.Analysis.Electron:
        for wp in flags.Analysis.Electron.extra_wps:
            wps.append((wp[0], wp[1]))

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    output_name = drop_sys(flags.Analysis.container_names.output.electrons)

    if flags.Analysis.Electron.MergeLRT:
        configSeq += makeConfig('Electrons.LRTMerging')
        configSeq.setOptionValue('.containerName', 'Electrons_LRTMerged')

    configSeq += makeConfig('Electrons')
    configSeq.setOptionValue('.containerName', output_name)
    if flags.Analysis.Electron.MergeLRT:
        configSeq.setOptionValue('.inputContainer', 'Electrons_LRTMerged')
    configSeq.setOptionValue('.crackVeto', True)
    configSeq.setOptionValue('.decorrelationModel',
                             flags.Analysis.Electron.correlationModelScale)
    configSeq.setOptionValue('.writeTrackD0Z0', True)

    # PID configuration
    for id, iso in wps:
        configSeq += makeConfig('Electrons.WorkingPoint')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.selectionName', id + '_' + iso)
        if "nottva" in id:
            configSeq.setOptionValue('.trackSelection', False)
        else:
            configSeq.setOptionValue('.trackSelection', True)
        id = id.replace("_nottva", "")
        configSeq.setOptionValue('.identificationWP', id)
        configSeq.setOptionValue('.isolationWP', iso)
        configSeq.setOptionValue('.maxD0Significance',
                                 flags.Analysis.Electron.maxD0Significance)
        configSeq.setOptionValue('.maxDeltaZ0SinTheta',
                                 flags.Analysis.Electron.maxDeltaZ0SinTheta)
        configSeq.setOptionValue('.chargeIDSelectionRun2',
                                 flags.Analysis.Electron.chargeIDSelectionRun2
                                 and flags.GeoModel.Run is LHCPeriod.Run2)
        configSeq.setOptionValue('.correlationModelId',
                                 flags.Analysis.Electron.correlationModelId)
        configSeq.setOptionValue('.correlationModelIso',
                                 flags.Analysis.Electron.correlationModelIso)
        # Only TOTAL correlation model is currently supported
        # for reconstruction efficiency correction in Run 3
        configSeq.setOptionValue('.correlationModelReco',
                                 flags.Analysis.Electron.correlationModelReco)
        configSeq.setOptionValue('.saveCombinedSF', True)

        if flags.Input.isMC:
            # No DNN SF yet
            if "DNN" in id:
                print("WARNING! Electron DNN ID does not have SF available yet")
                configSeq.setOptionValue('.noEffSF', True)
            # No Run 2 SF yet
            if flags.GeoModel.Run is LHCPeriod.Run2:
                print("WARNING! Run 2 electron SF are not available yet")
                configSeq.setOptionValue('.noEffSF', True)
            # No NoPix SF yet
            if "NoPix" in id:
                print("WARNING! Electron NoPix ID does not have SF available yet")
                configSeq.setOptionValue('.noEffSF', True)

    # Electron trigger SF
    trigSF_flags = flags.Analysis.Trigger.scale_factor
    if trigSF_flags.doSF and hasattr(trigSF_flags, 'Electron'):
        configSeq += makeConfig('Electrons.TriggerSF')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.electronID', trigSF_flags.Electron.ID)
        configSeq.setOptionValue('.electronIsol', trigSF_flags.Electron.Iso)
        configSeq.setOptionValue('.triggerChainsPerYear',
                                 get_trigger_chains_scale_factor(flags, 'Electron'))

    # IFF truth decoration
    if flags.Analysis.Electron.do_IFF_decoration:
        configSeq += makeConfig('Electrons.IFFClassification')
        configSeq.setOptionValue('.containerName', output_name)

    # Kinematic selection
    configSeq += makeConfig('Electrons.PtEtaSelection')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', flags.Analysis.Electron.min_pT)
    configSeq.setOptionValue('.maxEta', flags.Analysis.Electron.max_eta)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink')
    configSeq.setOptionValue('.containerName', output_name)

    configSeq += makeConfig('Thinning')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')

    return configSeq
