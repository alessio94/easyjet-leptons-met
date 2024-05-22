def get_selected_objects_branches(flags, analysis):
    object_level_branches, *_ = get_selected_objects_branches_variables(flags, analysis)
    return object_level_branches


def get_selected_objects_branches_variables(flags, analysis):
    branches = []
    float_variable_names = []
    int_variable_names = []

    # Checking if we need to store systematics for variables other than pT or SF.
    slim_variables_with_syst = \
        flags.Analysis.ttree_output[0].slim_variables_with_syst

    # All jets
    for var in [*flags.Analysis.small_R_jet.variables_int_allJets,
                *flags.Analysis.small_R_jet.variables_allJets]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.small_R_jet.amount):
            # Store the float and int variables
            if var in flags.Analysis.small_R_jet.variables_allJets:
                float_variable_names += [f"Jet{index+1}_{var}"]
            if var in flags.Analysis.small_R_jet.variables_int_allJets:
                int_variable_names += [f"Jet{index+1}_{var}"]
            # Translate the name to an analysis specific convention
            if slim_variables_with_syst and "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.Jet{index+1}_{var}_NOSYS \
                            -> {analysis}_Jet{index+1}_{var}"]
            else:
                branches += [f"EventInfo.Jet{index+1}_{var}_%SYS% \
                            -> {analysis}_Jet{index+1}_{var}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # B tagged jets
    for var in [*flags.Analysis.small_R_jet.variables_int_bjets,
                *flags.Analysis.small_R_jet.variables_bjets]:
        if not flags.Input.isMC and "SF" in var:
            continue
        for index in range(flags.Analysis.small_R_jet.amount_bjet):
            # Store the float and int variables
            if var in flags.Analysis.small_R_jet.variables_bjets:
                float_variable_names += [f"Jet_b{index+1}_{var}"]
            if var in flags.Analysis.small_R_jet.variables_int_bjets:
                int_variable_names += [f"Jet_b{index+1}_{var}"]
            # Translate the name to an analysis specific convention
            if slim_variables_with_syst and "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.Jet_b{index+1}_{var}_NOSYS \
                            -> {analysis}_Jet_b{index+1}_{var}"]
            else:
                branches += [f"EventInfo.Jet_b{index+1}_{var}_%SYS% \
                            -> {analysis}_Jet_b{index+1}_{var}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

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
            if slim_variables_with_syst and "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.Photon{index+1}_{var}_NOSYS \
                            -> {analysis}_Photon{index+1}_{var}"]
            else:
                branches += [f"EventInfo.Photon{index+1}_{var}_%SYS% \
                            -> {analysis}_Photon{index+1}_{var}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

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
            if slim_variables_with_syst and "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.Electron{index+1}_{var}_NOSYS \
                            -> {analysis}_Electron{index+1}_{var}"]
            else:
                branches += [f"EventInfo.Electron{index+1}_{var}_%SYS% \
                            -> {analysis}_Electron{index+1}_{var}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

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
            if slim_variables_with_syst and "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.Muon{index+1}_{var}_NOSYS \
                            -> {analysis}_Muon{index+1}_{var}"]
            else:
                branches += [f"EventInfo.Muon{index+1}_{var}_%SYS% \
                            -> {analysis}_Muon{index+1}_{var}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

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
            if slim_variables_with_syst and "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.Tau{index+1}_{var}_NOSYS \
                            -> {analysis}_Tau{index+1}_{var}"]
            else:
                branches += [f"EventInfo.Tau{index+1}_{var}_%SYS% \
                            -> {analysis}_Tau{index+1}_{var}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

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
            if slim_variables_with_syst and "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.Lepton{index_str}_{var}_NOSYS \
                            -> {analysis}_Lepton{index_str}_{var}"]
            else:
                branches += [f"EventInfo.Lepton{index_str}_{var}_%SYS% \
                            -> {analysis}_Lepton{index_str}_{var}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches, float_variable_names, int_variable_names
