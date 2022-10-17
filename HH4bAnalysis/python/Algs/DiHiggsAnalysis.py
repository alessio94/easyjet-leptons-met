from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# from HH4bAnalysis.utils.containerNameHelper import get_container_names


def DiHiggsAnalysisAlgCfg(
    flags,
    SmallJetKey,
    LargeJetKey,
):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.DiHiggsAnalysisAlg(
            "DiHiggsAnalysis",
            EventInfoKey="EventInfo",
            SmallJetKey=SmallJetKey,
            LargeJetKey=LargeJetKey,
            doResolvedAnalysis=flags.Analysis.do_resolved_analysis,
            doBoostedAnalysis=flags.Analysis.do_boosted_analysis,
            btag_wps=flags.Analysis.btag_wps,
            vr_btag_wps=flags.Analysis.vr_btag_wps,
            isMC=flags.Input.isMC,
        )
    )

    return cfg


def DiHiggsAnalysisAddBranches(flags):
    analysisTreeBranches = []

    # we will do this once the config is merged in
    # containers = get_container_names(flags, disable_calib)["outputs"]
    if flags.Input.isMC:
        analysisTreeBranches += [
            "AnalysisAntiKt4EMPFlowJets_%SYS%.dRtoTruthBs -> dRtoTruthBs_%SYS%"
        ]

    if flags.Analysis.do_resolved_analysis:
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
    if flags.Input.isMC:
        resolvedVars += [
            "h1_fromSameInitialParticle",
            "h2_fromSameInitialParticle",
            "h1_dR_leadingJet_closestB",
            "h2_dR_leadingJet_closestB",
            "h1_dR_subleadingJet_closestB",
            "h2_dR_subleadingJet_closestB",
        ]

    for btag_wp in flags.Analysis.btag_wps:
        for var in resolvedVars:
            analysisTreeBranches += [
                f"EventInfo.resolved_{var}_{btag_wp}   -> resolved_{btag_wp}_{var}"
            ]

    if flags.Analysis.do_boosted_analysis:
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
        for btag_wp in flags.Analysis.vr_btag_wps:
            for var in boostedVars:
                analysisTreeBranches += [
                    f"EventInfo.boosted_{var}_{btag_wp}   -> boosted_{btag_wp}_{var}"
                ]

    ### new implementation ####
    for btag_wp in flags.Analysis.btag_wps:
        vars = [
            f"resolvedAnalysisJets_{btag_wp}_pt",
            f"resolvedAnalysisJets_{btag_wp}_eta",
            f"resolvedAnalysisJets_{btag_wp}_phi",
            f"resolvedAnalysisJets_{btag_wp}_m",
            f"resolvedAnalysisJets_{btag_wp}_isCentral",
            f"resolvedAnalysisJets_{btag_wp}_n",
        ]
        for var in vars:
            analysisTreeBranches += [f"EventInfo.{var} -> {var}"]

    return analysisTreeBranches


def DiHiggsAnalysisChainCfg(flags, SmallJetKey, LargeJetKey):
    cfg = ComponentAccumulator()

    for btag_wp in flags.Analysis.btag_wps:
        cfg.addEventAlgo(
            CompFactory.HH4B.JetSelectorAlg(
                "JetSelectorAlg_" + btag_wp,
                containerInKey=SmallJetKey,
                containerOutKey="resolvedAnalysisJets_" + btag_wp,
                bTagWP=btag_wp,  # empty string: "" ignores btagging
                minPt=20_000,
                maxEta=2.5,
                howManyToKeep=4,  # -1 means keep all
                pTsort=True,
            )
        )
    return cfg
