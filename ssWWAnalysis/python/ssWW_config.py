from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def ssWW_cfg(flags, smalljetkey, muonkey, electronkey,
             float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="ssWWAnalysisMuons_%SYS%",
                                 looseMuonWP=MuonWPLabel,
                                 minPt=flags.Analysis.Muon.min_pT_ssWW,
                                 maxEta=flags.Analysis.Muon.max_eta_ssWW))

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="ssWWAnalysisElectrons_%SYS%",
                                     looseEleWP=ElectronWPLabel,
                                     isMC=flags.Input.isMC,
                                     minPt=flags.Analysis.Electron.min_pT_ssWW))

    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(JetSelectorAlgCfg(flags,
                                containerInKey=smalljetkey,
                                containerOutKey="ssWWAnalysisJets_%SYS%",
                                bTagWPDecorName="",
                                minPt=flags.Analysis.Small_R_jet.min_pT_ssWW,
                                maxEta=flags.Analysis.Small_R_jet.max_eta_ssWW))

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    # Selection
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    # Selection
    cfg.addEventAlgo(
        CompFactory.ssWWVBS.ssWWSelectorAlg(
            "ssWWSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            eventDecisionOutputDecoration="ssWW_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_ssWW_cutflow,
            isMC=flags.Input.isMC,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # calculate final ssWW vars
    cfg.addEventAlgo(
        CompFactory.ssWWVBS.BaselineVarsssWWAlg(
            "FinalVarsssWWAlg",
            isMC=flags.Input.isMC,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    return cfg


def get_BaselineVarsssWWAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for object in ["ll", "jj", "jj_vbs"]:
        for var in ["m", "pT", "Eta", "Phi", "dR", "dEta", "dPhi"]:
            float_variable_names.append(f"{var}{object}")

    float_variable_names += ["dPhillMET", "dPhil1MET", "dPhil2MET"]
    float_variable_names += ["mlljjmet", "METSig", "mT_Lepton1_Met", "mT_Lepton2_Met"]
    float_variable_names += ["mlljj", "mT_L_min", "HT2", "HT2r", "mT2_jj"]
    float_variable_names += ["MET_met", "MET_phi"]
    float_variable_names += ["mT", "epsilon_j3"]
    int_variable_names += ["nJets", "nBJets", "nCentralJets", "nForwardJets"]
    int_variable_names += ["nElectrons", "nMuons", "nLeptons", "nGapJets"]

    return float_variable_names, int_variable_names


def get_BaselineVarsssWWAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def ssWW_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsssWWAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to ssWW
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsssWWAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsssWWAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> ssWW_{var}_%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "ssWW")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    branches += ["EventInfo.ssWW_pass_sr_%SYS% -> ssWW_pass_SR_%SYS%"]

    if (flags.Analysis.save_ssWW_cutflow):
        cutList = flags.Analysis.CutList + flags.Analysis.Categories
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> ssWW_{cut}_%SYS%"]

    # trigger variables do not need to be added to variable_names
    # as it is written out in ssWWSelectorAlg
    for cat in ["SLT", "DLT", "ASLT1_em", "ASLT1_me", "ASLT2"]:
        branches += \
            [f"EventInfo.pass_trigger_{cat}_%SYS% -> ssWW_pass_trigger_{cat}_%SYS%"]

    return branches, float_variable_names, int_variable_names
