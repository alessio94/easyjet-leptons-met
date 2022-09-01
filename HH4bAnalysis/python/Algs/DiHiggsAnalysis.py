from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def DiHiggsAnalysisAlgCfg(
    flags,
    SmallJetKey,
    LargeJetKey,
    btag_wps,
    vr_btag_wps,
):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.DiHiggsAnalysisAlg(
            "DiHiggsAnalysis",
            EventInfoKey="EventInfo",
            SmallJetKey=SmallJetKey,
            LargeJetKey=LargeJetKey,
            doResolvedAnalysis=flags.do_resolved_analysis,
            doBoostedAnalysis=flags.do_boosted_analysis,
            btag_wps=btag_wps,
            vr_btag_wps=vr_btag_wps,
        )
    )

    return cfg


def DiHiggsAnalysisAddBranches(flags, working_points):

    analysisTreeBranches = []

    if flags.do_resolved_analysis:
        resolvedVars = [
            "nCentralJets",
            "nBtaggedCentralJets",
            "jet1_pt",
            "jet2_pt",
            "jet3_pt",
            "jet4_pt",
            "DeltaR12",
            "DeltaR13",
            "DeltaR14",
            "DeltaR23",
            "DeltaR24",
            "DeltaR34",
            "h1_m",
            "h1_dR_jets",
            "h2_m",
            "h2_dR_jets",
            "hh_m",
        ]
        for btag_wp in working_points["ak4"]:
            for var in resolvedVars:
                analysisTreeBranches += [
                    f"EventInfo.resolved_{var}_{btag_wp}   -> resolved_{btag_wp}_{var}"
                ]

    if flags.do_boosted_analysis:
        boostedVars = [
            "nLargeJets",
            "h1_nGhostAssocVrJets",
            "h1_nBtaggedGhostAssocVrTrackJets",
            "h1_m",
            "h1_jet1_pt",
            "h1_jet2_pt",
            "h1_dR_jets",
            "h2_nGhostAssocVrJets",
            "h2_nBtaggedGhostAssocVrTrackJets",
            "h2_m",
            "h2_jet1_pt",
            "h2_jet2_pt",
            "h2_dR_jets",
            "hh_m",
        ]
        for btag_wp in working_points["vr"]:
            for var in boostedVars:
                analysisTreeBranches += [
                    f"EventInfo.boosted_{var}_{btag_wp}   -> boosted_{btag_wp}_{var}"
                ]

    return analysisTreeBranches
