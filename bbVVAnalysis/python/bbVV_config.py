from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)


def bbVV_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    TightMuonWP = flags.Analysis.Muon.extra_wps[0]
    TightMuonWPLabel = f'{TightMuonWP[0]}_{TightMuonWP[1]}'
    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="bbVVAnalysisMuons_%SYS%",
                                 looseMuonWP=MuonWPLabel,
                                 tightMuonWPs=[TightMuonWPLabel]))

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    TightEleWP = flags.Analysis.Electron.extra_wps[0]
    TightEleWPLabel = f'{TightEleWP[0]}_{TightEleWP[1]}'
    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="bbVVAnalysisElectrons_%SYS%",
                                     looseEleWP=ElectronWPLabel,
                                     tightEleWPs=[TightEleWPLabel]))

    cfg.merge(JetSelectorAlgCfg(flags, name="SmallJetSelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="bbVVAnalysisJets_%SYS%",
                                bTagWPDecorName="",  # empty string: "" ignores btagging
                                selectBjet=False,
                                maxEta=2.5,
                                truncateAtAmount=2,  # -1 means keep all
                                minimumAmount=2))  # -1 means ignores this

    cfg.merge(JetSelectorAlgCfg(flags, name="LargeJetSelectorAlg",
                                containerInKey=largejetkey,
                                containerOutKey="bbVVAnalysisLRJets_%SYS%",
                                bTagWPDecorName="",
                                selectBjet=False,
                                minPt=200e3,
                                maxEta=2.0,
                                truncateAtAmount=3,
                                minimumAmount=2))

    # Selection
    cfg.addEventAlgo(
        CompFactory.HHBBVV.HHbbVVSelectorAlg(
            "HHbbVVSelectorAlg",
            eventDecisionOutputDecoration="bbVV_pass_sr_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            muonWP=TightMuonWPLabel,
            eleWP=TightEleWPLabel,
            channel=flags.Analysis.channels,
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # calculate final bbVV vars
    cfg.addEventAlgo(
        CompFactory.HHBBVV.BaselineVarsbbVVAlg(
            "FinalVarsbbVVAlg",
            isMC=flags.Input.isMC,
            muonWP=TightMuonWPLabel,
            eleWP=TightEleWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
        )
    )

    return cfg


def bbVV_branches(flags):
    branches = []

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    branches += get_selected_objects_branches(flags, "bbVV")

    branches += ["EventInfo.bbVV_pass_sr_%SYS% -> bbVV_pass_SR"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches
