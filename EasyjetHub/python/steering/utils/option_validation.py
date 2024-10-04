from EasyjetHub.steering.utils.log_helper import log


def validate_flags(flags):

    validate_do_obj_flags(flags)

    if hasattr(flags.Analysis, "ttree_output"):
        tree_flags = flags.Analysis.ttree_output
        validate_do_write_obj_flags(flags, tree_flags)

    validate_file_format(flags)

    validate_systematics_flags(flags)

    validate_output(flags)


def validate_do_obj_flags(flags):
    if flags.Analysis.do_met:
        assert flags.Analysis.do_small_R_jets, (
            "MET requires small R jets to be run"
        )
    if flags.Analysis.do_photons:
        assert flags.Analysis.Photon.ID, (
            "Photons require ID working point e.g. Photon.ID: 'Tight'"
        )
        assert flags.Analysis.Photon.Iso, (
            "Photons require isolation working point e.g. Photon.Iso: 'FixedCutLoose'"
        )
    if flags.Analysis.do_electrons:
        assert flags.Analysis.Electron.ID, (
            "Electrons require ID working point e.g. Electron.ID: 'LooseBLayerLH'"
        )
        assert flags.Analysis.Electron.Iso, (
            "Electrons require isolation working point e.g. Electron.Iso: 'NonIso'"
        )
    if flags.Analysis.do_muons:
        assert flags.Analysis.Muon.ID, (
            "Muons require ID working point e.g. Muon.ID: 'Loose'"
        )
        assert flags.Analysis.Muon.Iso, (
            "Muons require isolation working point e.g. Muon.Iso: 'Loose_VarRad'"
        )
    if flags.Analysis.do_taus:
        assert flags.Analysis.Tau.ID, (
            "Taus require ID working point e.g. Tau.ID: 'Loose'"
        )
        if flags.Input.isPHYSLITE:
            assert not flags.Analysis.Tau.addMuonRM, (
                "TauJets_MuonRM are not saved in PHYSLITE yet"
            )
    if flags.Analysis.do_small_R_jets and flags.Analysis.Small_R_jet.runBJetPtCalib:
        assert flags.Analysis.do_muons, (
            "B-jet pT calibration requires muons to be run"
        )
        assert len(flags.Analysis.Small_R_jet.btag_wp) > 0, (
            "B-jet pT calibration requires some btag WP to be set"
        )
        assert "Continuous" not in flags.Analysis.Small_R_jet.btag_wp, (
            "Pseudo-continuous b-tagging cannot be used as nominal btag_wp with B-jet "
            "pT calibration. A fixed cut should be used for nominal and PCBT can be "
            "used as btag_extra_wp"
        )
    if flags.Analysis.OverlapRemoval.do_small_R_jet_large_R_jet:
        assert flags.Analysis.OverlapRemoval.do_large_R_jets, (
            "Small-R / Large-R jets overlap removal requires for the Large-R jets "
            "overlap removal to be enabled"
        )
    if flags.Analysis.do_large_R_UFO_jets \
       and flags.Analysis.Large_R_jet.runMuonJetPtCorr:
        assert flags.Analysis.do_muons, (
            "Large-R jets muon-in-jet correction requires muons to be run"
        )


def validate_do_write_obj_flags(flags, tree_flags):
    # Verify output flags based on what CP algs are scheduled to run
    # TODO: Make a more consistent configuration of different execution branches
    # TODO: Validate which other features need which object types
    for objtype in [
        'small_R_jets',
        'large_R_Topo_jets',
        'large_R_UFO_jets',
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
        try:
            if any([
                small_R_flags.btag_info,
                small_R_flags.truth_parent_info,
                small_R_flags.JVT_details,
                small_R_flags.no_bjet_calib_p4,
                small_R_flags.btag_details,
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
            ]), "Jet variables requested are incompatible with PHYSLITE"

        if small_R_flags.no_bjet_calib_p4:
            assert not flags.Analysis.disable_calib, (
                "B-jet momentum correction requires muon and b-jet CP algs"
            )


def validate_file_format(flags):
    if flags.Input.isPHYSLITE:

        assert not any([
            flags.Analysis.do_large_R_Topo_jets,
        ]), "Collections requested are incompatible with PHYSLITE"

    else:
        assert not flags.Analysis.disable_calib, (
            "Disabling calibrations is not safe except on PHYSLITE!"
        )


def validate_systematics_flags(flags):
    if flags.Analysis.do_CP_systematics:
        assert flags.Analysis.systematics_regex, (
            "Systematics requested but no regex provided "
            "e.g. for all systematics_regex: ['.*']"
        )


def validate_output(flags):
    if flags.Analysis.splitOutputTree:
        assert len(flags.Analysis.channels) > 1, (
            "Should configure at least two channels to "
            "split output trees"
        )
        assert not flags.Analysis.bypass, (
            "Should not be in bypass mode to split output trees by channel. "
            "Where would the events in no channel go?"
        )
        assert flags.Analysis.splitCBK, (
            "SplitCBK should be set to true, to store CutBookKeeper in separate file"
        )
