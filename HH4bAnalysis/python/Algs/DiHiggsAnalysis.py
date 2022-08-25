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
    vars = [
        "m_h1",
        "m_h2",
        "m_hh",
        "dR_jets_in_h1",
        "dR_jets_in_h2",
    ]

    if flags.do_resolved_analysis:
        for btag_wp in working_points["ak4"]:
            for var in vars:
                analysisTreeBranches += [
                    f"EventInfo.resolved_{var}_{btag_wp}   -> resolved_{btag_wp}_{var}"
                ]
    if flags.do_boosted_analysis:
        vars += [
            "h1_nGhostAssocVrJets",
            "h1_nBtaggedGhostAssocVrTrackJets",
            "h2_nGhostAssocVrJets",
            "h2_nBtaggedGhostAssocVrTrackJets",
        ]
        for btag_wp in working_points["vr"]:
            for var in vars:
                analysisTreeBranches += [
                    f"EventInfo.boosted_{var}_{btag_wp}   -> boosted_{btag_wp}_{var}"
                ]

    return analysisTreeBranches
