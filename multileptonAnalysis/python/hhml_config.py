from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, TauSelectorAlgCfg, JetSelectorAlgCfg)


def hhml_cfg(
        flags, smalljetkey, muonkey, electronkey, taukey,
        float_variables=None, float_vector_variables=None,
        int_variables=None, char_vector_variables=None
):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    tightMuonWPs = [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Muon.extra_wps]
    cfg.merge(MuonSelectorAlgCfg(
        flags,
        containerInKey=muonkey,
        containerOutKey="hhmlAnalysisMuons_%SYS%",
        looseMuonWP=MuonWPLabel,
        tightMuonWPs=tightMuonWPs,
        minPt=9 * Units.GeV,
        maxEta=flags.Analysis.Muon.max_eta
    ))

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    tightElectronWP = [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Electron.extra_wps]
    cfg.merge(ElectronSelectorAlgCfg(
        flags,
        containerInKey=electronkey,
        containerOutKey="hhmlAnalysisElectrons_%SYS%",
        looseEleWP=ElectronWPLabel,
        tightEleWPs=tightElectronWP,
        minPt=9 * Units.GeV
    ))

    cfg.merge(TauSelectorAlgCfg(
        flags,
        # Baseline always needed for anti-taus
        containerInKey=taukey,
        containerOutKey="hhmlAnalysisTaus_%SYS%",
        # used to filter collection
        looseTauWP='Baseline',
        # used for subsequent event selections
        # only used to decorate flags + scale factors
        tightTauWPs=[flags.Analysis.Tau.ID],
    ))

    cfg.merge(JetSelectorAlgCfg(
        flags,
        containerInKey=smalljetkey,
        containerOutKey="hhmlAnalysisJets_%SYS%",
        bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
        selectBjet=False,
        minPt=20 * Units.GeV,
        minimumAmount=flags.Analysis.Small_R_jet.amount,  # -1 means ignores this
    ))

    # Selection
    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    cfg.addEventAlgo(
        CompFactory.MULTILEPTON.MultileptonSelectorAlg(
            "HHMLSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            eventDecisionOutputDecoration="hhml_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_hhml_cutflow,
            isMC=flags.Input.isMC,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # calculate final hhml vars
    cfg.addEventAlgo(
        CompFactory.MULTILEPTON.BaselineVarsMultileptonAlg(
            "FinalVarshhmlAlg",
            isMC=flags.Input.isMC,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            leptonAmount=flags.Analysis.Lepton.amount,
            tauAmount=flags.Analysis.Tau.amount,
            jetAmount=flags.Analysis.Small_R_jet.amount,
            lightJetAmount=(flags.Analysis.Small_R_jet.amount
                            - flags.Analysis.Small_R_jet.amount_bjet),
            bJetAmount=flags.Analysis.Small_R_jet.amount_bjet,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            floatVariableList=float_variables,
            floatVectorVariableList=float_vector_variables,
            intVariableList=int_variables,
            charVectorVariableList=char_vector_variables
        )
    )

    return cfg


def get_BaselineVarshhmlAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    int_variable_names += [
        "nTotalJets", "nElectrons", "nMuons", "nTaus",
        "nLightJets", "nBJets", "nCentralJets", "nLeptons",
        "totalLepCharge", "totalTauCharge",
        "subChannelID", "subChannelFlavor",
    ]

    return float_variable_names, int_variable_names


def hhml_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarshhmlAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    float_vector_variable_names = []
    int_variable_names = []
    char_vector_variable_names = []

    # these are the variables that will always be stored by easyjet specific to HHML
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarshhmlAlg_variables(flags)
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
            f"EventInfo.{var}_%SYS% -> hhml_{var}"
            + flags.Analysis.systematics_suffix_separator + "%SYS%"
        ]

    branches += [
        "EventInfo.hhml_pass_sr_%SYS% -> hhml_pass_SR"
        + flags.Analysis.systematics_suffix_separator + "%SYS%"
    ]

    for trigger in ["pass_trigger_SLT",
                    "pass_trigger_DLT",
                    "PASS_TRIGGER",]:
        branches += [f"EventInfo.{trigger}_%SYS% ->"
                     f"hhml_{trigger}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    for ch_cut in ["pass_2lss",
                   "pass_3l",
                   "pass_bb4l",
                   "pass_1l2tauhad",
                   "pass_2l2tauhad",
                   "pass_1l3tauhad",
                   "pass_baseline_tau_trigger"]:
        branches += [f"EventInfo.{ch_cut}_%SYS% ->"
                     f"hhml_{ch_cut}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return (branches, float_variable_names, float_vector_variable_names,
            int_variable_names, char_vector_variable_names)
