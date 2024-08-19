from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def ZCharm_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey,
               float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="ZCharmAnalysisMuons_%SYS%",
                                 looseMuonWP=MuonWPLabel,
                                 minPt=flags.Analysis.Muon.min_pT_ZCharm,
                                 maxEta=flags.Analysis.Muon.max_eta_ZCharm))

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="ZCharmAnalysisElectrons_%SYS%",
                                     looseEleWP=ElectronWPLabel,
                                     isMC=flags.Input.isMC,
                                     minPt=flags.Analysis.Electron.min_pT_ZCharm,
                                     maxEta=flags.Analysis.Electron.max_eta_ZCharm))

    cfg.merge(JetSelectorAlgCfg(flags, name="SmallRJet_SelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="ZCharmAnalysisJets_%SYS%",
                                bTagWPDecorName="",
                                minPt=flags.Analysis.small_R_jet.min_pT_ZCharm,
                                maxEta=flags.Analysis.small_R_jet.max_eta_ZCharm))

    cfg.merge(JetSelectorAlgCfg(flags, name="LargeRJet_SelectorAlg",
                                containerInKey=largejetkey,
                                containerOutKey="ZCharmAnalysisLargeJets_%SYS%",
                                minPt=250 * Units.GeV,
                                maxEta=2.5,
                                selectBjet=False))

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    # Selection
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    # Selection
    cfg.addEventAlgo(
        CompFactory.ZCC.ZCharmSelectorAlg(
            "ZCharmSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            eventDecisionOutputDecoration="ZCharm_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_ZCharm_cutflow,
            isMC=flags.Input.isMC,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # calculate final ZCharm vars
    cfg.addEventAlgo(
        CompFactory.ZCC.BaselineVarsZCharmAlg(
            "FinalVarsZCharmAlg",
            isMC=flags.Input.isMC,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    return cfg


def get_BaselineVarsZCharmAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for object in ["ll", "jj", "bb"]:
        for var in ["m", "pT", "Eta", "Phi", "dR", "dEta", "dPhi"]:
            float_variable_names.append(f"{var}{object}")

    float_variable_names += ["METSig"]
    int_variable_names += ["nJets", "nBJets", "nLargeRJets"]
    int_variable_names += ["nElectrons", "nMuons", "nLeptons"]

    return float_variable_names, int_variable_names


def get_BaselineVarsZCharmAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def ZCharm_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsZCharmAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to ZCharm
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsZCharmAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsZCharmAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> ZCharm_{var}_%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "ZCharm")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    branches += ["EventInfo.ZCharm_pass_sr_%SYS% -> ZCharm_pass_SR_%SYS%"]

    if (flags.Analysis.save_ZCharm_cutflow):
        cutList = flags.Analysis.CutList + flags.Analysis.Categories
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> ZCharm_{cut}_%SYS%"]

    # trigger variables do not need to be added to variable_names
    # as it is written out in ZCharmSelectorAlg
    for cat in ["SLT", "DLT"]:
        branches += \
            [f"EventInfo.pass_trigger_{cat}_%SYS% -> ZCharm_pass_trigger_{cat}_%SYS%"]

    return branches, float_variable_names, int_variable_names
