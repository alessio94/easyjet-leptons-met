def get_selected_objects_branches(flags, analysis):
    object_level_branches, *_ = get_selected_objects_branches_variables(
        flags, analysis)
    return object_level_branches


def get_selected_objects_branches_variables(flags, analysis):
    branches = []
    float_variable_names = []
    int_variable_names = []

    # functions in a dictionary
    get_selected_X_branches_variables = {
        "Jet": get_selected_jet_branches_variables,
        "Jet_b": lambda flags, analysis: get_selected_jet_tagged_branches_variables(
            flags, analysis, 'b'
        ),
        "Jet_c": lambda flags, analysis: get_selected_jet_tagged_branches_variables(
            flags, analysis, 'c'
        ),
        "Jet_l": lambda flags, analysis: get_selected_jet_tagged_branches_variables(
            flags, analysis, 'l'
        ),
        "LargeRJet": get_selected_largeR_jet_branches_variables,
        "Photon": get_selected_photon_branches_variables,
        "Electron": get_selected_electron_branches_variables,
        "Muon": get_selected_muon_branches_variables,
        "Tau": get_selected_tau_branches_variables,
        "Lepton": get_selected_lepton_branches_variables,
    }

    for object_type in get_selected_X_branches_variables.keys():
        _branches, _float_variable_names, _int_variable_names = \
            get_selected_X_branches_variables[object_type](flags, analysis)
        branches += _branches
        float_variable_names += _float_variable_names
        int_variable_names += _int_variable_names

    return branches, float_variable_names, int_variable_names


def get_selected_jet_branches_variables(flags, analysis):
    branches = []
    float_variable_names = []
    int_variable_names = []

    if not flags.Analysis.do_CP_systematics:
        sys_suffix = "NOSYS"
    else:
        sys_suffix = "%SYS%"

    # All jets
    for var in [*flags.Analysis.Small_R_jet.variables_int_allJets,
                *flags.Analysis.Small_R_jet.variables_allJets]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.Small_R_jet.amount):
            # Store the float and int variables
            if var in flags.Analysis.Small_R_jet.variables_allJets:
                float_variable_names += [f"Jet{index+1}_{var}"]
            if var in flags.Analysis.Small_R_jet.variables_int_allJets:
                int_variable_names += [f"Jet{index+1}_{var}"]
            # Translate the name to an analysis specific convention
            branch_str = (
                f"EventInfo.Jet{index+1}_{var}_{sys_suffix} "
                f"-> {analysis}_Jet{index+1}_{var}"
                f"{flags.Analysis.systematics_suffix_separator}{sys_suffix}"
            )
            branches += [branch_str]

    return branches, float_variable_names, int_variable_names


def get_selected_jet_tagged_branches_variables(flags, analysis, jet_type):
    """
    jet_type: 'b', 'c', or 'l' for b-tagged, c-tagged, or untagged jets
    """
    branches = []
    float_variable_names = []
    int_variable_names = []

    if not flags.Analysis.do_CP_systematics:
        sys_suffix = "NOSYS"
    else:
        sys_suffix = "%SYS%"

    config_map = {
        'b': {
            'var_list': flags.Analysis.Small_R_jet.variables_bjets,
            'int_var_list': flags.Analysis.Small_R_jet.variables_int_bjets,
            'amount': flags.Analysis.Small_R_jet.amount_bjet,
            'prefix': 'Jet_b'
        },
        'c': {
            'var_list': flags.Analysis.Small_R_jet.variables_cjets,
            'int_var_list': flags.Analysis.Small_R_jet.variables_int_cjets,
            'amount': flags.Analysis.Small_R_jet.amount_cjet,
            'prefix': 'Jet_c'
        },
        'l': {
            'var_list': flags.Analysis.Small_R_jet.variables_ljets,
            'int_var_list': flags.Analysis.Small_R_jet.variables_int_ljets,
            'amount': flags.Analysis.Small_R_jet.amount_ljet,
            'prefix': 'Jet_l'
        }
    }

    config = config_map[jet_type]

    for var in [*config['int_var_list'], *config['var_list']]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(config['amount']):

            if var in config['var_list']:
                float_variable_names += [f"{config['prefix']}{index+1}_{var}"]
            if var in config['int_var_list']:
                int_variable_names += [f"{config['prefix']}{index+1}_{var}"]

            branch_str = (
                f"EventInfo.{config['prefix']}{index+1}_{var}_{sys_suffix} "
                f"-> {analysis}_{config['prefix']}{index+1}_{var}"
                f"{flags.Analysis.systematics_suffix_separator}{sys_suffix}"
            )
            branches += [branch_str]

    return branches, float_variable_names, int_variable_names


def get_selected_largeR_jet_branches_variables(flags, analysis):
    branches = []
    float_variable_names = []
    int_variable_names = []

    if not flags.Analysis.do_CP_systematics:
        sys_suffix = "NOSYS"
    else:
        sys_suffix = "%SYS%"

    # Large R jets
    for var in [*flags.Analysis.Large_R_jet.variables_int_LargeRJets,
                *flags.Analysis.Large_R_jet.variables_LargeRJets]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.Large_R_jet.amount):
            # Store the float and int variables
            if var in flags.Analysis.Large_R_jet.variables_LargeRJets:
                float_variable_names += [f"LargeRJet{index+1}_{var}"]
            if var in flags.Analysis.Large_R_jet.variables_int_LargeRJets:
                int_variable_names += [f"LargeRJet{index+1}_{var}"]
            # Translate the name to an analysis specific convention
            branch_str = (
                f"EventInfo.LargeRJet{index+1}_{var}_{sys_suffix} "
                f"-> {analysis}_LargeRJet{index+1}_{var}"
                f"{flags.Analysis.systematics_suffix_separator}{sys_suffix}"
            )
            branches += [branch_str]

    return branches, float_variable_names, int_variable_names


