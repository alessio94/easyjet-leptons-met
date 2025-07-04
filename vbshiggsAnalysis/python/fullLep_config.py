# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def fullLep_cfg(flags, muonkey, electronkey, float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    # set jets container labels
    if flags.Analysis.UseVBFRNN:
        # vbs tagging jets no more
        VBSJetsLabel = ""
        # use full small-R jets pool to select signal jets
        SignalJetsLabel = "vbshiggsAnalysisJets_%SYS%"
    else:
        # use vbs tagging jets
        VBSJetsLabel = "vbshiggsAnalysisVBSJets_%SYS%"
        # use the small-R jets after selecting tagging jets
        SignalJetsLabel = "vbshiggsAnalysisSignalJets_%SYS%"
    # Selection
    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.FullLepSelectorAlg(
            "FullLepSelectorAlg",
            signaljets=SignalJetsLabel,
            UseVBFRNN=flags.Analysis.UseVBFRNN,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            eventDecisionOutputDecoration="vbshiggs_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_cutflow,
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
            do_resolved=flags.Analysis.do_resolved
        )
    )

    # calculate final vbshiggs vars
    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.BaselineVarsFullLepAlg(
            "FinalVarsFullLepAlg",
            signaljets=SignalJetsLabel,
            UseVBFRNN=flags.Analysis.UseVBFRNN,
            vbsJets=VBSJetsLabel,
            isMC=flags.Input.isMC,
            electrons=electronkey, eleWP=ElectronWPLabel,
            muons=muonkey, muonWP=MuonWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            PCBTDecorName="ftag_quantile_"
                          + flags.Analysis.Small_R_jet.btag_extra_wps[0],
            floatVariableList=float_variables,
            intVariableList=int_variables,
            do_resolved=flags.Analysis.do_resolved
        )
    )
    return cfg


def get_BaselineVarsFullLepAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    objects = ["LargeJet1", "ll"]
    if flags.Analysis.do_resolved:
        objects += ["Hdijet", "Hj1l1", "Hj2l2"]

    if not flags.Analysis.UseVBFRNN:
        objects += ["VBSJ1", "VBSJ2", "VBSdijet"]
    else:
        objects += ["RNNJets_boostedCategory_Jet1",
                    "RNNJets_boostedCategory_Jet2"]
        if flags.Analysis.do_resolved:
            objects += ["RNNJets_resolvedCategory_Jet1",
                        "RNNJets_resolvedCategory_Jet2"]

    for object in objects:
        for var in ["m", "pt", "eta", "phi"]:
            float_variable_names.append(f"{object}_{var}")

    objects = ["ll"]
    if flags.Analysis.do_resolved:
        objects += ["Hjj", "Hj1l1", "Hj2l2"]
    if not flags.Analysis.UseVBFRNN:
        objects += ["VBSjj"]
    for object in objects:
        for var in ["dR", "dEta", "dPhi"]:
            float_variable_names.append(f"{var}{object}")

    if flags.Analysis.do_resolved:
        for object in ["Jet_Higgs_candidate1", "Jet_Higgs_candidate2"]:
            for var in ["m", "pt", "eta", "phi", "E"]:
                float_variable_names.append(f"{object}_{var}")
            for var in ["pcbt", "truthLabel"]:
                int_variable_names.append(f"{object}_{var}")

        float_variable_names += ["Hdijetll_m", "Hdijetllmet_m"]

    float_variable_names += ["dPhillMET", "dPhil1MET", "dPhil2MET", "METSig",
                             "Lepton1_MET_mT", "Lepton2_MET_mT", "LargeJet1_DXbb"]

    int_variable_names += ["nLargeJets", "nLeptons",
                           "nCentralJets", "nForwardJets"]

    if flags.Analysis.do_resolved:
        float_variable_names += ["dRbl_min", "HT2", "HT2r"]
        int_variable_names += ["nJets", "nBJets"]

    return float_variable_names, int_variable_names


def fullLep_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsFullLepAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to HHbbtt
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsFullLepAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> {var}_%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, \
        object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "FullLep")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    cutList = flags.Analysis.CutList
    for cut in cutList:
        branches += [f"EventInfo.{cut}_%SYS% -> {cut}_%SYS%"]

    # truth info
    if flags.Input.isMC and flags.Analysis.AddTruthVBSQuarks:
        branches += ['EventInfo.nVBSQuarks -> nVBSQuarks']
        for var in ['pT', 'eta', 'phi', 'E']:
            branches += [f'EventInfo.VBSQuark1_{var} -> VBSQuark1_{var}']
            branches += [f'EventInfo.VBSQuark2_{var} -> VBSQuark2_{var}']

    # VBF tagger
    if flags.Analysis.UseVBFRNN:
        vars = ['RNNScore', 'nRNNJets']
        regs = ['boostedCategory']
        if flags.Analysis.do_resolved:
            regs += ['resolvedCategory']
        for var in vars:
            for reg in regs:
                branches += [f'EventInfo.{var}_{reg}_%SYS% -> {var}_{reg}_%SYS%']

    if flags.Analysis.save_high_level_variables:
        for cat in ["ele0_passSET", "ele1_passSET", "mu0_passSMT",
                    "mu1_passSMT", "ele0_trigPassed", "ele1_trigPassed",
                    "mu0_trigPassed", "mu1_trigPassed",
                    "ele0_trigMatched", "ele1_trigMatched",
                    "mu0_trigMatched", "mu1_trigMatched"]:
            branches += [f"EventInfo.{cat}_%SYS% -> {cat}_%SYS%"]
        if flags.Input.isMC and flags.Analysis.do_trigsf:
            for cat in ["ele0_trigEffSF", "ele1_trigEffSF",
                        "mu0_trigEffSF", "mu1_trigEffSF",
                        "event_trigEffSF"]:
                branches += [f"EventInfo.{cat}_%SYS% -> {cat}_%SYS%"]

    return branches, float_variable_names, int_variable_names
