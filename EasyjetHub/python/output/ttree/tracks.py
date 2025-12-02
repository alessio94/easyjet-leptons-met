# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Still WIP - follows pattern of muons.py and electrons.py
# Not fully tested!!

from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_track_branches(flags, tree_flags, input_container, output_prefix):
    """
    Configure track branches for TTree output.
    Similar to muons.py and electrons.py but for tracks.
    """
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    track_output_flags = tree_flags.collection_options.tracks

    track_branches = BranchManager(
        input_container,
        output_prefix,
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator
    )

    # Only systematic variations on d0/z0
    # All other variables have noSys=True in InDetTrackCalibrationConfig
    if tree_flags.slim_variables_with_syst:
        track_branches.syst_only_for = ["d0", "z0"]

    # Core kinematic variables (pt, eta, phi have noSys=True in calibration)
    # For some reason, pt,eta just doesn't work - works in the manual filling though...
    # TODO: Figure out why...
    track_branches.variables += ["pt", "eta", "phi"]

    # other variables (noSys=True in calibration)
    track_branches.variables += ["charge", "qOverP"]

    # Impact parameters I think - these HAVE systematics (d0/z0 smearing in MC)
    track_branches.variables += ["d0", "z0"]

    # Vertex position (noSys=True in calibration)
    track_branches.variables += ["vz"]

    # Track summary information (detector hits, etc.)
    # Only if configured to output this info
    # Note: These are only available if outputTrackSummaryInfo=True in calibration
    if flags.Analysis.Track.outputTrackSummaryInfo:
        track_branches.variables += [
            "numberOfInnermostPixelLayerHits_NOSYS",
            "numberOfPixelDeadSensors_NOSYS",
            "numberOfPixelHits_NOSYS",
            "numberOfPixelHoles_NOSYS",
            "numberOfPixelSharedHits_NOSYS",
            "numberOfSCTDeadSensors_NOSYS",
            "numberOfSCTHits_NOSYS",
            "numberOfSCTHoles_NOSYS",
            "numberOfSCTSharedHits_NOSYS",
            "numberOfTRTHits_NOSYS",
            "numberOfTRTOutliers_NOSYS",
        ]

    # Selection decorations for working points
    if flags.Analysis.Track.working_points:
        for wp in flags.Analysis.Track.working_points:
            track_branches.variables += [f"baselineSelection_{wp}_%SYS%"]

    # Kinematic selection decorations (created by InDetTracks calibration)
    # Note: InDetTracks creates selectPt and selectEta separately, not selectPtEta
    # Commenting out for now - may not be needed
    # track_branches.variables += ["selectPt_%SYS%", "selectEta_%SYS%"]

    # Extra user-specified variables
    if track_output_flags:
        track_branches.variables += track_output_flags.extra_variables

    # MC-specific variables
    if flags.Input.isMC:
        if track_output_flags:
            track_branches.variables += track_output_flags.mc_extra_variables

    return track_branches.get_output_list()
