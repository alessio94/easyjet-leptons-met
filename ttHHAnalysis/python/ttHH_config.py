from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def ttHH_cfg(flags, smalljetkey, muonkey, electronkey,
             float_variables=[], int_variables=[]):

    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=MuonWPLabel + muonkey,
            containerOutKey="ttHHAnalysisMuons_%SYS%",
            minPt=10e3,
            muonSF_WP=MuonWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=ElectronWPLabel + electronkey,
            containerOutKey="ttHHAnalysisElectrons_%SYS%",
            minPt=10e3,
            eleSF_WP=ElectronWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallRJet_BTag_SelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="ttHHAnalysisJets_BTag_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            maxEta=2.5,
            minPt=20e3,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallRJet_SelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="ttHHAnalysisJets_%SYS%",
            bTagWPDecorName="",
            minPt=20e3,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.JetPairingAlgttHH(
            "JetPairingAlgHH",
            containerInKey="ttHHAnalysisJets_BTag_%SYS%",
            containerOutKey="pairedttHHAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            pairingStrategyName="chiSquare",
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.ttHHSelectorAlg(
            "ttHHSelectorAlg",
            bjets="pairedttHHAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            jets="ttHHAnalysisJets_%SYS%",
            muons="ttHHAnalysisMuons_%SYS%",
            electrons="ttHHAnalysisElectrons_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_ttHH_cutflow,
            triggers=flags.Analysis.TriggerChains,
            eventDecisionOutputDecoration="ttHH_pass_baseline_%SYS%",
            bypass=flags.Analysis.bypass,
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.JetPairingAlgttHH(
            "JetPairingAlgHZ",
            containerInKey="ttHHAnalysisJets_BTag_%SYS%",
            containerOutKey="pairedttHZAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            pairingStrategyName="chiSquare",
            targetMass1=125 * Units.GeV,
            targetMass2=91.2 * Units.GeV,
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.JetPairingAlgttHH(
            "JetPairingAlgZZ",
            containerInKey="ttHHAnalysisJets_BTag_%SYS%",
            containerOutKey="pairedttZZAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            pairingStrategyName="chiSquare",
            targetMass1=91.2 * Units.GeV,
            targetMass2=91.2 * Units.GeV,
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.BaselineVarsttHHAlg(
            "BaselineVarsttHHAlg",
            bjets="pairedttHHAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            jets="ttHHAnalysisJets_%SYS%",
            muons="ttHHAnalysisMuons_%SYS%",
            electrons="ttHHAnalysisElectrons_%SYS%",
            met="AnalysisMET_%SYS%",
            HZPairs="pairedttHZAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            ZZPairs="pairedttZZAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            isMC=flags.Input.isMC,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.EventInfoGlobalAlg(
            isMC=flags.Input.isMC,
            Years=flags.Analysis.Years,
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
        "HZ_H_m", "HZ_H_pt", "HZ_H_eta", "HZ_H_phi",
        "HZ_Z_m", "HZ_Z_pt", "HZ_Z_eta", "HZ_Z_phi",
        "ZZ_Z1_m", "ZZ_Z1_pt", "ZZ_Z1_eta", "ZZ_Z1_phi",
        "ZZ_Z2_m", "ZZ_Z2_pt", "ZZ_Z2_eta", "ZZ_Z2_phi",
        "HH_m", "HH_CHI",
        "HZ_m", "HZ_CHI",
        "ZZ_m", "ZZ_CHI"
    ]

    float_variable_names += H_candidate_variables

    angular_variables = [
        "Jets_DeltaR12", "Jets_DeltaR34", "Jets_DeltaR56",
        "Jets_DeltaR1234", "Jets_DeltaR3456", "Jets_DeltaR5612",
        "Jets_DeltaPhi12", "Jets_DeltaPhi34", "Jets_DeltaPhi56",
        "Jets_DeltaPhi1234", "Jets_DeltaPhi3456", "Jets_DeltaPhi5612",
        "Jets_DeltaEta12", "Jets_DeltaEta34", "Jets_DeltaEta56",
        "Jets_DeltaEta1234", "Jets_DeltaEta3456", "Jets_DeltaEta5612",
        "Jets_DeltaRMax", "Jets_DeltaRMin", "Jets_DeltaRMean",
        "Jets_DeltaEtaMax", "Jets_DeltaEtaMin", "Jets_DeltaEtaMean"
    ]

    float_variable_names += angular_variables

    float_variable_names += ["HT", "missEt", "metphi"]

    int_variable_names += [
        "nJets", "nBJets", "nLeptons",
        "dilept_type", "trilept_type", "total_charge"
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

    for tree_flags in flags.Analysis.ttree_output:
        for var in all_baseline_variable_names:
            if tree_flags['slim_variables_with_syst'] and \
               "pt" not in var and "SF" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> ttHH_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> ttHH_{var}_%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "ttHH")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    branches += ["EventInfo.PassAllCuts_%SYS% -> ttHH_PassAllCuts_%SYS%"]

    branches += ["EventInfo.ttHH_pass_baseline_%SYS% -> ttHH_pass_baseline_%SYS%"]

    if (flags.Analysis.save_ttHH_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> ttHH_{cut}_%SYS%"]

    branches += ["EventInfo.dataTakingYear -> dataTakingYear"]

    return branches, float_variable_names, int_variable_names
