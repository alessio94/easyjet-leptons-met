from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def fullLep_cfg(flags, float_variables=[], int_variables=[]):

    cfg = ComponentAccumulator()

    # Selection
    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.FullLepSelectorAlg(
            "FullLepSelectorAlg",
            jets="vbshiggsAnalysisJets_%SYS%",
            muons="vbshiggsAnalysisMuons_%SYS%",
            electrons="vbshiggsAnalysisElectrons_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            eventDecisionOutputDecoration="vbshiggs_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_vbshiggs_cutflow,
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )
    # calculate final vbshiggs vars
    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.BaselineVarsFullLepAlg(
            "FinalVarsFullLepAlg",
            isMC=flags.Input.isMC,
            jets="vbshiggsAnalysisJets_%SYS%",
            muons="vbshiggsAnalysisMuons_%SYS%",
            electrons="vbshiggsAnalysisElectrons_%SYS%",
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )
    return cfg


def get_BaselineVarsFullLepAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for object in ["ll", "bb", "b1l1", "b2l2", "jj"]:
        for var in ["m", "pT", "Eta", "Phi", "dR", "dEta", "dPhi"]:
            float_variable_names.append(f"{var}{object}")

    float_variable_names += ["dPhillMET", "dPhil1MET", "dPhil2MET", "METSig"]

    int_variable_names += ["nJets", "nBJets", "nElectrons", "nMuons",
                           "nLeptons", "nCentralJets", "nForwardJets"]

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

    for tree_flags in flags.Analysis.ttree_output:
        for var in all_baseline_variable_names:
            branches += [f"EventInfo.{var}_%SYS% -> FullLep_{var}_%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "FullLep")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches
    branches += ["EventInfo.vbshiggs_pass_sr_%SYS% -> FullLep_pass_SR_%SYS%"]

    if (flags.Analysis.save_vbshiggs_cutflow):
        cutList = flags.Analysis.CutList + flags.Analysis.Categories
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> FullLep_{cut}_%SYS%"]
    return branches, float_variable_names, int_variable_names
