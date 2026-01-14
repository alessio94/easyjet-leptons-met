# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Track calibration and selection sequence for easyjet framework
# Again, WIP - follows pattern of muons.py and electrons.py

from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.steering.utils.name_helper import drop_sys


def track_sequence(flags, configAcc):
    """
    Configure the track calibration and selection sequence using
    the TrackingAnalysisConfig from Athena
    """

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    track_flags = flags.Analysis.Track
    output_name = drop_sys(flags.Analysis.container_names.output.tracks)

    # Main track calibration block
    # This includes pt/eta selection via minPt/maxEta options
    configSeq += makeConfig('InDetTracks')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.minPt', track_flags.min_pT)
    configSeq.setOptionValue('.maxEta', track_flags.max_eta)
    configSeq.setOptionValue('.outputTrackSummaryInfo',
                             track_flags.outputTrackSummaryInfo)

    # Configure working points if specified
    for wp in track_flags.working_points:
        configSeq += makeConfig('InDetTracks.WorkingPoint')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.selectionName', wp)
        configSeq.setOptionValue('.cutLevel', wp)

    # Add systematic object links (for MC systematics)
    configSeq += makeConfig('SystObjectLink')
    configSeq.setOptionValue('.containerName', output_name)

    return configSeq
