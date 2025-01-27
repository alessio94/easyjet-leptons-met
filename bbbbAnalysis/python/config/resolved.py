from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import JetSelectorAlgCfg
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)


def resolved_cfg(flags, smalljetkey):
    cfg = ComponentAccumulator()

    # This is a jet trigger scale factor block
    if (flags.Input.isMC
            and (flags.Analysis.Small_R_jet.doHLTMatching
                 or flags.Analysis.Small_R_jet.doL1Matching)):
        cfg.merge(
            JetSelectorAlgCfg(flags, name="SmallJetPreSelectorAlg",
                              containerInKey=smalljetkey.replace("%SYS%", "NOSYS"),
                              containerOutKey="smallRJetsForTriggerMatching",
                              minPt=20 * Units.GeV,
                              maxEta=2.4))
        if flags.Analysis.Small_R_jet.doL1Matching:
            cfg.addEventAlgo(
                CompFactory.HH4B.SmallRJetTriggerSFAlg(
                    "SmallRJetL1SFAlg",
                    containerInKey="smallRJetsForTriggerMatching",
                    containerOutKey="trigL1MatchedSmallRJets",
                    triggers=flags.Analysis.TriggerChains,
                    years=flags.Analysis.Years,
                    matchingLevel="L1",
                    doL1SF=flags.Analysis.Small_R_jet.doL1Matching,
                    doHLTSF=flags.Analysis.Small_R_jet.doHLTMatching
                )
            )
        if flags.Analysis.Small_R_jet.doHLTMatching:
            cfg.addEventAlgo(
                CompFactory.HH4B.SmallRJetTriggerSFAlg(
                    "SmallRJetHLTSFAlg",
                    containerInKey="smallRJetsForTriggerMatching",
                    containerOutKey="trigHLTMatchedSmallRJets",
                    triggers=flags.Analysis.TriggerChains,
                    years=flags.Analysis.Years,
                    matchingLevel="HLT",
                    doL1SF=flags.Analysis.Small_R_jet.doL1Matching,
                    doHLTSF=flags.Analysis.Small_R_jet.doHLTMatching
                )
            )

    # this is a resolved dihiggs analysis chain
    btag_wps = [flags.Analysis.Small_R_jet.btag_wp]
    btag_wps += flags.Analysis.Small_R_jet.btag_extra_wps
    for btag_wp in btag_wps:
        btag_sys = btag_wp + "_%SYS%"
        # get the 4 leading small R jets
        cfg.merge(JetSelectorAlgCfg(flags, name="SmallJetSelectorAlg_" + btag_wp,
                                    containerInKey=smalljetkey,
                                    containerOutKey="resolvedAnalysisJets_" + btag_sys,
                                    bTagWPDecorName="ftag_select_" + btag_wp,
                                    selectBjet=True,
                                    minPt=20 * Units.GeV,
                                    maxEta=2.5,
                                    truncateAtAmount=4,  # -1 means keep all
                                    minimumAmount=4))  # -1 means ignores this

        # pair them with some strategy and save them as leading (h1) and
        # subleading (h2) Higgs candidates in the order:
        # h1_leading_pt_jet
        # h1_subleading_pt_jet
        # h2_leading_pt_jet
        # h2_subleading_pt_jet
        cfg.addEventAlgo(
            CompFactory.HH4B.JetPairingAlg(
                "JetPairingAlg_" + btag_wp,
                containerInKey="resolvedAnalysisJets_" + btag_sys,
                containerOutKey="pairedResolvedAnalysisJets_" + btag_sys,
                pairingStrategy=flags.Analysis.pairingStrategy,  # minDeltaR, BDT
                kinematicGroup=flags.Analysis.kinematicGroup,
            )
        )

        # calculate final resolved vars
        cfg.addEventAlgo(
            CompFactory.HH4B.BaselineVarsResolvedAlg(
                "FinalVarsResolvedAlg_" + btag_wp,
                smallRContainerInKey="pairedResolvedAnalysisJets_" + btag_sys,
                bTagWP=btag_wp,
            )
        )

        # calculate trigger buckets
        cfg.addEventAlgo(
            CompFactory.HH4B.TriggerDecoratorAlg(
                "HH4bTriggerDecoratorAlg",
                jets=flags.Analysis.container_names.input.reco4PFlowJet,
                triggerLists=flags.Analysis.TriggerChains,
            )
        )
    return cfg


