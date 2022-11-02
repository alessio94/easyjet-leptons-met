from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# from HH4bAnalysis.utils.containerNameHelper import get_container_names


def DiHiggsAnalysisChainCfg(flags, SmallJetKey, LargeJetKey):
    cfg = ComponentAccumulator()

    # this is a resolved dihiggs analysis chain
    if flags.Analysis.do_resolved_dihiggs_analysis:
        for btag_wp in flags.Analysis.btag_wps:
            # get the 4 leading small R jets
            cfg.addEventAlgo(
                CompFactory.HH4B.JetSelectorAlg(
                    "SmallJetSelectorAlg_" + btag_wp,
                    containerInKey=SmallJetKey,
                    containerOutKey="resolvedAnalysisJets_" + btag_wp,
                    bTagWP=btag_wp,  # empty string: "" ignores btagging
                    minPt=20_000,
                    maxEta=2.5,
                    truncateAtAmount=4,  # -1 means keep all
                    minimumAmount=4,  # -1 means ignores this
                    pTsort=True,
                )
            )

            # pair them with some strategy and save them as leading (h1) and
            # subleading (h2) Higgs candidates in the order:
            # h1_leading_pt_jet
            # h1_subleading_pt_jet
            # h2_leading_pt_jet
            # h2_subleading_pt_jet
            cfg.addEventAlgo(
                CompFactory.HH4B.JetPairingAlg(
                    "JetPairingAlg_" + btag_wp,
                    containerInKey="resolvedAnalysisJets_" + btag_wp,
                    containerOutKey="pairedResolvedAnalysisJets_" + btag_wp,
                    pairingStrategy="minDeltaR",  # so far only minDeltaR
                )
            )

            # calculate final resolved vars
            cfg.addEventAlgo(
                CompFactory.HH4B.FinalVarsResolvedAlg(
                    "FinalVarsResolvedAlg_" + btag_wp,
                    smallRContainerInKey="pairedResolvedAnalysisJets_" + btag_wp,
                    bTagWP=btag_wp,
                )
            )

            # truth matching the paired jets
            if flags.Analysis.truth_match_resolved and flags.Input.isMC:
                cfg.addEventAlgo(
                    CompFactory.HH4B.JetTruthMatcherAlg(
                        "JetTruthMatcherAlg_" + btag_wp,
                        containerInKey="pairedResolvedAnalysisJets_" + btag_wp,
                        bTagWP=btag_wp,
                    )
                )

    # this is the boosted analysis chain
    if flags.Analysis.do_boosted_dihiggs_analysis:
        for btag_wp in flags.Analysis.vr_btag_wps:
            # get the two leading large R's
            cfg.addEventAlgo(
                CompFactory.HH4B.JetSelectorAlg(
                    "LargeJetSelectorAlg_" + btag_wp,
                    containerInKey=LargeJetKey,
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
                    subLeadingLargeR_GA_VRJets="SelectedSubLeadingLargeRVRJets_"
                    + btag_wp,
                    bTagWP=btag_wp,
                )
            )

    return cfg


def DiHiggsAnalysisAddBranches(flags):
    analysisTreeBranches = []

    if flags.Analysis.do_resolved_dihiggs_analysis:
        for btag_wp in flags.Analysis.btag_wps:
            resolvedVars = [
                "DeltaR12",
                "DeltaR13",
                "DeltaR14",
                "DeltaR23",
                "DeltaR24",
                "DeltaR34",
                "h1_m",
                "h2_m",
                "hh_m",
            ]
            if flags.Analysis.truth_match_resolved and flags.Input.isMC:
                resolvedVars += [
                    "h1_closestTruthBsHaveSameInitialParticle",
                    "h2_closestTruthBsHaveSameInitialParticle",
                    "h1_dR_leadingJet_closestTruthB",
                    "h1_dR_subleadingJet_closestTruthB",
                    "h2_dR_leadingJet_closestTruthB",
                    "h2_dR_subleadingJet_closestTruthB",
                ]

            for var in resolvedVars:
                analysisTreeBranches += [
                    f"EventInfo.resolved_{var}_{btag_wp} -> resolved_{btag_wp}_{var}"
                ]

    if flags.Analysis.do_boosted_dihiggs_analysis:
        for btag_wp in flags.Analysis.vr_btag_wps:
            boostedVars = [
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
            for var in boostedVars:
                analysisTreeBranches += [
                    f"EventInfo.boosted_{var}_{btag_wp} -> boosted_{btag_wp}_{var}"
                ]

    return analysisTreeBranches
