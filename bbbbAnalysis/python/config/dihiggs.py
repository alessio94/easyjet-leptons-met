from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from EasyjetHub.algs.event_counter_config import event_counter_cfg
from bbbbAnalysis.config.boosted import boosted_cfg
from bbbbAnalysis.config.resolved import resolved_cfg
from bbbbAnalysis.config.boost_histograms import histograms_cfg
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import JetSelectorAlgCfg
import AthenaCommon.SystemOfUnits as Units


def dihiggs_cfg(
    flags,
):
    cfg = ComponentAccumulator()

    couting_jet_container = ""
    btag_name = ""
    if flags.Analysis.do_small_R_jets:
        smalljetkey = flags.Analysis.container_names.output.reco4PFlowJet
        couting_jet_container = "resolvedAnalysisSmallRJets_%SYS%"
        if flags.Analysis.Small_R_jet.btag_wp != "":
            btag_name = f"ftag_select_{flags.Analysis.Small_R_jet.btag_wp}"
        # Used only for counting
        cfg.merge(
            JetSelectorAlgCfg(
                flags, name="SmallRJetSelectorAlg",
                containerInKey=smalljetkey,
                containerOutKey=couting_jet_container,
                bTagWPDecorName=btag_name,
                selectBjet=False,
                minPt=20. * Units.GeV,
                maxEta=2.5,
                minimumAmount=4,
            )
        )  # -1 means ignores this

    couting_lRjet_container = ""
    if flags.Analysis.do_large_R_UFO_jets:
        lr10jets_sys = flags.Analysis.container_names.output.reco10UFOJet
        couting_lRjet_container = "boostedAnalysisLargeRJets_%SYS%"
        cfg.merge(
            JetSelectorAlgCfg(
                flags, name="LargeRJetSelectorAlg",
                containerInKey=lr10jets_sys,
                containerOutKey=couting_lRjet_container,
                bTagWPDecorName="",
                selectBjet=False,
                minPt=200. * Units.GeV,
                maxPt=3000. * Units.GeV,
                maxMass=600. * Units.GeV,
                maxEta=2.0,
                jetAmount=2,
            )
        )  # -1 means ignores this

    selection_name = flags.Analysis.selection_name
    triggers = flags.Analysis.TriggerChains
    triggers = [trigger.replace(".", "p").replace("-", "_") for trigger in triggers]

    if len(flags.Analysis.CutList) > 0:
        cfg.addEventAlgo(
            CompFactory.HH4B.bbbbSelectorAlg(
                "bbbbSelectorAlg",
                cutList=flags.Analysis.CutList,
                saveCutFlow=flags.Analysis.save_bbbb_cutflow,
                Triggers=triggers,
                eventDecisionOutputDecoration=f"pass_{selection_name}_%SYS%",
                isMC=flags.Input.isMC,
                bypass=flags.Analysis.bypass,
                smallRJetToCount=couting_jet_container,
                largeRJetToCount=couting_lRjet_container,
                btagSelDecor=btag_name,
            )
        )

    if flags.Analysis.do_resolved_dihiggs:
        cfg.merge(
            resolved_cfg(
                flags,
                smalljetkey=smalljetkey,
            )
        )
        cfg.merge(event_counter_cfg("n_resolved"))

    if flags.Analysis.do_boosted_dihiggs:
        cfg.merge(
            boosted_cfg(
                flags,
                largejetkey=lr10jets_sys,
            )
        )
        cfg.merge(event_counter_cfg("n_merged"))

    if flags.Analysis.histograms:
        cfg.merge(histograms_cfg(flags))

    return cfg


def pass_branches(flags):
    branches = []

    branches += get_selected_objects_branches(flags, "bbbb_pass")

    selection_name = flags.Analysis.selection_name
    branches += [
        f"EventInfo.pass_{selection_name}_%SYS%"
        + f" -> bbbb_pass_{selection_name}"
        + flags.Analysis.systematics_suffix_separator + "%SYS%"
    ]

    if (flags.Analysis.save_bbbb_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS%"
                         + f" -> bbbb_pass_{cut}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches
