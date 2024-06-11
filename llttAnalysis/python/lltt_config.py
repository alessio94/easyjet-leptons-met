from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def lltt_cfg(
        flags, smalljetkey, muonkey, electronkey,
        taukey, float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    LooseMuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    TightMuonWP = flags.Analysis.Muon.extra_wps[0]
    TightMuonWPLabel = f'{TightMuonWP[0]}_{TightMuonWP[1]}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=LooseMuonWPLabel + muonkey,
            containerOutKey="llttAnalysisMuons_%SYS%",
            muon_WP=TightMuonWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    LooseElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    TightElectronWP = flags.Analysis.Electron.extra_wps[0]
    TightElectronWPLabel = f'{TightElectronWP[0]}_{TightElectronWP[1]}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=LooseElectronWPLabel + electronkey,
            containerOutKey="llttAnalysisElectrons_%SYS%",
            ele_WP=TightElectronWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.TauSelectorAlg(
            "TauSelectorAlg",
            containerInKey=flags.Analysis.Tau.ID + taukey,
            containerOutKey="llttAnalysisTaus_%SYS%",
            tau_WP=flags.Analysis.Tau.ID,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="llttAnalysisJets_%SYS%",
            minPt=25 * Units.GeV,
            maxEta=2.5,
            bTagWPDecorName="",  # empty string: "" ignores btagging
            selectBjet=False,
            minimumAmount=-1,  # -1 means ignores this
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    # Selection
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    cfg.addEventAlgo(
        CompFactory.HLLTT.HllttSelectorAlg(
            "HllttSelectorAlg",
            jets="llttAnalysisJets_%SYS%",
            muons="llttAnalysisMuons_%SYS%",
            electrons="llttAnalysisElectrons_%SYS%",
            taus="llttAnalysisTaus_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            tauWP=flags.Analysis.Tau.ID,
            muonWP=TightMuonWPLabel,
            eleWP=TightElectronWPLabel,
            eventDecisionOutputDecoration="lltt_pass_sr_noMMC_%SYS%",
            triggerLists=trigger_branches,
            channel=flags.Analysis.channel,
            isMC=flags.Input.isMC,
            bypass=flags.Analysis.bypass,
        )
    )

    # MMC decoration
    if flags.Analysis.do_baseline:
        baseline = "_baseline_"
    else:
        baseline = "_"
    if flags.Analysis.do_mmc:
        cfg.addEventAlgo(
            CompFactory.HLLTT.MMCDecoratorAlg(
                "MMCDecoratorAlg",
                jets="llttAnalysisJets_%SYS%",
                muons="llttAnalysisMuons_%SYS%",
                electrons="llttAnalysisElectrons_%SYS%",
                taus="llttAnalysisTaus_%SYS%",
                met="AnalysisMET_%SYS%",
                passLEPLEP="pass" + baseline + "LEPLEP_%SYS%",
                passLEPHAD="pass" + baseline + "LEPHAD_%SYS%",
                passHADHAD="pass" + baseline + "HADHAD_%SYS%",
                channel=flags.Analysis.channel,
            )
        )

        cfg.addEventAlgo(
            CompFactory.HLLTT.MMCSelectorAlg(
                "MMCSelectorAlg",
                MMC_min=1.0 * Units.GeV,
                eventDecisionOutputDecoration="lltt_pass_sr_%SYS%",
                bypass=flags.Analysis.bypass,
            )
        )

    # calculate final lltt vars
    cfg.addEventAlgo(
        CompFactory.HLLTT.BaselineVarsllttAlg(
            "FinalVarsllttAlg",
            isMC=flags.Input.isMC,
            jets="llttAnalysisJets_%SYS%",
            muons="llttAnalysisMuons_%SYS%",
            electrons="llttAnalysisElectrons_%SYS%",
            taus="llttAnalysisTaus_%SYS%",
            met="AnalysisMET_%SYS%",
            tauWP=flags.Analysis.Tau.ID,
            muonWP=TightMuonWPLabel,
            eleWP=TightElectronWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            floatVariableList=float_variables,
            intVariableList=int_variables,
        )
    )

    return cfg


def get_BaselineVarsllttAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for var in ["mll","ptll","drll","dphimetll","matt","ptatt","dratt",
                "maa","ptaa","draa","dphimetatt"]:
        float_variable_names.append(var)

    for var in ["isr", "recid", "nlep", "osatt", "nmuo", "nele", "ntaus",
                "njets", "nbjets", "diltype"]:
        int_variable_names.append(var)

    return float_variable_names, int_variable_names


def lltt_branches(flags):
    # a list of strings which maps the variable name in the c++ code
    # to the outputname:
    # "EventInfo.<var> -> <outputvarname>"
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsllttAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    float_variable_names, int_variable_names = get_BaselineVarsllttAlg_variables(flags)

    if flags.Analysis.do_mmc:
        # do not append mmc variables to float_variable_names
        # or int_variable_names as they are stored by the
        # mmc algortithm not BaselineVarsllttAlg
        for var in ["status", "pt", "eta", "phi", "m"]:
            all_baseline_variable_names.append(f"mmc_{var}")
        for var in ["maa", "ptaa", "draa"]:
            float_variable_names.append(f"mmc_{var}")

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> lltt_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    branches += ["EventInfo.lltt_pass_sr_%SYS% -> lltt_pass_SR_%SYS%"]

    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "lltt")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    # trigger variables do not need to be added to variable_names
    # as it is written out in HllttSelectorAlg
    for var in ["pass_trigger_SLT", "pass_Looseele", "pass_Loosemuo"]:
        branches += [f"EventInfo.{var}_%SYS% -> lltt_{var}_%SYS%"]

    for var in ["_trigger_", "_baseline_", "_"]:
        for cat in ["DLT"]:
            branches += [f"EventInfo.pass{var}{cat}_%SYS% -> "
                         f"lltt_pass{var}{cat}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    for var in ["_baseline_", "_"]:
        for cat in ["LEPLEP", "LEPHAD", "HADHAD"]:
            branches += [f"EventInfo.pass{var}{cat}_%SYS% -> "
                         f"lltt_pass{var}{cat}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches,float_variable_names,int_variable_names
