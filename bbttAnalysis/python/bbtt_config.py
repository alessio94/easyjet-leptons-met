from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def bbtt_cfg(flags, smalljetkey, muonkey, electronkey, taukey):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey='loose' + muonkey,
            containerOutKey="bbttAnalysisMuons_%SYS%",
            minPt=7_000,
            maxEta=2.7,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,     # -1 means ignores this
            pTsort=True,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey='loose' + electronkey,
            containerOutKey="bbttAnalysisElectrons_%SYS%",
            minPt=7_000,
            minEtaVeto=1.37,
            maxEtaVeto=1.52,
            maxEta=2.47,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,     # -1 means ignores this
            pTsort=True,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.TauSelectorAlg(
            "TauSelectorAlg",
            containerInKey='baseline' + taukey,
            containerOutKey="bbttAnalysisTaus_%SYS%",
            minPt=20_000,
            minEtaVeto=1.37,
            maxEtaVeto=1.52,
            maxEta=2.5,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,     # -1 means ignores this
            pTsort=True,
            checkOR=flags.Analysis.do_overlap_removal,
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
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    # MMC decoration
    if flags.Analysis.do_mmc:
        cfg.addEventAlgo(
            CompFactory.HHBBTT.MMCDecoratorAlg(
                "MMCDecoratorAlg",
                jets="bbttAnalysisJets_%SYS%",
                muons="bbttAnalysisMuons_%SYS%",
                electrons="bbttAnalysisElectrons_%SYS%",
                taus="bbttAnalysisTaus_%SYS%",
                met="AnalysisMET_%SYS%",
            )
        )

    # Selection
    cfg.addEventAlgo(
        CompFactory.HHBBTT.HHbbttSelectorAlg(
            "HHbbttSelectorAlg",
            jets="bbttAnalysisJets_%SYS%",
            muons="bbttAnalysisMuons_%SYS%",
            electrons="bbttAnalysisElectrons_%SYS%",
            taus="bbttAnalysisTaus_%SYS%",
            met="AnalysisMET_%SYS%",
            eventDecisionOutputDecoration="bbtt_pass_sr_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
            channel=flags.Analysis.channel,
        )
    )

    # calculate final bbtt vars
    cfg.addEventAlgo(
        CompFactory.HHBBTT.BaselineVarsbbttAlg(
            "FinalVarsbbttAlg",
            jets="bbttAnalysisJets_%SYS%",
            muons="bbttAnalysisMuons_%SYS%",
            electrons="bbttAnalysisElectrons_%SYS%",
            taus="bbttAnalysisTaus_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
        )
    )

    return cfg


def bbtt_branches(flags):
    branches = []

    bbtt_vars = [
        "Selected_Lepton_pt",
        "Selected_Lepton_eta",
        "Selected_Lepton_charge",
        "Selected_Lepton_pdgid",
        "Selected_Tau_pt",
        "Selected_Tau_eta",
        "Selected_Tau_charge",
    ]

    for tree_flags in flags.Analysis.ttree_output:
        for var in bbtt_vars:
            if tree_flags['write_object_systs_only_for_pt'] and "pt" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbtt_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbtt_%SYS%_{var}"]

    if flags.Analysis.do_mmc:
        for var in ["status", "pt", "eta", "phi", "m"]:
            if tree_flags['write_object_systs_only_for_pt'] and var != "pt":
                branches += [f"EventInfo.mmc_{var}_NOSYS -> mmc_{var}"]
            else:
                branches += [f"EventInfo.mmc_{var}_%SYS% -> mmc_%SYS%_{var}"]

        for hh in ["HH", "HH_vis"]:
            for var in ["pt", "eta", "phi", "m"]:
                if tree_flags['write_object_systs_only_for_pt'] and "pt" not in var:
                    branches += [f"EventInfo.{hh}_{var}_NOSYS -> {hh}_{var}"]
                else:
                    branches += [f"EventInfo.{hh}_{var}_%SYS% -> {hh}_%SYS%_{var}"]

    branches += ["EventInfo.bbtt_pass_sr_%SYS% -> bbtt_pass_SR_%SYS%"]

    branches += ["EventInfo.pass_SLT_%SYS% -> bbtt_pass_SLT_%SYS%"]
    branches += ["EventInfo.pass_LTT_%SYS% -> bbtt_pass_LTT_%SYS%"]

    return branches
