# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def semiLep_cfg(flags, muonkey, electronkey,
                float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    # Selection
    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.SemiLepSelectorAlg(
            "SemiLepSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            eventDecisionOutputDecoration="vbshiggs_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_cutflow,
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # calculate final vbshiggs vars
    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.BaselineVarsSemiLepAlg(
            "FinalVarsSemiLepAlg",
            isMC=flags.Input.isMC,
            muons=muonkey, muonWP=MuonWPLabel,
            electrons=electronkey, eleWP=ElectronWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            PCBTDecorName="ftag_quantile_"
                          + flags.Analysis.Small_R_jet.btag_extra_wps[0],
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    return cfg


def get_BaselineVarsSemiLepAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    obj = "LargeJet1"
    for var in ["m", "pt", "eta", "phi", "GN2X"]:
        float_variable_names.append(f"{obj}_{var}")
    for var in ["phbb", "pqcd", "phcc", "ptop"]:
        float_variable_names.append(f"{obj}_GN2X_{var}")

    for obj in ["Jet_Higgs_candidate1", "Jet_Higgs_candidate2"]:
        for var in ["m", "pt", "eta", "phi", "E"]:
            float_variable_names.append(f"{obj}_{var}")
        for var in ["pcbt", "truthLabel"]:
            int_variable_names.append(f"{obj}_{var}")

    for obj in ["Hdijet", "resolvedWCandidate1",
                "resolvedWCandidate2", "boostedWCandidate1",
                "boostedWCandidate2", "boostedWdijet", "resolvedWdijet",
                "VBSJ1", "VBSJ2", "VBSdijet"]:
        for var in ["m", "pt", "eta", "phi"]:
            float_variable_names.append(f"{obj}_{var}")

    obj = "VBSjj"
    for var in ["dR", "dEta", "dPhi"]:
        float_variable_names.append(f"{var}{obj}")

    float_variable_names += ["dRHjj", "dPhiHjj", "dEtaHjj", "METSig", "W_mT"]

    int_variable_names += ["nLeptons", "nSigJets", "nBJets", "nCentralJets",
                           "nForwardJets", "nJets", "nLargeJets"]

    return float_variable_names, int_variable_names


def semiLep_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsSemiLepAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsSemiLepAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> {var}_%SYS%"]

    # These are the variables always saved with the objects
    # selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "SemiLep")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches
    branches += ["EventInfo.vbshiggs_pass_sr_%SYS% -> pass_SR_%SYS%"]

    if (flags.Analysis.save_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> {cut}_%SYS%"]

    for cat in ["SLT"]:
        branches += \
            [f"EventInfo.pass_trigger_{cat}_%SYS% -> pass_trigger_{cat}"
             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches, float_variable_names, int_variable_names
