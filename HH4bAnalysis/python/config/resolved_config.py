from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# this is a resolved dihiggs analysis chain


def resolved_cfg(flags, smalljetkey):
    cfg = ComponentAccumulator()

    # this is a resolved dihiggs analysis chain
    for btag_wp in flags.Analysis.btag_wps:
        # get the 4 leading small R jets
        cfg.addEventAlgo(
            CompFactory.HH4B.JetSelectorAlg(
                "SmallJetSelectorAlg_" + btag_wp,
                containerInKey=smalljetkey,
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
            CompFactory.HH4B.BaselineVarsResolvedAlg(
                "FinalVarsResolvedAlg_" + btag_wp,
                smallRContainerInKey="pairedResolvedAnalysisJets_" + btag_wp,
                bTagWP=btag_wp,
            )
        )

        # truth matching the paired jets
        if flags.Analysis.truth_match_resolved and flags.Input.isMC:
            cfg.addEventAlgo(
                CompFactory.HH4B.JetTruthMatcherAlg(
                    "ResolvedJetTruthMatcherAlg_" + btag_wp,
                    smallRContainerInKey="pairedResolvedAnalysisJets_" + btag_wp,
                    bTagWP=btag_wp,
                    regime="resolved",
                    containerOutKey="resolvedTruthMatched_out_" + btag_wp,
                )
            )
            cfg.addEventAlgo(
                CompFactory.HH4B.JetSelectorAlg(
                    "ResolvedTruthMatchDecoratorAlg_" + btag_wp,
                    containerInKey="resolvedTruthMatched_out_" + btag_wp,
                    containerOutKey="resolved_truthMatched_" + btag_wp,
                    bTagWP="",  # empty string: "" ignores btagging
                    minPt=0,
                    maxEta=10,
                    truncateAtAmount=-1,  # -1 means keep all
                    minimumAmount=-1,  # -1 means ignores this
                    pTsort=False,
                )
            )

    return cfg


def resolved_branches(flags):
    branches = []

    for btag_wp in flags.Analysis.btag_wps:
        resolved_vars = [
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
            resolved_vars += [
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
                    f"EventInfo.resolved_truthMatched_{btag_wp}_{var} ->"
                    f" resolved_truthMatched_{btag_wp}_{var}"
                ]

        for var in resolved_vars:
            branches += [
                f"EventInfo.resolved_{var}_{btag_wp} -> resolved_{btag_wp}_{var}"
            ]

    return branches
