from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# bbtt analysis chain


def bbtt_cfg(flags, smalljetkey, muonkey, electronkey, taukey):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=muonkey,
            containerOutKey="bbttAnalysisMuons_%SYS%",
            minPt=7_000,
            maxEta=2.7,
            pTsort=True,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=electronkey,
            containerOutKey="bbttAnalysisElectrons_%SYS%",
            minPt=7_000,
            minEtaVeto=1.37,
            maxEtaVeto=1.52,
            maxEta=2.47,
            pTsort=True,
        )
    )

    # add Tau
    cfg.addEventAlgo(
        CompFactory.Easyjet.TauSelectorAlg(
            "TauSelectorAlg",
            containerInKey=taukey,
            containerOutKey="bbttAnalysisTaus_%SYS%",
            minPt=20_000,
            minEtaVeto=1.37,
            maxEtaVeto=1.52,
            maxEta=2.5,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,     # -1 means ignores this
            pTsort=True,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbttAnalysisJets_%SYS%",
            bTagWPDecorName="",  # empty string: "" ignores btagging
            minPt=25_000,
            maxEta=2.5,
            truncateAtAmount=2,  # -1 means keep all
            minimumAmount=2,  # -1 means ignores this
            pTsort=True,
        )
    )

    # Selection
    cfg.addEventAlgo(
        CompFactory.HH4B.HHbbttSelectorAlg(
            "HHbbttSelectorAlg",
            jets="bbttAnalysisJets_%SYS%",
            muons="bbttAnalysisMuons_%SYS%",
            electrons="bbttAnalysisElectrons_%SYS%",
            taus="bbttAnalysisTaus_%SYS%",
            met="AnalysisMET_%SYS%",
        )
    )

    # MMC decoration
    if flags.Analysis.do_mmc:
        cfg.addEventAlgo(
            CompFactory.HH4B.MMCDecoratorAlg(
                "MMCDecoratorAlg",
                jets="bbttAnalysisJets_%SYS%",
                muons="bbttAnalysisMuons_%SYS%",
                electrons="bbttAnalysisElectrons_%SYS%",
                taus="bbttAnalysisTaus_%SYS%",
                met="AnalysisMET_%SYS%",
            )
        )

    # calculate final bbtt vars
    cfg.addEventAlgo(
        CompFactory.HH4B.BaselineVarsbbttAlg(
            "FinalVarsbbttAlg",
            jets="bbttAnalysisJets_%SYS%",
            muons="bbttAnalysisMuons_%SYS%",
            electrons="bbttAnalysisElectrons_%SYS%",
            taus="bbttAnalysisTaus_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWP="",
        )
    )

    return cfg


def bbtt_branches(flags):
    branches = []

    bbtt_vars = [
        # Leading muon
        "Leading_Muon_pt",
        "Leading_Muon_eta",
        # Leading electron
        "Leading_Electron_pt",
        "Leading_Electron_eta",
        # Leading tau
        "Leading_Tau_pt",
        "Leading_Tau_eta",
    ]

    for tree_name in flags.Analysis.ttree_output.ttree_names:
        tree_flags = getattr(flags.Analysis.ttree_output,tree_name)
        for var in bbtt_vars:
            if tree_flags.write_object_systs_only_for_pt and "pt" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbtt_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbtt_%SYS%_{var}"]

    if flags.Analysis.do_mmc:
        for var in ["status", "pt", "eta", "phi", "m"]:
            if tree_flags.write_object_systs_only_for_pt and var != "pt":
                branches += [f"EventInfo.mmc_{var}_NOSYS -> mmc_{var}"]
            else:
                branches += [f"EventInfo.mmc_{var}_%SYS% -> mmc_%SYS%_{var}"]

    branches += ["EventInfo.pass_bbtt_sr_%SYS% -> bbtt_pass_SR_%SYS%"]

    return branches
