def validate_flags(flags):

    validate_do_write_obj_flags(flags)

    validate_analysis_prerequisites(flags)

    validate_file_format(flags)


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


def validate_analysis_prerequisites(flags):
    if flags.Analysis.do_resolved_dihiggs_analysis:
        assert flags.Analysis.do_small_R_jets

    if flags.Analysis.do_boosted_dihiggs_analysis:
        assert flags.Analysis.do_large_R_Topo_jets


def validate_file_format(flags):
    if flags.Input.isPHYSLITE:
        try:
            assert not flags.Analysis.do_VR_jets
            assert not flags.Analysis.do_large_R_Topo_jets
        except AssertionError:
            raise RuntimeError("Collections requested are incompatible with PHYSLITE")
        assert not flags.Analysis.do_overlap_removal, "OR not needed on PHYSLITE"
    else:
        assert not flags.Analysis.disable_calib, (
            "Disabling calibrations is not safe except on PHYSLITE!"
        )
