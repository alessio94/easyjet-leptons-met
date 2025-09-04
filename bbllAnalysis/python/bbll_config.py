# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
import AthenaCommon.SystemOfUnits as Units
from itertools import chain

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_lepton_branches_variables,
    get_selected_jet_b_tagged_branches_variables,

)


def bbll_cfg(flags, smalljetkey, muonkey, electronkey,
             float_variables=None, int_variables=None, float_NW_variables=None,
             float_PNN_variables=None):
    keys = ["baseline", "jets", "leptons"]
    if not float_variables:
        float_variables = {key: [] for key in keys}
    if not int_variables:
        int_variables = {key: [] for key in keys}
    if not float_NW_variables:
        float_NW_variables = []
    if not float_PNN_variables:
        float_PNN_variables = []

    cfg = ComponentAccumulator()

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
    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.HHBBLL.LeptonVarsbbllAlg(
            "LeptonVarsbbllAlg",
            isMC=flags.Input.isMC,
            electrons=electronkey, eleWP=ElectronWPLabel,
            muons=muonkey, muonWP=MuonWPLabel,
            floatVariableList=float_variables['leptons'],
            intVariableList=int_variables['leptons'],
            save_extra_vars=flags.Analysis.save_extra_vars,
            doSystematics=flags.Analysis.do_CP_systematics,
        )
    )
    btag_pcbt_wps \
        = [wp for wp in flags.Analysis.Small_R_jet.btag_extra_wps if "Continuous" in wp]

    cfg.addEventAlgo(
        CompFactory.HHBBLL.BJetsVarsbbllAlg(
            "BJetsVarsbbllAlg",
            isMC=flags.Input.isMC,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            PCBTDecorList=["ftag_quantile_" + pcbt_wp for pcbt_wp in btag_pcbt_wps],
            floatVariableList=float_variables['jets'],
            intVariableList=int_variables['jets'],
            save_extra_vars=flags.Analysis.save_extra_vars,
            doSystematics=flags.Analysis.do_CP_systematics,
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
                save_extra_vars=flags.Analysis.save_extra_vars,
                NW_cutList=flags.Analysis.NeutrinoWeighting.cutList,
                NeutrinoWeightingTools=[
                    NeutrinoWeightingTool_1,
                    NeutrinoWeightingTool_2],
            )
        )

    # calculate final bbll vars
    cfg.addEventAlgo(
        CompFactory.HHBBLL.BaselineVarsbbllAlg(
            "FinalVarsbbllAlg",
            isMC=flags.Input.isMC,
            floatVariableList=float_variables['baseline'],
            intVariableList=int_variables['baseline'],
            save_extra_vars=flags.Analysis.save_extra_vars
        )
    )
    if flags.Analysis.do_resonant_PNN:
        cfg.addEventAlgo(
            CompFactory.HHBBLL.ResonantPNNbbllAlg(
                "ResonantPNNbbllAlg",
                bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
                floatPNNVariables=float_PNN_variables,
                mX_values=flags.Analysis.mX_values,
                run_Run2_train=flags.GeoModel.Run is LHCPeriod.Run2
            )
        )

    return cfg


def get_bjetsVarsbbllAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    # bjet variables
    int_variable_names = ["nBJets"]
    float_variable_names += ["mbb", "pTbb", "dRbb", "mT2_bb"]
    if (flags.Analysis.save_extra_vars):
        int_variable_names += ["nJets", "nCentralJets"]
        float_variable_names += ["Phibb"]

    return float_variable_names, int_variable_names


def get_LeptonVarsbbllAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    # Lepton variables
    int_variable_names = ["nLeptons"]
    float_variable_names += ["mll", "pTll", "dRll"]
    if (flags.Analysis.save_extra_vars):
        int_variable_names += ["nElectrons",
                               "nMuons"]
        float_variable_names += ["Phill"]

    return float_variable_names, int_variable_names


def get_BaselineVarsbbllAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    float_variable_names += ["mbbll", "mbbllmet", "mbl", "MET_sig"]
    if (flags.Analysis.save_extra_vars):
        float_variable_names += ["mT_L_min", "mT_Lepton1_Met",
                                 "mT_Lepton2_Met", "HT2", "HT2r"]
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
    keys = ["baseline", "leptons", "jets"]
    float_variable_names = {key: [] for key in keys}
    int_variable_names = {key: [] for key in keys}

    # these are the variables that will always be stored by easyjet specific to HHbbtt
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsbbllAlg_variables(flags)
    lepton_float_variables, lepton_int_variables \
        = get_LeptonVarsbbllAlg_variables(flags)
    bjet_float_variables, bjet_int_variables \
        = get_bjetsVarsbbllAlg_variables(flags)

    float_variable_names['baseline'] += baseline_float_variables
    int_variable_names['baseline'] += baseline_int_variables
    float_variable_names['leptons'] += lepton_float_variables
    int_variable_names['leptons'] += lepton_int_variables
    float_variable_names['jets'] += bjet_float_variables
    int_variable_names['jets'] += bjet_int_variables

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
        if (flags.Analysis.save_extra_vars):
            all_baseline_variable_names.append("NW_solutions")
        for var in ["neutrinoweight"]:
            float_NW_variable_names.append(f"NW_{var}")
    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsbbllAlg_highlevelvariables(flags)
        float_variable_names['baseline'] += high_level_float_variables
        int_variable_names['baseline'] += high_level_int_variables
    if flags.Analysis.do_resonant_PNN:
        for m_X in flags.Analysis.mX_values:
            float_PNN_variable_names += [f"PNN_Score_X{m_X}"]
            float_PNN_variable_names += [f"PNN_Score_SR2_X{m_X}"]
        float_PNN_variable_names += ["DNN_Score_SR1"]
    all_baseline_variable_names += [
        *chain.from_iterable(float_variable_names.values()),
        *chain.from_iterable(int_variable_names.values()),
        *float_NW_variable_names,
        *float_PNN_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> bbll_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches = {key: [] for key in keys}
    object_level_float_variables = {key: [] for key in keys}
    object_level_int_variables = {key: [] for key in keys}

    (object_level_branches['leptons'],
     object_level_float_variables['leptons'],
     object_level_int_variables['leptons']) = \
        get_selected_lepton_branches_variables(flags, "bbll")

    float_variable_names['leptons'] += object_level_float_variables['leptons']
    int_variable_names['leptons'] += object_level_int_variables['leptons']

    (object_level_branches['jets'],
     object_level_float_variables['jets'],
     object_level_int_variables['jets']) = \
        get_selected_jet_b_tagged_branches_variables(flags, "bbll")

    float_variable_names['jets'] += object_level_float_variables['jets']
    int_variable_names['jets'] += object_level_int_variables["jets"]

    branches += object_level_branches["leptons"]
    branches += object_level_branches["jets"]

    if (flags.Analysis.save_cutflow):
        cutList = flags.Analysis.CutList + flags.Analysis.Categories
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> bbll_{cut}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # trigger variables do not need to be added to variable_names
    # as it is written out in HHbbllSelectorAlg
    if flags.Analysis.store_high_level_variables:
        branches += ["EventInfo.bbll_pass_sr_%SYS% -> bbll_pass_SR"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]
        for cat in ["SLT", "DLT", "ASLT1_em", "ASLT1_me", "ASLT2"]:
            branches += \
                [f"EventInfo.pass_trigger_{cat}_%SYS% -> bbll_pass_trigger_{cat}"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return (branches, float_variable_names, int_variable_names, float_NW_variable_names,
            float_PNN_variable_names)
