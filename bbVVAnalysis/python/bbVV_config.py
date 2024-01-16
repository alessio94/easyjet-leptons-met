from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)


def bbVV_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=MuonWPLabel + muonkey,
            containerOutKey="bbVVAnalysisMuons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=ElectronWPLabel + electronkey,
            containerOutKey="bbVVAnalysisElectrons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbVVAnalysisJets_%SYS%",
            bTagWPDecorName="",  # empty string: "" ignores btagging
            maxEta=2.5,
            truncateAtAmount=2,  # -1 means keep all
            minimumAmount=2,  # -1 means ignores this
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "LargeJetSelectorAlg",
            containerInKey=largejetkey,
            containerOutKey="bbVVAnalysisLRJets_%SYS%",
            bTagWPDecorName="",  # empty string: "" ignores btagging
            minPt=200e3,
            maxEta=2.0,
            truncateAtAmount=3,  # -1 means keep all
            minimumAmount=2,  # -1 means ignores this
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    for btag_wp in flags.Analysis.large_R_jet.vr_btag_wps:
        # get the two leading large R's
        cfg.addEventAlgo(
            CompFactory.Easyjet.JetSelectorAlg(
                "LargeJetSelectorAlg_" + btag_wp,
                containerInKey=largejetkey,
                containerOutKey="boostedAnalysisJets_" + btag_wp,
                bTagWPDecorName="ftag_select_" + btag_wp,
                minPt=250e3,
                maxEta=2.0,
                truncateAtAmount=2,  # -1 means keep all
                minimumAmount=2,  # -1 means ignores this
                checkOR=flags.Analysis.do_overlap_removal,
            )
        )
        # get the ghost associated VR jets from the leading Large R jet
        cfg.addEventAlgo(
            CompFactory.Easyjet.GhostAssocVRJetGetterAlg(
                "LeadingLargeRGhostAssocVRJetGetterAlg_" + btag_wp,
                containerInKey="boostedAnalysisJets_" + btag_wp,
                containerOutKey="leadingLargeRVRJets_" + btag_wp,
                whichJet=0,
            )
        )
        # make sure we have at least 2 and maximally 3 ghost associated in
        # the leading large R jet
        cfg.addEventAlgo(
            CompFactory.Easyjet.JetSelectorAlg(
                "LeadingLargeRVRJetSelectorAlg_" + btag_wp,
                containerInKey="leadingLargeRVRJets_" + btag_wp,
                containerOutKey="SelectedLeadingLargeRVRJets_" + btag_wp,
                bTagWP=btag_wp,  # empty string: "" ignores btagging
                minPt=10e3,
                maxEta=2.5,
                truncateAtAmount=3,  # -1 means keep all
                minimumAmount=2,  # -1 means ignores this
                removeRelativeDeltaRToVRJet=True,
                checkOR=flags.Analysis.do_overlap_removal,
            )
        )

        # get the ghost associated VR jets from the subleading Large R jet
        cfg.addEventAlgo(
            CompFactory.Easyjet.GhostAssocVRJetGetterAlg(
                "SubLeadingLargeRGhostAssocVRJetGetterAlg_" + btag_wp,
                containerInKey="boostedAnalysisJets_" + btag_wp,
                containerOutKey="SubLeadingLargeRVRJets_" + btag_wp,
                whichJet=1,
            )
        )

        # make sure we have at least 2 and maximally 3 ghost associated in
        # the subleading large R jet
        cfg.addEventAlgo(
            CompFactory.Easyjet.JetSelectorAlg(
                "SubLeadingLargeRVRJetSelectorAlg_" + btag_wp,
                containerInKey="SubLeadingLargeRVRJets_" + btag_wp,
                containerOutKey="SelectedSubLeadingLargeRVRJets_" + btag_wp,
                bTagWP=btag_wp,  # empty string: "" ignores btagging
                minPt=10e3,
                maxEta=2.5,
                truncateAtAmount=3,  # -1 means keep all
                minimumAmount=2,  # -1 means ignores this
                removeRelativeDeltaRToVRJet=True,
            )
        )

    # Selection
    TightMuonWP = flags.Analysis.Muon.extra_wps[0]
    TightMuonWPLabel = f'{TightMuonWP[0]}_{TightMuonWP[1]}'
    TightEleWP = flags.Analysis.Electron.extra_wps[0]
    TightEleWPLabel = f'{TightEleWP[0]}_{TightEleWP[1]}'
    cfg.addEventAlgo(
        CompFactory.HHBBVV.HHbbVVSelectorAlg(
            "HHbbVVSelectorAlg",
            jets="bbVVAnalysisJets_%SYS%",
            lrjets="bbVVAnalysisLRJets_%SYS%",
            muons="bbVVAnalysisMuons_%SYS%",
            electrons="bbVVAnalysisElectrons_%SYS%",
            met="AnalysisMET_%SYS%",
            eventDecisionOutputDecoration="bbVV_pass_sr_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            muonWP=TightMuonWPLabel,
            eleWP=TightEleWPLabel,
            channel=flags.Analysis.channel,
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # calculate final bbVV vars
    cfg.addEventAlgo(
        CompFactory.HHBBVV.BaselineVarsbbVVAlg(
            "FinalVarsbbVVAlg",
            jets="bbVVAnalysisJets_%SYS%",
            lrjets="bbVVAnalysisLRJets_%SYS%",
            muons="bbVVAnalysisMuons_%SYS%",
            electrons="bbVVAnalysisElectrons_%SYS%",
            muonWP=TightMuonWPLabel,
            eleWP=TightEleWPLabel,
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
        )
    )

    return cfg


def bbVV_branches(flags):
    branches = []

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    branches += get_selected_objects_branches(flags, "bbVV")

    #     for hh in ["HH", "HH_vis", "HH_visMet"]:
    #         for var in ["pt", "eta", "phi", "m"]:
    #             if tree_flags['slim_variables_with_syst'] and "pt" not in var:
    #                 branches += [f"EventInfo.{hh}_{var}_NOSYS -> {hh}_{var}"]
    #             else:
    #                 branches += [f"EventInfo.{hh}_{var}_%SYS% -> {hh}_%SYS%_{var}"]

    branches += ["EventInfo.bbVV_pass_sr_%SYS% -> bbVV_pass_SR_%SYS%"]

    if flags.Input.isMC:
        branches += ["EventInfo.ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%"
                     " -> weight_ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%",]

        branches += ["EventInfo.jvt_effSF_%SYS% -> weight_jvt_effSF_%SYS%"]

    return branches
