from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from AthenaConfiguration.Enums import LHCPeriod


def get_photon_branches(flags, tree_flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    photon_branches = BranchManager(
        input_container,
        output_prefix,
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator
    )

    if tree_flags.slim_variables_with_syst:
        photon_branches.syst_only_for = ["pt"]

    photon_branches.add_four_mom_branches(do_mass=False)

    if flags.Analysis.do_overlap_removal:
        photon_branches.variables += ["passesOR_%SYS%"]

    id_wps = [f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}']
    if 'extra_wps' in flags.Analysis.Photon:
        for wp in flags.Analysis.Photon.extra_wps:
            id_wps.append(wp[0] + "_" + wp[1])

    photon_branches.variables += [
        f"baselineSelection_{id_wp}_%SYS%"
        for id_wp in id_wps
    ]

    if tree_flags.collection_options.photons.shower_shapes:
        photon_branches.variables += [
            "Rhad",
            "Rhad1",
            "Reta",
            "Rphi",
            "weta1",
            "weta2",
            "f1",
            "f3",
            "fracs1",
            "wtots1",
            "DeltaE",
            "Eratio",
        ]

    if tree_flags.collection_options.photons.iso_variables:
        photon_branches.variables += [
            "ptcone20",
            "topoetcone20",
            "topoetcone40",
        ]

    if flags.Input.isMC and tree_flags.collection_options.photons.truth_labels:
        photon_branches.variables += [
            "truthType",
            "truthOrigin",
        ]

    if flags.Input.isMC:
        # No Run 2 SF yet
        if flags.GeoModel.Run is LHCPeriod.Run3:
            photon_branches.variables += [
                f"effSF_{id_wp}_%SYS%"
                for id_wp in id_wps
            ]

    # Requires PhotonSelectorAlg to be run
    if tree_flags.collection_options.photons.run_selection:
        photon_branches.variables += ["isAnalysisPhoton_%SYS%"]
        for index in range(flags.Analysis.Photon.amount):
            photon_branches.variables += [f"isPhoton{index+1}_%SYS%"]

    return photon_branches.get_output_list()
