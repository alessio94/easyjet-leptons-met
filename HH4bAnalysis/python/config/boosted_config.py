from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# this is a boosted analysis chain


def boosted_cfg(flags, largejetkey):
    cfg = ComponentAccumulator()

    for btag_wp in flags.Analysis.vr_btag_wps:
        # get the two leading large R's
        cfg.addEventAlgo(
            CompFactory.HH4B.JetSelectorAlg(
                "LargeJetSelectorAlg_" + btag_wp,
                containerInKey=largejetkey,
                containerOutKey="boostedAnalysisJets_" + btag_wp,
                bTagWP="",  # empty string: "" ignores btagging
                minPt=250_000,
                maxEta=2.0,
                truncateAtAmount=2,  # -1 means keep all
                minimumAmount=2,  # -1 means ignores this
                pTsort=True,
            )
        )
        # get the ghost associated VR jets from the leading Large R jet
        cfg.addEventAlgo(
            CompFactory.HH4B.GhostAssocVRJetGetterAlg(
                "LeadingLargeRGhostAssocVRJetGetterAlg_" + btag_wp,
                containerInKey="boostedAnalysisJets_" + btag_wp,
                containerOutKey="leadingLargeRVRJets_" + btag_wp,
                whichJet=0,
            )
        )
        # make sure we have at least 2 and maximally 3 ghost associated in
        # the leading large R jet
        cfg.addEventAlgo(
            CompFactory.HH4B.JetSelectorAlg(
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
            )
        )

        # get the ghost associated VR jets from the subleading Large R jet
        cfg.addEventAlgo(
            CompFactory.HH4B.GhostAssocVRJetGetterAlg(
                "SubLeadingLargeRGhostAssocVRJetGetterAlg_" + btag_wp,
                containerInKey="boostedAnalysisJets_" + btag_wp,
                containerOutKey="SubLeadingLargeRVRJets_" + btag_wp,
                whichJet=1,
            )
        )

        # make sure we have at least 2 and maximally 3 ghost associated in
        # the subleading large R jet
        cfg.addEventAlgo(
            CompFactory.HH4B.JetSelectorAlg(
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

        # calculate final boosted vars
        cfg.addEventAlgo(
            CompFactory.HH4B.FinalVarsBoostedAlg(
                "FinalVarsBoostedAlg_" + btag_wp,
                largeRContainerInKey="boostedAnalysisJets_" + btag_wp,
                leadingLargeR_GA_VRJets="SelectedLeadingLargeRVRJets_" + btag_wp,
                subLeadingLargeR_GA_VRJets="SelectedSubLeadingLargeRVRJets_" + btag_wp,
                bTagWP=btag_wp,
            )
        )

        # truth matching the vr jets
        if flags.Analysis.truth_match_boosted and flags.Input.isMC:
            cfg.addEventAlgo(
                CompFactory.HH4B.JetTruthMatcherAlg(
                    "BoostedJetTruthMatcherAlg_" + btag_wp,
                    leadingLargeR_GA_VRJets="SelectedLeadingLargeRVRJets_" + btag_wp,
                    subLeadingLargeR_GA_VRJets="SelectedSubLeadingLargeRVRJets_"
                    + btag_wp,
                    bTagWP=btag_wp,
                    regime="boosted",
                    containerOutKey="boostedTruthMatched_out_" + btag_wp,
                )
            )
            cfg.addEventAlgo(
                CompFactory.HH4B.JetSelectorAlg(
                    "BoostedTruthMatchDecoratorAlg_" + btag_wp,
                    containerInKey="boostedTruthMatched_out_" + btag_wp,
                    containerOutKey="boosted_truthMatched_" + btag_wp,
                    bTagWP="",  # empty string: "" ignores btagging
                    minPt=0,
                    maxEta=10,
                    truncateAtAmount=-1,  # -1 means keep all
                    minimumAmount=-1,  # -1 means ignores this
                    pTsort=False,
                )
            )

    return cfg


def boosted_branches(flags):
    branches = []

    for btag_wp in flags.Analysis.vr_btag_wps:
        boosted_vars = [
            "h1_m",
            "h1_jet1_pt",
            "h1_jet2_pt",
            "h1_dR_jets",
            "h2_m",
            "h2_jet1_pt",
            "h2_jet2_pt",
            "h2_dR_jets",
            "hh_m",
        ]
        if flags.Analysis.truth_match_boosted and flags.Input.isMC:
            boosted_vars += [
                "h1_closestTruthBsHaveSameInitialParticle",
                "h2_closestTruthBsHaveSameInitialParticle",
                "h1_dR_leadingJet_closestTruthB",
                "h1_dR_subleadingJet_closestTruthB",
                "h2_dR_leadingJet_closestTruthB",
                "h2_dR_subleadingJet_closestTruthB",
                "h1_parentPdgId_leadingJet_closestTruthB",
                "h1_parentPdgId_subleadingJet_closestTruthB",
                "h2_parentPdgId_leadingJet_closestTruthB",
                "h2_parentPdgId_subleadingJet_closestTruthB",
            ]
            for var in ["pt", "eta", "phi", "m"]:
                branches += [
                    f"EventInfo.boosted_truthMatched_{btag_wp}_{var} ->"
                    f" boosted_truthMatched_{btag_wp}_{var}"
                ]

        for var in boosted_vars:
            branches += [
                f"EventInfo.boosted_{var}_{btag_wp} -> boosted_{btag_wp}_{var}"
            ]

    return branches
