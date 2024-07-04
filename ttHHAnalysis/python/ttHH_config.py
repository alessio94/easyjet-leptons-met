from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)

import pathlib
import os


def ttHH_cfg(flags, smalljetkey, muonkey, electronkey,
             float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    LooseMuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    TightMuonWP = flags.Analysis.Muon.extra_wps[0]
    TightMuonWPLabel = f'{TightMuonWP[0]}_{TightMuonWP[1]}'
    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=LooseMuonWPLabel + muonkey,
                                 containerOutKey="ttHHAnalysisMuons_%SYS%",
                                 minPt=10e3,
                                 muon_WPs=[TightMuonWPLabel]))

    LooseElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    TightEleWP = flags.Analysis.Electron.extra_wps[0]
    TightEleWPLabel = f'{TightEleWP[0]}_{TightEleWP[1]}'
    cfg.merge(ElectronSelectorAlgCfg(
        flags,
        containerInKey=LooseElectronWPLabel + electronkey,
        containerOutKey="ttHHAnalysisElectrons_%SYS%",
        minPt=10e3,
        ele_WPs=[TightEleWPLabel]))

    cfg.merge(JetSelectorAlgCfg(flags, name="SmallRJet_BTag_SelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="ttHHAnalysisJets_BTag_%SYS%",
                                bTagWPDecorName="ftag_select_"
                                + flags.Analysis.small_R_jet.btag_wp,
                                selectBjet=True,
                                maxEta=2.5,
                                minPt=20e3))

    cfg.merge(JetSelectorAlgCfg(flags, name="SmallRJet_SelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="ttHHAnalysisJets_%SYS%",
                                PCBTDecorName="ftag_quantile_"
                                + flags.Analysis.small_R_jet.btag_extra_wps[0],
                                PCBTsort=True,
                                pTsort=False,
                                bTagWPDecorName="",
                                selectBjet=False,
                                minPt=20e3))

    cfg.addEventAlgo(
        CompFactory.ttHH.JetPairingAlgttHH(
            "JetPairingAlgHH",
            bTagWPDecorName="ftag_select_"
            + flags.Analysis.small_R_jet.btag_wp,
            pairingStrategyName="chiSquare",
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.TriggerDecoratorAlg(
            "ttHHTriggerDecoratorAlg",
            isMC=flags.Input.isMC,
            triggerLists=flags.Analysis.TriggerChains,
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.ttHHSelectorAlg(
            "ttHHSelectorAlg",
            muonWP=TightMuonWPLabel,
            eleWP=TightEleWPLabel,
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_ttHH_cutflow,
            triggerLists=flags.Analysis.TriggerChains,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            eventDecisionOutputDecoration="ttHH_pass_baseline_%SYS%",
            bypass=flags.Analysis.bypass,
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.BaselineVarsttHHAlg(
            "BaselineVarsttHHAlg",
            bTagWPDecorName="ftag_select_"
            + flags.Analysis.small_R_jet.btag_wp,
            PCBTDecorName="ftag_quantile_"
            + flags.Analysis.small_R_jet.btag_extra_wps[0],
            muonWP=TightMuonWPLabel,
            eleWP=TightEleWPLabel,
            isMC=flags.Input.isMC,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    return cfg


def get_BaselineVarsttHHAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for object in ["ll"]:
        for var in ["m", "pt", "dR", "eta", "phi"]:
            float_variable_names.append(f"{object}_{var}")

    H_candidate_variables = [
        "H1_m", "H1_pt", "H1_eta", "H1_phi",
        "H2_m", "H2_pt", "H2_eta", "H2_phi",
        "HH_m", "HH_CHI",
    ]

    float_variable_names += H_candidate_variables

    angular_variables = [
        "Jets_DeltaR12", "Jets_DeltaR34", "Jets_DeltaR56",
        "Jets_DeltaEta12", "Jets_DeltaEta34", "Jets_DeltaEta56",
        "Jets_DeltaRMax", "Jets_DeltaRMin", "Jets_DeltaRMean",
        "Jets_DeltaEtaMax", "Jets_DeltaEtaMin", "Jets_DeltaEtaMean"
    ]

    float_variable_names += angular_variables

    float_variable_names += ["HT", "HTall"]

    int_variable_names += [
        "nJets", "nBJets", "nLeptons", "sumPCBT",
        "dilept_type", "trilept_type", "total_charge",
    ]

    return float_variable_names, int_variable_names


def get_BaselineVarsttHHAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def ttHH_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsttHHAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to ttHH
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsttHHAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsttHHAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> ttHH_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "ttHH")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    branches += ["EventInfo.PassAllCuts_%SYS% -> ttHH_PassAllCuts"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    branches += ["EventInfo.ttHH_pass_baseline_%SYS% -> ttHH_pass_baseline"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    for trig in ["dilep", "singlep"]:
        branches += [f"EventInfo.ttHH_pass_trigger_{trig} \
                             -> ttHH_pass_trigger_{trig}"]
    for trig in ["dilep", "singlep"]:
        branches += [f"EventInfo.pass_matching_trigger_{trig}_%SYS% \
                             -> ttHH_pass_matching_trigger_{trig}_%SYS%"]

    if (flags.Analysis.save_ttHH_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> ttHH_{cut}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches, float_variable_names, int_variable_names


def FullPath(rawpath):
    fpath = pathlib.Path(rawpath)
    for dirpath in [""] + os.environ["DATAPATH"].split(":"):
        fullpath = dirpath / fpath
        if fullpath.exists():
            return fullpath
