from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# bbll analysis chain

def bbll_cfg(flags, smalljetkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=muonkey,
            containerOutKey="bbllAnalysisMuons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=electronkey,
            containerOutKey="bbllAnalysisElectrons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbllAnalysisJets_%SYS%",
            bTagWPDecorName="",  # empty string: "" ignores btagging
            truncateAtAmount=4,  # -1 means keep all
            minimumAmount=4,  # -1 means ignores this
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
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
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
            jets="bbllAnalysisJets_%SYS%",
            muons="bbllAnalysisMuons_%SYS%",
            electrons="bbllAnalysisElectrons_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
        )
    )

    return cfg


def bbll_branches(flags):
    branches = []

    bbll_vars = [
        # Leading muon
        "Leading_Muon_pt",
        "Leading_Muon_eta",
        # Leading electron
        "Leading_Electron_pt",
        "Leading_Electron_eta",
    ]

    for tree_flags in flags.Analysis.ttree_output:
        for var in bbll_vars:
            if tree_flags['write_object_systs_only_for_pt'] and "pt" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbll_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbll_%SYS%_{var}"]

    if flags.Analysis.do_mmc:
        for var in ["status", "pt", "eta", "phi", "m"]:
            if tree_flags['write_object_systs_only_for_pt'] and var != "pt":
                branches += [f"EventInfo.mmc_{var}_NOSYS -> mmc_{var}"]
            else:
                branches += [f"EventInfo.mmc_{var}_%SYS% -> mmc_%SYS%_{var}"]

    branches += ["EventInfo.bbll_pass_sr_%SYS% -> bbll_pass_SR_%SYS%"]

    return branches
