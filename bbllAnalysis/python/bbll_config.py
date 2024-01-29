from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)


def bbll_cfg(flags, smalljetkey, muonkey, electronkey):
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
            minPt=10 * Units.GeV,
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

    return cfg


def bbll_branches(flags):
    branches = []

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    branches += get_selected_objects_branches(flags, "bbll")

    dilepton_variables = ["dRee", "Etaee", "Phiee",
                          "dRmumu", "Etamumu", "Phimumu",
                          "dRemu", "Etaemu", "Phiemu"]
    for tree_flags in flags.Analysis.ttree_output:
        for var in dilepton_variables:
            if tree_flags['slim_variables_with_syst'] and "pt" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbll_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbll_{var}_%SYS%"]

    branches += ["EventInfo.mee_%SYS% -> bbll_mee_%SYS%"]
    branches += ["EventInfo.pTee_%SYS% -> bbll_pTee_%SYS%"]
    branches += ["EventInfo.mmumu_%SYS% -> bbll_mmumu_%SYS%"]
    branches += ["EventInfo.pTmumu_%SYS% -> bbll_pTmumu_%SYS%"]
    branches += ["EventInfo.memu_%SYS% -> bbll_memu_%SYS%"]
    branches += ["EventInfo.pTemu_%SYS% -> bbll_pTemu_%SYS%"]
    branches += ["EventInfo.mll_%SYS% -> bbll_mll_%SYS%"]
    branches += ["EventInfo.pTll_%SYS% -> bbll_pTll_%SYS%"]

    # BJets
    if flags.Input.isMC:
        branches += ["EventInfo.ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%"
                     " -> weight_ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%",]

        branches += ["EventInfo.jvt_effSF_%SYS% -> weight_jvt_effSF_%SYS%"]

    branches += ["EventInfo.nBJets_%SYS% -> bbll_nBJets_%SYS%"]
    branches += ["EventInfo.nElectrons_%SYS% -> bbll_nElectrons_%SYS%"]
    branches += ["EventInfo.nMuons_%SYS% -> bbll_nMuons_%SYS%"]
    branches += ["EventInfo.nJets_%SYS% -> bbll_nJets_%SYS%"]

    dibjet_variables = ["dRbb", "Etabb", "Phibb"]
    for tree_flags in flags.Analysis.ttree_output:
        for var in dibjet_variables:
            if tree_flags['slim_variables_with_syst'] and "pt" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbll_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbll_{var}_%SYS%"]

    branches += ["EventInfo.mbb_%SYS% -> bbll_mbb_%SYS%"]
    branches += ["EventInfo.pTbb_%SYS% -> bbll_pTbb_%SYS%"]

    if flags.Analysis.do_mmc:
        for var in ["status", "pt", "eta", "phi", "m"]:
            if tree_flags['slim_variables_with_syst'] and var != "pt":
                branches += [f"EventInfo.mmc_{var}_NOSYS -> bbll_mmc_{var}"]
            else:
                branches += [f"EventInfo.mmc_{var}_%SYS% -> bbll_mmc_{var}_%SYS%"]

    branches += ["EventInfo.bbll_pass_sr_%SYS% -> bbll_pass_SR_%SYS%"]

    if (flags.Analysis.save_bbll_cutflow):
        cutList = flags.Analysis.CutList + flags.Analysis.Categories
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> bbll_{cut}_%SYS%"]

    return branches
