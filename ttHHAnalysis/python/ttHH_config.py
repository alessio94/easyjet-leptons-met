from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)


def ttHH_cfg(flags, smalljetkey, muonkey, electronkey):
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
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallRJet_SelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="ttHHAnalysisJets_%SYS%",
            bTagWPDecorName="",
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
            HZPairs="pairedttHZAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            ZZPairs="pairedttZZAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            isMC=flags.Input.isMC
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.SelectionFlagsttHHAlg(
            "SelectionFlagsttHHAlg",
            bjets="pairedttHHAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            jets="ttHHAnalysisJets_%SYS%",
            muons="ttHHAnalysisMuons_%SYS%",
            electrons="ttHHAnalysisElectrons_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_ttHH_cutflow,
            triggers=flags.Analysis.TriggerChains,
            nLeptons=flags.Analysis.nLeptons
        )
    )

    return cfg


def ttHH_branches(flags):
    branches = []

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    branches += get_selected_objects_branches(flags, "ttHH")

    if (flags.Analysis.save_ttHH_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> ttHH_{cut}_%SYS%"]

    # BJets
    if flags.Input.isMC:
        branches += ["EventInfo.ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%"
                     " -> weight_ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%",]

        branches += ["EventInfo.jvt_effSF_%SYS% -> weight_jvt_effSF_%SYS%"]

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
    for var in H_candidate_variables:
        branches += [f"EventInfo.{var}_%SYS% -> ttHH_{var}_%SYS%"]

    # additional variables
    additional_variables = ["HT", "nJets", "nBJets"]
    for var in additional_variables:
        branches += [f"EventInfo.{var}_%SYS% -> ttHH_{var}_%SYS%"]

    angular_variables = [
        "DeltaR12", "DeltaR34", "DeltaR56",
        "DeltaR1234", "DeltaR3456", "DeltaR5612",
        "DeltaPhi12", "DeltaPhi34", "DeltaPhi56",
        "DeltaPhi1234", "DeltaPhi3456", "DeltaPhi5612",
        "DeltaEta12", "DeltaEta34", "DeltaEta56",
        "DeltaEta1234", "DeltaEta3456", "DeltaEta5612",
        "DeltaRMax", "DeltaRMin", "DeltaRMean",
        "DeltaEtaMax", "DeltaEtaMin", "DeltaEtaMean"
    ]

    for var in angular_variables:
        branches += [f"EventInfo.Jets_{var}_%SYS% -> ttHH_Jets_{var}_%SYS%"]

    branches += ["EventInfo.PassAllCuts_%SYS% -> ttHH_PassAllCuts_%SYS%"]

    leptonPair_variables = ["pt", "eta", "phi", "m", "dR"]
    leptonPairs = [
        "ee", "mumu", "emu"
    ]

    for lep in leptonPairs:
        for var in leptonPair_variables:
            branches += [f"EventInfo.{lep}_{var}_%SYS% -> ttHH_{lep}_{var}_%SYS%"]

    return branches
