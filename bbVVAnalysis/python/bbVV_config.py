from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, JetSelectorAlgCfg, LeptonOrderingAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def bbVV_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey,
             float_variables=None, int_variables=None
             ):
    cfg = ComponentAccumulator()

    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="bbVVAnalysisMuons_%SYS%"))

    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="bbVVAnalysisElectrons_%SYS%"))

    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(JetSelectorAlgCfg(flags, name="SmallJetSelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="bbVVAnalysisJets_%SYS%",
                                bTagWPDecorName="",  # empty string: "" ignores btagging
                                selectBjet=False,
                                maxEta=2.5,
                                truncateAtAmount=2,  # -1 means keep all
                                minimumAmount=2))  # -1 means ignores this

    cfg.merge(JetSelectorAlgCfg(flags, name="LargeJetSelectorAlg",
                                containerInKey=largejetkey,
                                containerOutKey="bbVVAnalysisLRJets_%SYS%",
                                bTagWPDecorName="",
                                selectBjet=False,
                                minPt=200e3,
                                maxEta=2.0,
                                truncateAtAmount=-1,  # Keep all lrjets
                                minimumAmount=flags.Analysis.Large_R_jet.amount))

    # Selection
    TightMuonWP = flags.Analysis.Muon.extra_wps[0]
    TightMuonWPLabel = f'{TightMuonWP[0]}_{TightMuonWP[1]}'
    TightEleWP = flags.Analysis.Electron.extra_wps[0]
    TightEleWPLabel = f'{TightEleWP[0]}_{TightEleWP[1]}'

    cfg.addEventAlgo(
        CompFactory.HHBBVV.HHbbVVSelectorAlg(
            "HHbbVVSelectorAlg",
            eventDecisionOutputDecoration="bbVV_pass_sr_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            muonWP=TightMuonWPLabel,
            eleWP=TightEleWPLabel,
            channel=flags.Analysis.channel,
            cutList=(flags.Analysis.CutList
                     if hasattr(flags.Analysis, "CutList") else []),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
            GN2X_WPs=flags.Analysis.Large_R_jet.GN2X_hbb_wps
        )
    )

    # calculate final bbVV vars
    cfg.addEventAlgo(
        CompFactory.HHBBVV.BaselineVarsbbVVAlg(
            "BaselineVarsbbVVAlg",
            isMC=flags.Input.isMC,
            electrons=electronkey, eleWP=TightEleWPLabel,
            muons=muonkey, muonWP=TightMuonWPLabel,
            floatVariableList=float_variables,
            intVariableList=int_variables,
            channel=flags.Analysis.channel,
            GN2X_WPs=flags.Analysis.Large_R_jet.GN2X_hbb_wps,
            wtag_type=flags.Analysis.Large_R_jet.wtag_type,
            wtag_wp=flags.Analysis.Large_R_jet.wtag_wp
        )
    )

    return cfg


def get_BaselineVarshhbbVVAlg_variables(flags):

    int_variable_names = ["lrjets_n", "srjets_n", "Selected_Lepton_n"]
    float_variable_names = ["Whad_Jet_DeltaR"]

    objects = ["Hbb", "Whad"]
    if "SplitBoosted0Lep" in flags.Analysis.channel:
        objects += ["Whad2"]

    wtag_type = flags.Analysis.Large_R_jet.wtag_type
    wtag_wp = flags.Analysis.Large_R_jet.wtag_wp

    for obj in objects:
        for var in ["pt", "eta", "phi", "m"]:
            float_variable_names += [obj + "_Jet_" + var]
        for var in ["phbb", "pqcd", "phcc", "ptop"]:
            float_variable_names += [obj + "_Jet_GN2Xv01_" + var]
        float_variable_names += [obj + "_Jet_" + wtag_type
                                 + "_" + wtag_wp + "_Score"]

    for obj in objects:
        for var in flags.Analysis.Large_R_jet.GN2X_hbb_wps:
            int_variable_names += [obj + "_Jet_Pass_GN2X_" + var]
        int_variable_names += [obj + "_Jet_Pass_" + wtag_type
                                   + "_" + wtag_wp]

    if "Boosted0Lep" in flags.Analysis.channel:
        for i in range(1, 5):
            float_variable_names += [f"Whad_Jet_Tau{i}_wta"]
        for i in range(1, 4):
            float_variable_names += [f"Whad_Jet_ECF{i}"]

    return float_variable_names, int_variable_names


def bbVV_branches(flags):
    branches = []

    float_variable_names = []
    int_variable_names = []
    all_baseline_variable_names = []

    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarshhbbVVAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    all_baseline_variable_names += [
        *float_variable_names,
        *int_variable_names
    ]

    for var in all_baseline_variable_names:
        branches += [
            f"EventInfo.{var}_%SYS% -> bbVV_{var}"
            + flags.Analysis.systematics_suffix_separator + "%SYS%"
        ]

    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "bbVV")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    # cuts in ttree
    branches += ["EventInfo.PassAllCuts_%SYS% -> PassAllCuts"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches, float_variable_names, int_variable_names
