from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import JetSelectorAlgCfg

from VBSVV4qAnalysis.JJ_config import JJ_cfg, JJ_branches
# from VBSVV4qAnalysis.Jjj_config import Jjj_cfg, Jjj_branches
# from VBSVV4qAnalysis.jjjj_config import jjjj_cfg, jjjj_branches

import AthenaCommon.SystemOfUnits as Units


def VBSVV4q_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey):

    cfg = ComponentAccumulator()

    cfg.merge(JetSelectorAlgCfg(flags, name="SmallJetSelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="VBSVV4qAnalysisJets_%SYS%",
                                bTagWPDecorName="",
                                selectBjet=False,
                                minPt=20 * Units.GeV,
                                minimumAmount=1))  # -1 means ignores this

    cfg.merge(JetSelectorAlgCfg(flags,
                                containerInKey=largejetkey,
                                containerOutKey="VBSVV4qAnalysisLargeJets_%SYS%",
                                minPt=200 * Units.GeV,
                                maxEta=2.0,
                                minimumAmount=1))  # -1 means ignores this

    # trigger
    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    trigger_branches = [
        f"trigPassed_{c}"
        for c in flags.Analysis.TriggerChains
    ]

    cfg.addEventAlgo(
        CompFactory.VBSVV4q.TriggerDecoratorAlg(
            "VBSVV4qTriggerDecoratorAlg",
            jets="VBSVV4qAnalysisLargeJets_%SYS%",
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            triggerLists=trigger_branches,
        )
    )

    # boson tagger
    if flags.Analysis.loadDisCoJet:
        VTagger = CompFactory.JSSTaggerUtils(
            "JSSTaggerUtils/DisCoJetTagger",
            ContainerName=flags.Analysis.container_names.input.reco10UFOJet,
            CalibArea="Local",
            ConfigFile="/VBSVV4qAnalysis/" + flags.Analysis.DisCoJetCfg
        )

        cfg.addEventAlgo(
            CompFactory.VBSVV4q.BosonTaggerAlg(
                "BosonTaggerAlg",
                LargeRJets=flags.Analysis.container_names.input.reco10UFOJet,
                MLTagger=VTagger
            )
        )

    for channel in flags.Analysis.channels:
        if channel == "JJ":
            extra_VBSVV4q_branches, float_variable_names, \
                int_variable_names = JJ_branches(flags)
            cfg.merge(
                JJ_cfg(
                    flags,
                    float_variables=float_variable_names,
                    int_variables=int_variable_names)
            )

        """
        if channel == "Jjj":
            extra_VBSVV4q_branches, float_variable_names, \
                int_variable_names = Jjj_branches(flags)
            cfg.merge(
                Jjj_cfg(
                    flags,
                    float_variables=float_variable_names,
                    int_variables=int_variable_names)
            )

        if channel == "jjjj":
            extra_VBSVV4q_branches, float_variable_names, int_variable_names \
                = jjjj_branches(flags)
            cfg.merge(
                jjjj_cfg(
                    flags,
                    float_variables=float_variable_names,
                    int_variables=int_variable_names)
            )
        """

    return cfg
