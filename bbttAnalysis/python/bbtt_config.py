from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units


def bbtt_cfg(flags, smalljetkey, muonkey, electronkey, taukey):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey='loose' + muonkey,
            containerOutKey="bbttAnalysisMuons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey='loose' + electronkey,
            containerOutKey="bbttAnalysisElectrons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.TauSelectorAlg(
            "TauSelectorAlg",
            containerInKey='baseline' + taukey,
            containerOutKey="bbttAnalysisTaus_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbttAnalysisJets_%SYS%",
            minPt=20 * Units.GeV,
            bTagWPDecorName="",  # empty string: "" ignores btagging
            minimumAmount=2,  # -1 means ignores this
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    # Selection
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    cfg.addEventAlgo(
        CompFactory.HHBBTT.HHbbttSelectorAlg(
            "HHbbttSelectorAlg",
            jets="bbttAnalysisJets_%SYS%",
            muons="bbttAnalysisMuons_%SYS%",
            electrons="bbttAnalysisElectrons_%SYS%",
            taus="bbttAnalysisTaus_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
            eventDecisionOutputDecoration="bbtt_pass_sr_noMMC_%SYS%",
            triggerlists=trigger_branches,
            channel=flags.Analysis.channel,
            isMC=flags.Input.isMC,
            Years=flags.Analysis.Years,
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
            CompFactory.HHBBTT.MMCDecoratorAlg(
                "MMCDecoratorAlg",
                jets="bbttAnalysisJets_%SYS%",
                muons="bbttAnalysisMuons_%SYS%",
                electrons="bbttAnalysisElectrons_%SYS%",
                taus="bbttAnalysisTaus_%SYS%",
                met="AnalysisMET_%SYS%",
                passSLT="pass" + baseline + "SLT_%SYS%",
                passLTT="pass" + baseline + "LTT_%SYS%",
                passSTT="pass" + baseline + "STT_%SYS%",
                passDTT="pass" + baseline + "DTT_%SYS%",
                channel=flags.Analysis.channel,
            )
        )

        cfg.addEventAlgo(
            CompFactory.HHBBTT.MMCSelectorAlg(
                "MMCSelectorAlg",
                MMC_min=60 * Units.GeV,
                eventDecisionOutputDecoration="bbtt_pass_sr_%SYS%",
                bypass=flags.Analysis.bypass,
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
        "Lepton_pt",
        "Lepton_eta",
        "Lepton_charge",
        "Lepton_pdgid",
        "Leading_Tau_pt",
        "Leading_Tau_eta",
        "Leading_Tau_charge",
        "Sublead_Tau_pt",
        "Sublead_Tau_eta",
        "Sublead_Tau_charge",
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

    for var in ["_trigger_", "_baseline_", "_"]:
        for cat in ["SLT", "LTT", "STT", "DTT"]:
            branches += [f"EventInfo.pass{var}{cat}_%SYS% -> bbtt_pass{var}{cat}_%SYS%"]

    return branches
