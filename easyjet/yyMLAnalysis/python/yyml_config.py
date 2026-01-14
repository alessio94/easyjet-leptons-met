from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg
from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    PhotonSelectorAlgCfg, ElectronSelectorAlgCfg, MuonSelectorAlgCfg,
    LeptonOrderingAlgCfg, JetSelectorAlgCfg, TauSelectorAlgCfg)


def yyml_cfg(
        flags, photonkey, electronkey, muonkey, smalljetkey, taukey,
        float_variables=None, float_vector_variables=None,
        int_variables=None, char_vector_variables=None
):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(PhotonSelectorAlgCfg(flags,
                                   containerInKey=photonkey,
                                   containerOutKey="yymlAnalysisPhotons_%SYS%",
                                   minPt=flags.Analysis.Photon.min_pT * Units.MeV
                                   ))

    cfg.merge(ElectronSelectorAlgCfg(
        flags,
        containerInKey=electronkey,
        containerOutKey="yymlAnalysisElectrons_%SYS%",
        minPt=flags.Analysis.Electron.min_pT * Units.MeV
    ))

    cfg.merge(MuonSelectorAlgCfg(
        flags,
        containerInKey=muonkey,
        containerOutKey="yymlAnalysisMuons_%SYS%",
        minPt=flags.Analysis.Muon.min_pT * Units.MeV
    ))

    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(JetSelectorAlgCfg(
        flags,
        containerInKey=smalljetkey,
        containerOutKey="yymlAnalysisJets_%SYS%",
        bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
        selectBjet=False,
        minPt=flags.Analysis.Small_R_jet.min_pT * Units.MeV
    ))

    cfg.merge(TauSelectorAlgCfg(
        flags,
        containerInKey=taukey,
        containerOutKey="yymlAnalysisTaus_%SYS%"
    ))

    # Selection
    # Taken care of internally in yymlSelectorAlg:
    # from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg
    # trigger_branches = [
    #     f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
    #     for c in flags.Analysis.TriggerChains
    # ]

    cfg.addEventAlgo(
        CompFactory.HHYYML.yymlSelectorAlg(
            "yymlSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            eventDecisionOutputDecoration="yyml_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_cutflow,
            isMC=flags.Input.isMC,
            photonTriggers=flags.Analysis.TriggerChains,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # calculate final yyml vars
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.HHYYML.BaselineVarsyymlAlg(
            "BaselineVarsyymlAlg",
            isMC=flags.Input.isMC,
            eleWP=ElectronWPLabel,
            muonWP=MuonWPLabel,
            leptonAmount=flags.Analysis.Lepton.amount,
            jetAmount=flags.Analysis.Small_R_jet.amount,
            lightJetAmount=(flags.Analysis.Small_R_jet.amount
                            - flags.Analysis.Small_R_jet.amount_bjet),
            bJetAmount=flags.Analysis.Small_R_jet.amount_bjet,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            tauAmount=flags.Analysis.Tau.amount,
            floatVariableList=float_variables,
            floatVectorVariableList=float_vector_variables,
            intVariableList=int_variables,
            charVectorVariableList=char_vector_variables
        )
    )

    return cfg


def get_BaselineVarsyymlAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    int_variable_names += [
        "nTotalJets", "nElectrons", "nMuons", "nTaus",
        "nLightJets", "nBJets", "nCentralJets", "nLeptons",
        "totalLepCharge", "totalTauCharge",
        "subChannelID", "subChannelFlavor",
    ]

    return float_variable_names, int_variable_names


def yyml_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsyymlAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    float_vector_variable_names = []
    int_variable_names = []
    char_vector_variable_names = []

    # these are the variables that will always be stored by easyjet specific to HHyyml
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsyymlAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    all_baseline_variable_names += [
        *float_variable_names,
        *float_vector_variable_names,
        *int_variable_names,
        *char_vector_variable_names
    ]

    for var in all_baseline_variable_names:
        branches += [
            f"EventInfo.{var}_%SYS% -> yyml_{var}"
            + flags.Analysis.systematics_suffix_separator + "%SYS%"
        ]

    branches += [
        "EventInfo.yyml_pass_sr_%SYS% -> yyml_pass_SR"
        + flags.Analysis.systematics_suffix_separator + "%SYS%"
    ]

    for trigger in ["pass_trigger_diphoton",
                    "pass_matching_trigger_diphoton",
                    "PASS_TRIGGER",]:
        branches += [f"EventInfo.{trigger}_%SYS% ->"
                     f"yyml_{trigger}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    for ch_cut in ["pass_1l0tau",
                   "pass_0l1tau",
                   "pass_2l0tau",
                   "pass_1l1tau",
                   "pass_0l2tau"]:
        branches += [f"EventInfo.{ch_cut}_%SYS% ->"
                     f"yyml_{ch_cut}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return (branches, float_variable_names, float_vector_variable_names,
            int_variable_names, char_vector_variable_names)
