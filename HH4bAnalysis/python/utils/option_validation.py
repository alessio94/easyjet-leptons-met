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

        try:
            if any([
                flags.Analysis.write_small_R_btag,
                flags.Analysis.write_small_R_higgs_parent_info,
                flags.Analysis.write_small_R_JVT_details,
                flags.Analysis.write_small_R_no_bjet_calib,
                flags.Analysis.write_small_R_gn2_branches,
            ]):
                assert flags.Analysis.write_small_R_jets
            if any([
                flags.Analysis.write_large_R_substructure,
            ]):
                assert any([
                    flags.Analysis.write_large_R_Topo_jets,
                    flags.Analysis.write_large_R_UFO_jets
                ])
        except AssertionError:
            raise RuntimeError(
                "Detailed branches requested when base container not written"
            )

        if flags.Analysis.write_small_R_no_bjet_calib:
            assert not flags.Analysis.disable_calib, (
                "B-jet momentum correction requires muon and b-jet CP algs"
            )


def validate_analysis_prerequisites(flags):
    if flags.Analysis.do_resolved_dihiggs_analysis:
        assert flags.Analysis.do_small_R_jets
        assert not flags.Analysis.disable_calib

    if flags.Analysis.do_boosted_dihiggs_analysis:
        assert flags.Analysis.do_large_R_Topo_jets
        assert not flags.Analysis.disable_calib

    if flags.Analysis.do_yybb_analysis:
        assert flags.Analysis.do_small_R_jets


def validate_file_format(flags):
    if flags.Input.isPHYSLITE:

        assert not any([
            flags.Analysis.do_VR_jets,
            flags.Analysis.do_large_R_Topo_jets,
            flags.Analysis.write_small_R_JVT_details,
            flags.Analysis.write_large_R_truth_labels
        ]), "Collections/variables requested are incompatible with PHYSLITE"

        assert not flags.Analysis.do_overlap_removal, "OR not needed on PHYSLITE"
    else:
        assert not flags.Analysis.disable_calib, (
            "Disabling calibrations is not safe except on PHYSLITE!"
        )
