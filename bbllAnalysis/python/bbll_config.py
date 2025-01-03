from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    JetSelectorAlgCfg, PhotonSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def bbll_cfg(flags, smalljetkey, muonkey, electronkey, photonkey,
             float_variables=None, int_variables=None, float_NW_variables=None,
             float_PNN_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []
    if not float_NW_variables:
        float_NW_variables = []
    if not float_PNN_variables:
        float_PNN_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(PhotonSelectorAlgCfg(flags,
                                   containerInKey=photonkey,
                                   containerOutKey="bbllAnalysisPhotons_%SYS%",
                                   minPt=20e3))

    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="bbllAnalysisMuons_%SYS%",
                                 minPt=9 * Units.GeV))

    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="bbllAnalysisElectrons_%SYS%",
                                     minPt=9 * Units.GeV))

    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(JetSelectorAlgCfg(flags,
                                containerInKey=smalljetkey,
                                containerOutKey="bbllAnalysisJets_%SYS%",
                                bTagWPDecorName="",
                                selectBjet=False,
                                minPt=20 * Units.GeV,
                                minimumAmount=2))  # -1 means ignores this

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    # Selection
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    # Selection
    cfg.addEventAlgo(
        CompFactory.HHBBLL.HHbbllSelectorAlg(
            "HHbbllSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            eventDecisionOutputDecoration="bbll_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_cutflow,
            isMC=flags.Input.isMC,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # MMC decoration
    if flags.Analysis.do_mmc:
        from EasyjetHub.algs.mmc_tool_config import MissingMassToolCfg
        cfg.addEventAlgo(CompFactory.HHBBLL.MMCDecoratorAlg(
            mmcTool=cfg.popToolsAndMerge(MissingMassToolCfg(flags))))

    # NeutrinoWeighting
    if flags.Analysis.NeutrinoWeighting.doNW:
        NeutrinoWeightingTool_1 = CompFactory.HHBBLL.NeutrinoWeightingTool(
            "NeutrinoWeightingTool_1",
            resolution_settings=flags.Analysis.NeutrinoWeighting.resolution_settings,
            resolution_number=flags.Analysis.NeutrinoWeighting.resolution_number)
        NeutrinoWeightingTool_2 = CompFactory.HHBBLL.NeutrinoWeightingTool(
            "NeutrinoWeightingTool_2",
            resolution_settings=flags.Analysis.NeutrinoWeighting.resolution_settings,
            resolution_number=flags.Analysis.NeutrinoWeighting.resolution_number)
        cfg.addEventAlgo(
            CompFactory.HHBBLL.NeutrinoWeightingAlg(
                "NeutrinoWeightingAlg",
                bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
                floatNWVariables=float_NW_variables,
                NW_cutList=flags.Analysis.NeutrinoWeighting.cutList,
                NeutrinoWeightingTools=[
                    NeutrinoWeightingTool_1,
                    NeutrinoWeightingTool_2],
            )
        )

    btag_pcbt_wps \
        = [wp for wp in flags.Analysis.Small_R_jet.btag_extra_wps if "Continuous" in wp]

    # calculate final bbll vars
    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.HHBBLL.BaselineVarsbbllAlg(
            "FinalVarsbbllAlg",
            isMC=flags.Input.isMC,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            PCBTDecorList=["ftag_quantile_" + pcbt_wp for pcbt_wp in btag_pcbt_wps],
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )
    if flags.Analysis.do_resonant_PNN:
        cfg.addEventAlgo(
            CompFactory.HHBBLL.ResonantPNNbbllAlg(
                "ResonantPNNbbllAlg",
                bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
                floatPNNVariables=float_PNN_variables,
                mX_values=flags.Analysis.mX_values,
            )
        )

    return cfg


def get_BaselineVarsbbllAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for object in ["ll", "bb"]:
        for var in ["m", "pT", "dR", "Eta", "Phi"]:
            float_variable_names.append(f"{var}{object}")

    float_variable_names += ["met_x", "met_y", "mbbll",
                             "mbbllmet", "MET_sig", "mT_Lepton1_Met", "mT_Lepton2_Met",
                             "mT_L_min", "dRbl_min", "HT2", "HT2r", "mT2_bb", "mbl"]

    int_variable_names += ["nJets", "nBJets", "nElectrons", "nMuons", "nCentralJets"]

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
        # mmc algortithm not BaselineVarsbbllAlg
        for var in ["status", "pt", "eta", "phi", "m"]:
            all_baseline_variable_names.append(f"mmc_{var}")

    float_NW_variable_names = []
    float_PNN_variable_names = []
    if flags.Analysis.NeutrinoWeighting.doNW:
        # do not append TopReco variables to float_variable_names
        # or int_variable_names as they are stored by the
        # TopReco algortithm not BaselineVarsbbllAlg
        all_baseline_variable_names.append("NW_solutions")
        for var in ["neutrinoweight"]:
            float_NW_variable_names.append(f"NW_{var}")
    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsbbllAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables
    if flags.Analysis.do_resonant_PNN:
        for m_X in flags.Analysis.mX_values:
            float_PNN_variable_names.append(f"PNN_Score_X{m_X}")
    all_baseline_variable_names += [
        *float_variable_names,
        *int_variable_names,
        *float_NW_variable_names,
        *float_PNN_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> bbll_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "bbll")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    branches += ["EventInfo.bbll_pass_sr_%SYS% -> bbll_pass_SR"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    if (flags.Analysis.save_cutflow):
        cutList = flags.Analysis.CutList + flags.Analysis.Categories
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> bbll_{cut}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # trigger variables do not need to be added to variable_names
    # as it is written out in HHbbllSelectorAlg
    for cat in ["SLT", "DLT", "ASLT1_em", "ASLT1_me", "ASLT2"]:
        branches += \
            [f"EventInfo.pass_trigger_{cat}_%SYS% -> bbll_pass_trigger_{cat}"
             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return (branches, float_variable_names, int_variable_names, float_NW_variable_names,
            float_PNN_variable_names)
