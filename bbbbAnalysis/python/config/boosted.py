from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import JetSelectorAlgCfg
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)


def boosted_cfg(flags, largejetkey):
    cfg = ComponentAccumulator()

    for btag_wp in flags.Analysis.Large_R_jet.vr_btag_wps:
        # get the two leading large R's
        cfg.merge(JetSelectorAlgCfg(flags, name="LargeJetSelectorAlg_" + btag_wp,
                                    containerInKey=largejetkey,
                                    containerOutKey="boostedAnalysisJets_" + btag_wp,
                                    bTagWPDecorName="ftag_select_" + btag_wp,
                                    selectBjet=True,
                                    minPt=250e3,
                                    maxEta=2.0,
                                    truncateAtAmount=2,  # -1 means keep all
                                    minimumAmount=2))  # -1 means ignores this

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
        cfg.merge(JetSelectorAlgCfg(
            flags, name="LeadingLargeRVRJetSelectorAlg_" + btag_wp,
            containerInKey="leadingLargeRVRJets_" + btag_wp,
            containerOutKey="SelectedLeadingLargeRVRJets_" + btag_wp,
            bTagWP=btag_wp,
            selectBjet=True,
            minPt=10e3,
            maxEta=2.5,
            truncateAtAmount=3,
            minimumAmount=2,
            removeRelativeDeltaRToVRJet=True))

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
        cfg.merge(JetSelectorAlgCfg(
            flags, name="SubLeadingLargeRVRJetSelectorAlg_" + btag_wp,
            containerInKey="SubLeadingLargeRVRJets_" + btag_wp,
            containerOutKey="SelectedSubLeadingLargeRVRJets_" + btag_wp,
            bTagWP=btag_wp,
            selectBjet=True,
            minPt=10e3,
            maxEta=2.5,
            truncateAtAmount=3,
            minimumAmount=2,
            removeRelativeDeltaRToVRJet=True))

        # calculate final boosted vars
        cfg.addEventAlgo(
            CompFactory.HH4B.BaselineVarsBoostedAlg(
                "FinalVarsBoostedAlg_" + btag_wp,
                largeRContainerInKey="boostedAnalysisJets_" + btag_wp,
                leadingLargeR_GA_VRJets="SelectedLeadingLargeRVRJets_" + btag_wp,
                subLeadingLargeR_GA_VRJets="SelectedSubLeadingLargeRVRJets_" + btag_wp,
                bTagWP=btag_wp,
            )
        )

    return cfg


def boosted_branches(flags):
    branches = []

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    branches += get_selected_objects_branches(flags, "bbbb_boosted")

    for btag_wp in flags.Analysis.Large_R_jet.vr_btag_wps:
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

        for var in boosted_vars:
            branches += [
                f"EventInfo.boosted_{var}_{btag_wp} -> bbbb_boosted_{btag_wp}_{var}"
            ]

    return branches
