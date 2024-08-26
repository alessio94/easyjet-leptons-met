from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, JetSelectorAlgCfg)

from vbshiggsAnalysis.fullLep_config import fullLep_cfg, fullLep_branches
from vbshiggsAnalysis.semiLep_config import semiLep_cfg, semiLep_branches
from vbshiggsAnalysis.fullHad_config import fullHad_cfg, fullHad_branches

import AthenaCommon.SystemOfUnits as Units


def vbshiggs_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey):

    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    muon_WPs = [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Muon.extra_wps]
    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="vbshiggsAnalysisMuons_%SYS%",
                                 looseMuonWP=MuonWPLabel,
                                 tightMuonWPs=muon_WPs,
                                 minPt=9 * Units.GeV))

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    ele_WPs = [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Electron.extra_wps]
    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="vbshiggsAnalysisElectrons_%SYS%",
                                     looseEleWP=ElectronWPLabel,
                                     tightEleWPs=ele_WPs,
                                     minPt=9 * Units.GeV))

    cfg.merge(JetSelectorAlgCfg(flags, name="SmallJetSelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="vbshiggsAnalysisJets_%SYS%",
                                bTagWPDecorName="",
                                selectBjet=False,
                                minPt=20 * Units.GeV,
                                minimumAmount=2))  # -1 means ignores this

    cfg.merge(JetSelectorAlgCfg(flags,
                                containerInKey=largejetkey,
                                containerOutKey="vbshiggsAnalysisLargeJets_%SYS%",
                                minPt=250 * Units.GeV,
                                maxEta=2.0,
                                minimumAmount=1,  # -1 means ignores this
                                checkOR=flags.Analysis.do_overlap_removal))

    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.VBSJetsSelectorAlg(
            "VBSJetsSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
        )
    )

    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.HiggsSelectorAlg(
            "HiggsSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
        )
    )
    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.TriggerDecoratorAlg(
            "VBSHIGGSTriggerDecoratorAlg",
            muons="vbshiggsAnalysisMuons_%SYS%",
            electrons="vbshiggsAnalysisElectrons_%SYS%",
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            triggerLists=trigger_branches,
        )
    )

    # truth info
    if flags.Input.isMC:
        cfg.addEventAlgo(
            CompFactory.VBSHIGGS.TruthVBSQuarksInfoAlg(
                "TruthVBSQuarksInfoAlg",
                TruthParticleInKey="HardScatterParticles",
            )
        )

    if flags.Analysis.Channel == "FullLep":
        extra_vbshiggs_branches, float_variable_names, \
            int_variable_names = fullLep_branches(flags)
        cfg.merge(
            fullLep_cfg(
                flags,
                float_variables=float_variable_names,
                int_variables=int_variable_names)
        )

    if flags.Analysis.Channel == "SemiLep":
        extra_vbshiggs_branches, float_variable_names, \
            int_variable_names = semiLep_branches(flags)
        cfg.merge(
            semiLep_cfg(
                flags,
                float_variables=float_variable_names,
                int_variables=int_variable_names)
        )

    if flags.Analysis.Channel == "FullHad":
        extra_vbshiggs_branches, float_variable_names, int_variable_names \
            = fullHad_branches(flags)
        cfg.merge(
            fullHad_cfg(
                flags,
                float_variables=float_variable_names,
                int_variables=int_variable_names)
        )
    return cfg
