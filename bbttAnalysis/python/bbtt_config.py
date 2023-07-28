from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# bbtt analysis chain


def bbtt_cfg(flags, smalljetkey, muonkey, electronkey, taukey):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=muonkey,
            containerOutKey="bbttAnalysisMuons",
            minPt=7_000,
            maxEta=2.7,
            pTsort=True,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=electronkey,
            containerOutKey="bbttAnalysisElectrons",
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
            containerOutKey="bbttAnalysisTaus",
            minPt=20_000,
            minEtaVeto=1.37,
            maxEtaVeto=1.52,
            maxEta=2.5,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,     # -1 means ignores this
            pTsort=True,
        )
    )

    for btag_wp in flags.Analysis.btag_wps:
        cfg.addEventAlgo(
            CompFactory.Easyjet.JetSelectorAlg(
                "SmallJetSelectorAlg_" + btag_wp,
                containerInKey=smalljetkey,
                containerOutKey="bbttAnalysisJets_" + btag_wp,
                bTagWP=btag_wp,  # empty string: "" ignores btagging
                minPt=25_000,
                maxEta=2.5,
                truncateAtAmount=2,  # -1 means keep all
                minimumAmount=2,  # -1 means ignores this
                pTsort=True,
            )
        )

        # TODO: Output currently only works for 1 b-tagging WP
        cfg.addEventAlgo(
            CompFactory.HH4B.MMCDecoratorAlg(
                "MMCDecoratorAlg_" + btag_wp,
                jets="bbttAnalysisJets_" + btag_wp,
                muons="bbttAnalysisMuons",
                electrons="bbttAnalysisElectrons",
                taus="bbttAnalysisTaus",
                met="AnalysisMET_%SYS%",
            )
        )

        # calculate final bbtt vars
        cfg.addEventAlgo(
            CompFactory.HH4B.BaselineVarsbbttAlg(
                "FinalVarsbbttAlg_" + btag_wp,
                smallRContainerInKey="bbttAnalysisJets_" + btag_wp,
                muonContainerInKey="bbttAnalysisMuons",
                electronContainerInKey="bbttAnalysisElectrons",
                tauContainerInKey="bbttAnalysisTaus",
                bTagWP=btag_wp,
            )
        )

    return cfg


def bbtt_branches(flags):
    branches = []

    for btag_wp in flags.Analysis.btag_wps:
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

        for var in bbtt_vars:
            branch_name = f"EventInfo.{var}_{btag_wp} -> {btag_wp}_{var}"
            branches += [branch_name]

    for var in ["status", "pt", "eta", "phi", "m"]:
        branches += [f"EventInfo.mmc_{var}_%SYS% -> mmc_%SYS%_{var}"]

    return branches
