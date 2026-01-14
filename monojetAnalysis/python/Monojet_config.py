from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import \
    (MuonSelectorAlgCfg, ElectronSelectorAlgCfg, TauSelectorAlgCfg,
     LeptonOrderingAlgCfg, JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def Monojet_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey,
                taukey, float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="MonojetAnalysisMuons_%SYS%",
                                 minPt=flags.Analysis.Muon.min_pT,
                                 maxEta=flags.Analysis.Muon.max_eta))

    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="MonojetAnalysisElectrons_%SYS%",
                                     minPt=flags.Analysis.Electron.min_pT,
                                     maxEta=flags.Analysis.Electron.max_eta))

    cfg.merge(TauSelectorAlgCfg(flags,
                                containerInKey=taukey,
                                containerOutKey="MonojetAnalysisTaus_%SYS%"))

    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(JetSelectorAlgCfg(flags,
                                containerInKey=smalljetkey,
                                containerOutKey="MonojetAnalysisJets_%SYS%",
                                bTagWPDecorName="",
                                minPt=flags.Analysis.Small_R_jet.min_pT,
                                maxEta=flags.Analysis.Small_R_jet.max_eta))

    cfg.merge(JetSelectorAlgCfg(flags, name="LargeRJet_SelectorAlg",
                                containerInKey=largejetkey,
                                containerOutKey="MonojetAnalysisLargeJets_%SYS%",
                                minPt=flags.Analysis.Large_R_jet.min_pT * Units.GeV,
                                maxEta=flags.Analysis.Large_R_jet.max_eta,
                                minMass=flags.Analysis.Large_R_jet.min_m * Units.GeV,
                                maxMass=flags.Analysis.Large_R_jet.max_m * Units.GeV,
                                selectBjet=False))

    # Selection
    trigger_branches = [
        f"{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    btag_pcbt_wps = [wp for wp in flags.Analysis.Small_R_jet.btag_extra_wps
                     if "Continuous" in wp]

    # Selection
    cfg.addEventAlgo(
        CompFactory.MONOJET.MonojetSelectorAlg(
            "MonojetSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp
                            if flags.Analysis.Small_R_jet.btag_wp
            else "",
            PCBTDecorList=["ftag_quantile_" + pcbt_wp for pcbt_wp in btag_pcbt_wps],
            eventDecisionOutputDecoration="Monojet_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_Monojet_cutflow,
            isMC=flags.Input.isMC,
            triggerLists=trigger_branches,
            met_cut=flags.Analysis.met_cut,
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    btag_pcbt_wps = [wp for wp in flags.Analysis.Small_R_jet.btag_extra_wps
                     if "Continuous" in wp]

    # calculate final Monojet vars
    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    TauWPLabel = f'{flags.Analysis.Tau.ID}'
    cfg.addEventAlgo(
        CompFactory.MONOJET.BaselineVarsMonojetAlg(
            "FinalVarsMonojetAlg",
            isMC=flags.Input.isMC,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            tauWP=TauWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            PCBTDecorName=["ftag_quantile_" + pcbt_wp for pcbt_wp in btag_pcbt_wps][0],
            btagWPs=flags.Analysis.Small_R_jet.monojet_btagWPs,
            floatVariableList=float_variables,
            intVariableList=int_variables,
            Small_R_jet_amount=flags.Analysis.Small_R_jet.amount,
            Large_R_jet_amount=flags.Analysis.Large_R_jet.amount,
            max_eta_central_jet=flags.Analysis.Small_R_jet.max_eta_central
        )
    )

    return cfg


def get_BaselineVarsMonojetAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []
    float_variable_names += ["METSig", "sum_pT_CentralJets", "sum_pT_ForwardJets",
                             "sum_pT_LargeRJets", "DeltaPhi_MET_jets",
                             "DeltaPhi_MET_largeJets"]
    int_variable_names += ["nCentralJets", "nForwardJets", "nLargeRJets",
                           "nTightBadJets", "TIGHTBAD_event"]

    for wp in flags.Analysis.Small_R_jet.monojet_btagWPs:
        int_variable_names += ["nBJets" + wp]

    return float_variable_names, int_variable_names


def get_BaselineVarsMonojetAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def Monojet_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsMonojetAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to Monojet
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables = \
        get_BaselineVarsMonojetAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsMonojetAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> Monojet_{var}_%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "Monojet")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    if (flags.Analysis.save_Monojet_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> Monojet_{cut}_%SYS%"]

    return branches, float_variable_names, int_variable_names
