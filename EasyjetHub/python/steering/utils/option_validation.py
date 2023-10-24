from EasyjetHub.steering.utils.log_helper import log


def validate_flags(flags):

    validate_do_obj_flags(flags)

    for tree_flags in flags.Analysis.ttree_output:
        validate_do_write_obj_flags(flags, tree_flags)

    validate_file_format(flags)


def validate_do_obj_flags(flags):
    if flags.Analysis.do_met:
        assert flags.Analysis.do_small_R_jets, (
            "MET requires small R jets to be run"
        )
    if flags.Analysis.do_photons:
        assert flags.Analysis.PhotonID, (
            "Photons require ID working point e.g. PhotonID: 'Tight'"
        )
        assert flags.Analysis.PhotonIso, (
            "Photons require isolation working point e.g. PhotonIso: 'FixedCutLoose'"
        )


def validate_do_write_obj_flags(flags, tree_flags):
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
        'taus',
        'met'
    ]:
        try:
            do_obj = flags.Analysis[f'do_{objtype}']
            write_obj = tree_flags.reco_outputs[objtype]
        except Exception as e:
            log.error(f'Failed to retrieve do/write {objtype} flags')
            raise e
        if write_obj and not do_obj:
            raise RuntimeError(
                f'{tree_flags.tree_name}.reco_outputs.{objtype} defined'
                f' when do_{objtype}=False'
            )

        small_R_flags = tree_flags.collection_options.small_R_jets
        large_R_flags = tree_flags.collection_options.large_R_jets
        try:
            if any([
                small_R_flags.btag_info,
                small_R_flags.higgs_parent_info,
                small_R_flags.JVT_details,
                small_R_flags.no_bjet_calib_p4,
                small_R_flags.gn2_branches,
            ]):
                assert tree_flags.reco_outputs.small_R_jets
            if any([
                tree_flags.collection_options.large_R_jets.substructure_info,
            ]):
                assert any([
                    tree_flags.reco_outputs.large_R_Topo_jets,
                    tree_flags.reco_outputs.large_R_UFO_jets
                ])
        except AssertionError:
            raise RuntimeError(
                "Detailed branches requested when base container not written"
            )
        if flags.Input.isPHYSLITE:
            assert not any([
                small_R_flags.JVT_details,
                large_R_flags.truth_labels,
            ]), "Jet variables requested are incompatible with PHYSLITE"

        if small_R_flags.no_bjet_calib_p4:
            assert not flags.Analysis.disable_calib, (
                "B-jet momentum correction requires muon and b-jet CP algs"
            )


def validate_file_format(flags):
    if flags.Input.isPHYSLITE:

        assert not any([
            flags.Analysis.do_VR_jets,
            flags.Analysis.do_large_R_Topo_jets,
        ]), "Collections requested are incompatible with PHYSLITE"

    else:
        assert not flags.Analysis.disable_calib, (
            "Disabling calibrations is not safe except on PHYSLITE!"
        )
