# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from AthenaConfiguration.Enums import LHCPeriod

from EasyjetHub.steering.utils.name_helper import drop_sys


def photon_sequence(flags, configAcc):

    wps = [(flags.Analysis.Photon.ID, flags.Analysis.Photon.Iso)]
    if 'extra_wps' in flags.Analysis.Photon:
        for wp in flags.Analysis.Photon.extra_wps:
            wps.append((wp[0], wp[1]))

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.photons)
    configSeq += makeConfig('Photons')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.recomputeIsEM', False)
    configSeq.setOptionValue('.crackVeto', True)
    configSeq.setOptionValue('.decorrelationModel',
                             flags.Analysis.Photon.correlationModelScale)

    # PID configuration
    for id, iso in wps:
        configSeq += makeConfig('Photons.WorkingPoint')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.selectionName', id + '_' + iso)
        # To be removed with 25.2.57
        configSeq.setOptionValue('.postfix', id + '_' + iso)
        configSeq.setOptionValue('.qualityWP', id)
        configSeq.setOptionValue('.isolationWP', iso)
        configSeq.setOptionValue('.saveCombinedSF', True)

        # No Run 2 SF yet
        if flags.GeoModel.Run is LHCPeriod.Run2 and flags.Input.isMC:
            print("WARNING! Run 2 photon SF are not available yet")
            configSeq.setOptionValue('.noEffSFForIso', True)

    # Kinematic selection
    configSeq += makeConfig('Photons.PtEtaSelection')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', flags.Analysis.Photon.min_pT)
    configSeq.setOptionValue('.maxEta', flags.Analysis.Photon.max_eta)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink')
    configSeq.setOptionValue('.containerName', output_name)

    configSeq += makeConfig('Thinning')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')

    return configSeq
