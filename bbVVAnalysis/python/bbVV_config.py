from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def bbVV_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey='loose' + muonkey,
            containerOutKey="bbVVAnalysisMuons_%SYS%",
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
            containerOutKey="bbVVAnalysisElectrons_%SYS%",
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
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbVVAnalysisJets_%SYS%",
            bTagWPDecorName="",  # empty string: "" ignores btagging
            minPt=25_000,
            maxEta=2.5,
            truncateAtAmount=2,  # -1 means keep all
            minimumAmount=2,  # -1 means ignores this
            pTsort=True,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "LargeJetSelectorAlg",
            containerInKey=largejetkey,
            containerOutKey="bbVVAnalysisLRJets_%SYS%",
            bTagWPDecorName="",  # empty string: "" ignores btagging
            minPt=200_000,
            maxEta=2.,
            truncateAtAmount=3,  # -1 means keep all
            minimumAmount=2,  # -1 means ignores this
            pTsort=True,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    for btag_wp in flags.Analysis.large_R.vr_btag_wps:
        # get the two leading large R's
        cfg.addEventAlgo(
            CompFactory.Easyjet.JetSelectorAlg(
                "LargeJetSelectorAlg_" + btag_wp,
                containerInKey=largejetkey,
                containerOutKey="boostedAnalysisJets_" + btag_wp,
                bTagWPDecorName="ftag_select_" + btag_wp,
                minPt=250_000,
                maxEta=2.0,
                truncateAtAmount=2,  # -1 means keep all
                minimumAmount=2,  # -1 means ignores this
                pTsort=True,
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
                minPt=10_000,
                maxEta=2.5,
                truncateAtAmount=3,  # -1 means keep all
                minimumAmount=2,  # -1 means ignores this
                pTsort=True,
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
                minPt=10_000,
                maxEta=2.5,
                truncateAtAmount=3,  # -1 means keep all
                minimumAmount=2,  # -1 means ignores this
                pTsort=True,
                removeRelativeDeltaRToVRJet=True,
            )
        )

    # Selection
    cfg.addEventAlgo(
        CompFactory.HHBBVV.HHbbVVSelectorAlg(
            "HHbbVVSelectorAlg",
            jets="bbVVAnalysisJets_%SYS%",
            lrjets="bbVVAnalysisLRJets_%SYS%",
            muons="bbVVAnalysisMuons_%SYS%",
            electrons="bbVVAnalysisElectrons_%SYS%",
            met="AnalysisMET_%SYS%",
            eventDecisionOutputDecoration="bbVV_pass_sr_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
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
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
        )
    )

    return cfg


def bbVV_branches(flags):
    branches = []

    bbVV_vars = [
        "Selected_Lepton_pt",
        "Selected_Lepton_eta",
        "Selected_Lepton_charge",
        "Selected_Lepton_pdgid",
    ]

    for tree_flags in flags.Analysis.ttree_output:
        for var in bbVV_vars:
            if tree_flags['write_object_systs_only_for_pt'] and "pt" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbVV_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbVV_%SYS%_{var}"]

    #     for hh in ["HH", "HH_vis", "HH_visMet"]:
    #         for var in ["pt", "eta", "phi", "m"]:
    #             if tree_flags['write_object_systs_only_for_pt'] and "pt" not in var:
    #                 branches += [f"EventInfo.{hh}_{var}_NOSYS -> {hh}_{var}"]
    #             else:
    #                 branches += [f"EventInfo.{hh}_{var}_%SYS% -> {hh}_%SYS%_{var}"]

    branches += ["EventInfo.bbVV_pass_sr_%SYS% -> bbVV_pass_SR_%SYS%"]

    return branches
