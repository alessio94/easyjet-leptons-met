from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)


def resolved_cfg(flags, smalljetkey):
    cfg = ComponentAccumulator()

    # this is a resolved dihiggs analysis chain
    btag_wps = [flags.Analysis.small_R_jet.btag_wp]
    btag_wps += flags.Analysis.small_R_jet.btag_extra_wps
    for btag_wp in btag_wps:
        # get the 4 leading small R jets
        cfg.addEventAlgo(
            CompFactory.Easyjet.JetSelectorAlg(
                "SmallJetSelectorAlg_" + btag_wp,
                containerInKey=smalljetkey,
                containerOutKey="resolvedAnalysisJets_" + btag_wp,
                bTagWPDecorName="ftag_select_" + btag_wp,
                minPt=20e3,
                maxEta=2.5,
                truncateAtAmount=4,  # -1 means keep all
                minimumAmount=4,  # -1 means ignores this
                checkOR=flags.Analysis.do_overlap_removal,
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

    return cfg


def resolved_branches(flags):
    branches = []

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    branches += get_selected_objects_branches(flags, "bbbb_resolved")

    btag_wps = [flags.Analysis.small_R_jet.btag_wp]
    btag_wps += flags.Analysis.small_R_jet.btag_extra_wps
    for btag_wp in btag_wps:
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

        for var in resolved_vars:
            branches += [
                f"EventInfo.resolved_{var}_{btag_wp} -> bbbb_resolved_{btag_wp}_{var}"
            ]

    return branches
