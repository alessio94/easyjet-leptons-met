from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    TauSelectorAlgCfg, JetSelectorAlgCfg)


def hhml_cfg(
        flags, smalljetkey, muonkey, electronkey, taukey,
        float_variables=None, float_vector_variables=None,
        int_variables=None, char_vector_variables=None, save_extrabb4l_vars=False
):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(MuonSelectorAlgCfg(
        flags,
        containerInKey=muonkey,
        containerOutKey="hhmlAnalysisMuons_%SYS%",
        minPt=flags.Analysis.Muon.min_pT * Units.MeV
    ))

    cfg.merge(ElectronSelectorAlgCfg(
        flags,
        containerInKey=electronkey,
        containerOutKey="hhmlAnalysisElectrons_%SYS%",
        minPt=flags.Analysis.Electron.min_pT * Units.MeV
    ))

    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(TauSelectorAlgCfg(
        flags,
        containerInKey=taukey,
        containerOutKey="hhmlAnalysisTaus_%SYS%"
    ))

    cfg.merge(JetSelectorAlgCfg(
        flags,
        containerInKey=smalljetkey,
        containerOutKey="hhmlAnalysisJets_%SYS%",
        bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
        selectBjet=False,
        minPt=flags.Analysis.Small_R_jet.min_pT * Units.MeV,
    ))

    # Kinematic Fit
    from KinematicFitTool.KinematicFit_config import KinematicFitTool_bb4l_Cfg
    if flags.Analysis.do_KinematicFit:
        kinematic_fit_tool = cfg.popToolsAndMerge(
            KinematicFitTool_bb4l_Cfg(
                flags,
                JetMinPt=flags.Analysis.Small_R_jet.min_pT,
                bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp
            )
        )
        cfg.addEventAlgo(
            CompFactory.MULTILEPTON.MbbKinFitDecoratorAlg(
                "MbbKinFitDecoratorAlg",
                KinFitTool=kinematic_fit_tool,
                doSystematics=flags.Analysis.do_CP_systematics,
            )
        )

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
            saveCutFlow=flags.Analysis.save_cutflow,
            isMC=flags.Input.isMC,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # calculate final hhml vars
    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.MULTILEPTON.BaselineVarsMultileptonAlg(
            "FinalVarshhmlAlg",
            isMC=flags.Input.isMC,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            leptonAmount=flags.Analysis.Lepton.amount,
            doKF=flags.Analysis.do_KinematicFit,
            tauAmount=flags.Analysis.Tau.amount,
            jetAmount=flags.Analysis.Small_R_jet.amount,
            lightJetAmount=(flags.Analysis.Small_R_jet.amount
                            - flags.Analysis.Small_R_jet.amount_bjet),
            bJetAmount=flags.Analysis.Small_R_jet.amount_bjet,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            floatVariableList=float_variables,
            floatVectorVariableList=float_vector_variables,
            intVariableList=int_variables,
            charVectorVariableList=char_vector_variables,
            save_extrabb4l_vars=flags.Analysis.save_extrabb4l_vars
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

    if flags.Analysis.save_extrabb4l_vars:
        HbbCandidate_float_vars = ["pt", "phi", "eta", "E"]
        HbbCandidate_float_vars += ["uncorrPt", "muonCorrPt"]
        for i in range(1, 3):
            for var in HbbCandidate_float_vars:
                float_variable_names += [f"HbbCandidate_Jet{i}_" + var]
            for var in ["n_muons"]:
                int_variable_names += [f"HbbCandidate_Jet{i}_" + var]

        for var in [
            "m_bb",
            "pt_bb",
            "eta_bb",
            "phi_bb",
            "dR_bb",
            "Jet1_pt",
            "Jet1_eta",
            "Jet1_phi",
            "Jet1_m",
            "Jet2_pt",
            "Jet2_eta",
            "Jet2_phi",
                "Jet2_m"]:
            float_variable_names += ["HbbCand_uncorr_" + var,
                                     "HbbCand_muonCorr_" + var,
                                     "HbbCand_PtCorr_" + var,
                                     "HbbCand_KF_" + var]

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

    if flags.Analysis.do_KinematicFit:
        # do not append KF_mbb variables to float_variable_names['baseline']
        # as they are stored by the KF algorithm not BaselineVarsbb4lAlg
        all_baseline_variable_names += ["KF_mbb"]

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

    for ch_cut in ["pass_2lsc",
                   "pass_3l",
                   "pass_bb4l",
                   "pass_1l2tau",
                   "pass_2l2tau",
                   "pass_1l3tau",
                   "pass_3l1tau",
                   "pass_2lsc1tau",
                   "pass_baseline_tau_trigger"]:
        branches += [f"EventInfo.{ch_cut}_%SYS% ->"
                     f"hhml_{ch_cut}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return (branches, float_variable_names, float_vector_variable_names,
            int_variable_names, char_vector_variable_names)
