from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def bbll_cfg(flags, smalljetkey, muonkey, electronkey,
             float_variables=[], int_variables=[]):

    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=MuonWPLabel + muonkey,
            containerOutKey="bbllAnalysisMuons_%SYS%",
            muonSF_WP=MuonWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
            minPt=9 * Units.GeV,
        )
    )

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=ElectronWPLabel + electronkey,
            containerOutKey="bbllAnalysisElectrons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
            eleSF_WP=ElectronWPLabel,
            isMC=flags.Input.isMC,
            minPt=9 * Units.GeV,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbllAnalysisJets_%SYS%",
            bTagWPDecorName="",
            minPt=20 * Units.GeV,
            minimumAmount=2,  # -1 means ignores this
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    # Selection
    cfg.addEventAlgo(
        CompFactory.HHBBLL.HHbbllSelectorAlg(
            "HHbbllSelectorAlg",
            jets="bbllAnalysisJets_%SYS%",
            muons="bbllAnalysisMuons_%SYS%",
            electrons="bbllAnalysisElectrons_%SYS%",
            met="AnalysisMET_%SYS%",
            eventDecisionOutputDecoration="bbll_pass_sr_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
        )
    )
    # MMC decoration
    if flags.Analysis.do_mmc:
        cfg.addEventAlgo(
            CompFactory.HHBBLL.MMCDecoratorAlg(
                "MMCDecoratorAlg",
                jets="bbllAnalysisJets_%SYS%",
                muons="bbllAnalysisMuons_%SYS%",
                electrons="bbllAnalysisElectrons_%SYS%",
                met="AnalysisMET_%SYS%",
            )
        )

    # calculate final bbll vars
    cfg.addEventAlgo(
        CompFactory.HHBBLL.BaselineVarsbbllAlg(
            "FinalVarsbbllAlg",
            isMC=flags.Input.isMC,
            jets="bbllAnalysisJets_%SYS%",
            muons="bbllAnalysisMuons_%SYS%",
            electrons="bbllAnalysisElectrons_%SYS%",
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBLL.SelectionFlagsbbllAlg(
            "SelectionFlagsbbllAlg",
            jets="bbllAnalysisJets_%SYS%",
            muons="bbllAnalysisMuons_%SYS%",
            electrons="bbllAnalysisElectrons_%SYS%",
            cutList=flags.Analysis.CutList,
            categoryList=flags.Analysis.Categories,
            saveCutFlow=flags.Analysis.save_bbll_cutflow,
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            passTriggers=flags.Analysis.TriggerChains
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.EventInfoGlobalAlg(
            isMC=flags.Input.isMC,
            Years=flags.Analysis.Years,
        )
    )

    return cfg


def get_BaselineVarsbbllAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for object in ["ee", "mumu", "emu", "bb"]:
        for var in ["m", "pT", "dR", "Eta", "Phi"]:
            float_variable_names.append(f"{var}{object}")

    float_variable_names += ["mll", "pTll"]

    int_variable_names += ["nJets", "nBJets", "nElectrons", "nMuons", "nCentralJets",
                           "Jet_b1_truthLabel", "Jet_b2_truthLabel"]

    return float_variable_names, int_variable_names


def get_BaselineVarsbbllAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def bbll_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsbbllAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to HHbbtt
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsbbllAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    if flags.Analysis.do_mmc:
        # do not append mmc variables to float_variable_names
        # or int_variable_names as they are stored by the
        # mmc algortithm not BaselineVarsbbttAlg
        for var in ["status", "pt", "eta", "phi", "m"]:
            all_baseline_variable_names.append(f"mmc_{var}")

    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsbbllAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for tree_flags in flags.Analysis.ttree_output:
        for var in all_baseline_variable_names:
            if tree_flags['slim_variables_with_syst'] and \
               "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbll_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbll_{var}_%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "bbll")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    # BJets
    if flags.Input.isMC:
        branches += ["EventInfo.ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%"
                     " -> weight_ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%",]

        branches += ["EventInfo.jvt_effSF_%SYS% -> weight_jvt_effSF_%SYS%"]

    branches += ["EventInfo.bbll_pass_sr_%SYS% -> bbll_pass_SR_%SYS%"]

    if (flags.Analysis.save_bbll_cutflow):
        cutList = flags.Analysis.CutList + flags.Analysis.Categories
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> bbll_{cut}_%SYS%"]

    branches += ["EventInfo.dataTakingYear -> dataTakingYear"]

    return branches, float_variable_names, int_variable_names
