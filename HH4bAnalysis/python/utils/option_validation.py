from HH4bAnalysis.utils.inputs_helper import is_physlite


def validate_flags(flags):
    # Arg checks
    assert not (
        flags.Analysis.disable_calib and not is_physlite(flags)
    ), "Disabling calibrations is not safe except on PHYSLITE!"

    validate_do_write_obj_flags(flags)

    validate_analysis_prerequisites(flags)


def validate_do_write_obj_flags(flags):
    # Verify output flags based on what CP algs are scheduled to run
    # TODO: Make a more consistent configuration of different execution branches
    # TODO: Validate which other features need which object types
    for objtype in [
        'small_R_jets',
        'large_R_Topo_jets',
        'large_R_UFO_jets',
        'VR_jets',
        'muons',
        'electrons',
        'photons',
    ]:
        do_obj = flags(f'Analysis.do_{objtype}')
        write_obj = flags(f'Analysis.write_{objtype}')
        if write_obj and not do_obj:
            raise RuntimeError(f'write_{objtype}=True when do_{objtype}=False')

    # Overlap removal only supports one large-R jet collection, so enforce
    # that only one is specified
    assert not (
        flags.Analysis.do_large_R_Topo_jets
        and flags.Analysis.do_large_R_UFO_jets
    ), 'Only one large-R jet collection (Topo or UFO) can be handled in a job'


def validate_analysis_prerequisites(flags):
    if flags.Analysis.do_resolved_dihiggs_analysis:
        assert flags.Analysis.do_small_R_jets

    if flags.Analysis.do_boosted_dihiggs_analysis:
        assert flags.Analysis.do_large_R_Topo_jets