def get_selected_photon_branches_variables(flags, analysis):
    branches = []
    float_variable_names = []
    int_variable_names = []

    if not flags.Analysis.do_CP_systematics:
        sys_suffix = "NOSYS"
    else:
        sys_suffix = "%SYS%"

    # Photons
    for var in [*flags.Analysis.Photon.variables,
                *flags.Analysis.Photon.variables_int]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.Photon.amount):
            # Store the float and int variables
            if var in flags.Analysis.Photon.variables:
                float_variable_names += [f"Photon{index+1}_{var}"]
            if var in flags.Analysis.Photon.variables_int:
                int_variable_names += [f"Photon{index+1}_{var}"]
            # Translate the name to an analysis specific convention
            branch_str = (
                f"EventInfo.Photon{index+1}_{var}_{sys_suffix} "
                f"-> {analysis}_Photon{index+1}_{var}"
                f"{flags.Analysis.systematics_suffix_separator}{sys_suffix}"
            )
            branches += [branch_str]

    return branches, float_variable_names, int_variable_names


def get_selected_electron_branches_variables(flags, analysis):
    branches = []
    float_variable_names = []
    int_variable_names = []

    if not flags.Analysis.do_CP_systematics:
        sys_suffix = "NOSYS"
    else:
        sys_suffix = "%SYS%"

    # Electrons
    for var in [*flags.Analysis.Electron.variables,
                *flags.Analysis.Electron.variables_int]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.Electron.amount):
            # Store the float and int variables
            if var in flags.Analysis.Electron.variables:
                float_variable_names += [f"Electron{index+1}_{var}"]
            if var in flags.Analysis.Electron.variables_int:
                int_variable_names += [f"Electron{index+1}_{var}"]
            # Translate the name to an analysis specific convention
            branch_str = (
                f"EventInfo.Electron{index+1}_{var}_{sys_suffix} "
                f"-> {analysis}_Electron{index+1}_{var}"
                f"{flags.Analysis.systematics_suffix_separator}{sys_suffix}"
            )
            branches += [branch_str]

    return branches, float_variable_names, int_variable_names


def get_selected_muon_branches_variables(flags, analysis):
    branches = []
    float_variable_names = []
    int_variable_names = []

    if not flags.Analysis.do_CP_systematics:
        sys_suffix = "NOSYS"
    else:
        sys_suffix = "%SYS%"

    # Muons
    for var in [*flags.Analysis.Muon.variables,
                *flags.Analysis.Muon.variables_int]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.Muon.amount):
            # Store the float and int variables
            if var in flags.Analysis.Muon.variables:
                float_variable_names += [f"Muon{index+1}_{var}"]
            if var in flags.Analysis.Muon.variables_int:
                int_variable_names += [f"Muon{index+1}_{var}"]
            # Translate the name to an analysis specific convention
            branch_str = (
                f"EventInfo.Muon{index+1}_{var}_{sys_suffix} "
                f"-> {analysis}_Muon{index+1}_{var}"
                f"{flags.Analysis.systematics_suffix_separator}{sys_suffix}"
            )
            branches += [branch_str]

    return branches, float_variable_names, int_variable_names


def get_selected_tau_branches_variables(flags, analysis):
    branches = []
    float_variable_names = []
    int_variable_names = []

    if not flags.Analysis.do_CP_systematics:
        sys_suffix = "NOSYS"
    else:
        sys_suffix = "%SYS%"

    # Taus
    for var in [*flags.Analysis.Tau.variables,
                *flags.Analysis.Tau.variables_int]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.Tau.amount):
            # Store the float and int variables
            if var in flags.Analysis.Tau.variables:
                float_variable_names += [f"Tau{index+1}_{var}"]
            if var in flags.Analysis.Tau.variables_int:
                int_variable_names += [f"Tau{index+1}_{var}"]
            # Translate the name to an analysis specific convention
            branch_str = (
                f"EventInfo.Tau{index+1}_{var}_{sys_suffix} "
                f"-> {analysis}_Tau{index+1}_{var}"
                f"{flags.Analysis.systematics_suffix_separator}{sys_suffix}"
            )
            branches += [branch_str]

    return branches, float_variable_names, int_variable_names


def get_selected_lepton_branches_variables(flags, analysis):
    branches = []
    float_variable_names = []
    int_variable_names = []

    if not flags.Analysis.do_CP_systematics:
        sys_suffix = "NOSYS"
    else:
        sys_suffix = "%SYS%"

    # Selected Lepton
    for var in [*flags.Analysis.Lepton.variables,
                *flags.Analysis.Lepton.variables_int]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.Lepton.amount):
            index_str = "" if flags.Analysis.Lepton.amount == 1 else f"{index+1}"
            if var in flags.Analysis.Lepton.variables:
                float_variable_names += [f"Lepton{index_str}_{var}"]
            if var in flags.Analysis.Lepton.variables_int:
                int_variable_names += [f"Lepton{index_str}_{var}"]
            # Translate the name to an analysis specific convention
            branch_str = (
                f"EventInfo.Lepton{index_str}_{var}_{sys_suffix} "
                f"-> {analysis}_Lepton{index_str}_{var}"
                f"{flags.Analysis.systematics_suffix_separator}{sys_suffix}"
            )
            branches += [branch_str]

    return branches, float_variable_names, int_variable_names
