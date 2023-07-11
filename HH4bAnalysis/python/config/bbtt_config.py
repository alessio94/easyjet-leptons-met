from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# bbtt analysis chain


def bbtt_cfg(flags, smalljetkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=muonkey,
            containerOutKey="bbttAnalysisMuons",
            minPt=21_000,
            maxEta=2.5,
            pTsort=True,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=electronkey,
            containerOutKey="bbttAnalysisElectrons",
            minPt=25_000,
            maxEta=2.5,
            pTsort=True,
        )
    )

    for btag_wp in flags.Analysis.btag_wps:
        # get the 2 leading small R jets
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

        # calculate final bbtt vars
        cfg.addEventAlgo(
            CompFactory.HH4B.BaselineVarsbbttAlg(
                "FinalVarsbbttAlg_" + btag_wp,
                smallRContainerInKey="bbttAnalysisJets_" + btag_wp,
                muonContainerInKey="bbttAnalysisMuons",
                electronContainerInKey="bbttAnalysisElectrons",
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
        ]

        for var in bbtt_vars:
            branch_name = f"EventInfo.{var}_{btag_wp} -> {btag_wp}_{var}"
            branches += [branch_name]

    return branches
