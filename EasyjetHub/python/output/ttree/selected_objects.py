def get_selected_objects_branches(flags, analysis):
    object_level_branches, *_ = get_selected_objects_branches_variables(flags, analysis)
    return object_level_branches


def get_selected_objects_branches_variables(flags, analysis):
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
            branches += [f"EventInfo.Jet{index+1}_{var}_{sys_suffix} \
                        -> {analysis}_Jet{index+1}_{var}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

    # B tagged jets
    for var in [*flags.Analysis.Small_R_jet.variables_int_bjets,
                *flags.Analysis.Small_R_jet.variables_bjets]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.Small_R_jet.amount_bjet):
            # Store the float and int variables
            if var in flags.Analysis.Small_R_jet.variables_bjets:
                float_variable_names += [f"Jet_b{index+1}_{var}"]
            if var in flags.Analysis.Small_R_jet.variables_int_bjets:
                int_variable_names += [f"Jet_b{index+1}_{var}"]
            # Translate the name to an analysis specific convention
            branches += [f"EventInfo.Jet_b{index+1}_{var}_{sys_suffix} \
                        -> {analysis}_Jet_b{index+1}_{var}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

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
            branches += [f"EventInfo.LargeRJet{index+1}_{var}_{sys_suffix} \
                        -> {analysis}_LargeRJet{index+1}_{var}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

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
            branches += [f"EventInfo.Photon{index+1}_{var}_{sys_suffix} \
                        -> {analysis}_Photon{index+1}_{var}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

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
            branches += [f"EventInfo.Electron{index+1}_{var}_{sys_suffix} \
                        -> {analysis}_Electron{index+1}_{var}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

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
            branches += [f"EventInfo.Muon{index+1}_{var}_{sys_suffix} \
                        -> {analysis}_Muon{index+1}_{var}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

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
            branches += [f"EventInfo.Tau{index+1}_{var}_{sys_suffix} \
                        -> {analysis}_Tau{index+1}_{var}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

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
            branches += [f"EventInfo.Lepton{index_str}_{var}_{sys_suffix} \
                        -> {analysis}_Lepton{index_str}_{var}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

    return branches, float_variable_names, int_variable_names