def resolved_branches(flags):
    branches = []

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    branches += get_selected_objects_branches(flags, "bbbb_resolved")

    btag_wps = [flags.Analysis.Small_R_jet.btag_wp]
    btag_wps += flags.Analysis.Small_R_jet.btag_extra_wps
    for btag_wp in btag_wps:
        resolved_vars = ["hh_m", "hh_pt", "DeltaEtaHH"]
        for part in ["h1", "h2"]:
            for var in ["m", "pt", "eta", "phi"]:
                resolved_vars += [part + "_" + var]

        if flags.Analysis.pairingStrategy == "minDeltaR":
            resolved_vars += [
                "DeltaR12",
                "DeltaR13",
                "DeltaR14",
                "DeltaR23",
                "DeltaR24",
                "DeltaR34",
            ]

        for var in resolved_vars:
            deco_suffix = ""
            pairing_strategy = flags.Analysis.pairingStrategy
            if pairing_strategy == "minDeltaR":
                deco_suffix = "mindrPaired_"
            elif pairing_strategy == "BDT":
                deco_suffix = "bdtPaired_"
            else:
                print("Pairing method not specified or incorrect")

            branches += [
                f"EventInfo.resolved_{var}_{btag_wp}_%SYS%"
                + f" -> bbbb_resolved_{deco_suffix}{btag_wp}_{var}"
                + flags.Analysis.systematics_suffix_separator + "%SYS%"
            ]

    if flags.Input.isMC:
        # add trigger scale factor output
        for trig in flags.Analysis.TriggerChains:
            trig = trig.replace("-", "_").replace(".", "p")
            matchLevels = []
            if flags.Analysis.Small_R_jet.doL1Matching:
                matchLevels.append("L1")
            if flags.Analysis.Small_R_jet.doHLTMatching:
                matchLevels.append("HLT")
            for matchLevel in matchLevels:
                branches += [
                    f'EventInfo.trigSF_{trig}_{matchLevel}SF'
                    f'->trigSF_{trig}_{matchLevel}SF',
                    f'EventInfo.trigSF_{trig}_{matchLevel}SF_stats_1up'
                    f'->trigSF_{trig}_{matchLevel}SF_stats_1up',
                    f'EventInfo.trigSF_{trig}_{matchLevel}SF_syst_1up'
                    f'->trigSF_{trig}_{matchLevel}SF_syst_1up',
                ]
                # more jet-level info for validation
                if flags.Analysis.Small_R_jet.saveTriggerInfo:
                    jet_coll = f'trig{matchLevel}MatchedSmallRJets'
                    branches += [
                        f'{jet_coll}.NoBJetCalibMomentum_pt'
                        f'->{jet_coll}_NoBJetCalibMomentum_pt',
                        f'{jet_coll}.{trig}_{matchLevel}threshold_NOSYS'
                        f'->{jet_coll}_{trig}_{matchLevel}threshold',
                        f'{jet_coll}.{trig}_{matchLevel}SF_NOSYS'
                        f'->{jet_coll}_{trig}_{matchLevel}SF',
                        f'{jet_coll}.{trig}_{matchLevel}SF_stats_1up'
                        f'->{jet_coll}_{trig}_{matchLevel}SF_stats_1up',
                        f'{jet_coll}.{trig}_{matchLevel}SF_syst_1up'
                        f'->{jet_coll}_{trig}_{matchLevel}SF_syst_1up',
                    ]

    return branches
