# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    JetSelectorAlgCfg, TruthMuonSelectorAlgCfg, TruthElectronSelectorAlgCfg,
    TruthLeptonOrderingAlgCfg, TruthJetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def ZQG_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey,
            float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="ZQGAnalysisMuons_%SYS%",
                                 minPt=flags.Analysis.Muon.min_pT_ZQG,
                                 maxEta=flags.Analysis.Muon.max_eta_ZQG
                                 ))

    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="ZQGAnalysisElectrons_%SYS%",
                                     minPt=flags.Analysis.Electron.min_pT_ZQG,
                                     maxEta=2.5
                                     ))

    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(JetSelectorAlgCfg(flags, name="SmallRJet_SelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="ZQGAnalysisJets_%SYS%",
                                bTagWPDecorName="",
                                minPt=flags.Analysis.Small_R_jet.min_pT_ZQG,
                                maxEta=flags.Analysis.Small_R_jet.max_eta_ZQG
                                ))

    cfg.merge(JetSelectorAlgCfg(flags, name="LargeRJet_SelectorAlg",
                                containerInKey=largejetkey,
                                containerOutKey="ZQGAnalysisLargeJets_%SYS%",
                                minPt=250 * Units.GeV,
                                maxEta=2.5,
                                selectBjet=False
                                ))

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    # Selection
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    btag_pcbt_wps \
        = [wp for wp in flags.Analysis.Small_R_jet.btag_extra_wps if "Continuous" in wp]

    # Selection
    cfg.addEventAlgo(
        CompFactory.ZQG.ZQGSelectorAlg(
            "ZQGSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            PCBTDecorList=["ftag_quantile_" + pcbt_wp for pcbt_wp in btag_pcbt_wps],
            eventDecisionOutputDecoration="ZQG_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_cutflow,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    btag_pcbt_wps \
        = [wp for wp in flags.Analysis.Small_R_jet.btag_extra_wps if "Continuous" in wp]

    # calculate final ZQG vars
    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.ZQG.BaselineVarsZQGAlg(
            "BaselineVarsZQGAlg",
            isMC=flags.Input.isMC,
            electrons=electronkey, eleWP=ElectronWPLabel,
            muons=muonkey, muonWP=MuonWPLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            PCBTDecorList=["ftag_quantile_" + pcbt_wp for pcbt_wp in btag_pcbt_wps],
            floatVariableList=float_variables,
            intVariableList=int_variables,
        )
    )

    # include variables for IC reweighting
    if (
        flags.Analysis.Truth.do_IC
        and flags.Input.MCChannelNumber in flags.Analysis.Truth.DSID_ICPDF_Reweight
    ):
        cfg.addEventAlgo(
            CompFactory.CP.PDFReweightAlg(
                "PDFReweightAlg",
                inPDFName="NNPDF31_nnlo_as_0118_luxqed",
                outPDFName=list(flags.Analysis.additionalPdf),
                additionalPdfPath=flags.Analysis.additionalPdfPath
            )
        )

    return cfg


def ZQGTruth_cfg(flags, truthsmalljetkey, truthlargejetkey, truthmuonkey,
                 truthelectronkey, float_variables=None):

    if not float_variables:
        float_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(TruthMuonSelectorAlgCfg(flags,
                                      containerInKey=truthmuonkey,
                                      containerOutKey="ZQGTruthMuons",
                                      minPt=25 * Units.GeV,
                                      maxEta=flags.Analysis.Muon.max_eta_ZQG))

    cfg.merge(TruthElectronSelectorAlgCfg(flags,
                                          containerInKey=truthelectronkey,
                                          containerOutKey="ZQGTruthElectrons",
                                          minPt=25 * Units.GeV,
                                          maxEta=2.5))

    cfg.merge(TruthLeptonOrderingAlgCfg(flags,
                                        containerInTruthElectronKey=truthelectronkey,
                                        containerInTruthMuonKey=truthmuonkey))

    cfg.merge(TruthJetSelectorAlgCfg(flags, name="TruthSmallRJet_SelectorAlg",
                                     containerInKey=truthsmalljetkey,
                                     containerOutKey="ZQGTruthJets",
                                     minPt=15 * Units.GeV,
                                     maxEta=flags.Analysis.Small_R_jet.max_eta_ZQG,
                                     hasTruthLabel=True,
                                     truthLabelDecorName=truthsmalljetkey
                                     + ".HadronConeExclTruthLabelID",
                                     decorOutName="TruthEvents.nSmallJets",
                                     decoration=truthsmalljetkey + ".isTruthJet"))

    cfg.merge(TruthJetSelectorAlgCfg(flags, name="TruthLargeRJet_SelectorAlg",
                                     containerInKey=truthlargejetkey,
                                     containerOutKey="ZQGTruthLargeJets",
                                     minPt=250 * Units.GeV,
                                     maxEta=2.5,
                                     hasTruthLabel=False,
                                     decorOutName="TruthEvents.nLargeJets",
                                     decoration=truthlargejetkey + ".isTruthJet"))

    cfg.addEventAlgo(
        CompFactory.ZQG.ZQGTruthSelectorAlg(
            "ZQGTruthSelectorAlg",
            truthcutList=flags.Analysis.TruthCutList,
            saveCutFlow=flags.Analysis.save_cutflow,
            truthSmallJetInContainer="ZQGTruthJets",
            truthLargeJetInContainer="ZQGTruthLargeJets",
            truthElectronInContainer="ZQGTruthElectrons",
            truthMuonInContainer="ZQGTruthMuons",
            jetTruthFlavourDecoration="ZQGTruthJets.HadronConeExclTruthLabelID",
            floatVariableList=float_variables
        )
    )

    return cfg


def get_BaselineVarsZQGAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for object in ["ll", "jj", "bb", "cc", "LL"]:
        for var in ["m", "pT", "Eta", "Phi", "dR", "dEta", "dPhi"]:
            float_variable_names.append(f"{var}{object}")

    for object in ["Zj", "Zb", "Zc", "ZL"]:
        for var in ["dR", "dPhi"]:
            float_variable_names.append(f"{var}{object}")

    float_variable_names += ["pT_over_mbb", "pT_over_mcc", "pT_over_mLL"]
    int_variable_names += ["nJets", "nBJets", "nCJets", "nLJets", "nLargeRJets"]
    int_variable_names += ["nElectrons", "nMuons", "nLeptons"]
    int_variable_names += ["PCFT2D2bjets", "PCFT2D2cjets", "PCFT2D2Ljets"]

    return float_variable_names, int_variable_names


def get_BaselineVarsZQGAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def get_ZQGTruthSelectorAlg_variables(flags):
    branches = []
    float_variable_names = []

    # C tagged jets

    for var in [*flags.Analysis.Small_R_jet.variables_truthjets]:
        for index in range(flags.Analysis.Small_R_jet.amount_cjet):
            # Store the float variables
            if var in flags.Analysis.Small_R_jet.variables_truthjets:
                float_variable_names += [f"TruthJet_c{index+1}_{var}"]
            branches += [f"TruthEvents.TruthJet_c{index+1}_{var} \
                        -> ZQG_TruthJet_c{index+1}_{var}"]

    # Dilepton
    for var in [*flags.Analysis.Lepton.variables_truth_Z]:
        float_variable_names += [f"Truth_ll_{var}"]
        branches += [f"TruthEvents.Truth_ll_{var} \
                    -> ZQG_Truth_ll_{var}"]

    return branches, float_variable_names


def ZQG_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsZQGAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    truth_float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to ZQG
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsZQGAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsZQGAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> ZQG_{var}_%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "ZQG")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    branches += ["EventInfo.ZQG_pass_sr_%SYS% -> ZQG_pass_SR_%SYS%"]

    if flags.Analysis.save_cutflow:
        cutList = flags.Analysis.CutList + flags.Analysis.Categories
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% \
                        -> ZQG_{cut}_%SYS%"]
            branches += ["EventInfo.PassCuts_%SYS% -> ZQG_PassRecoCuts_%SYS%"]
        if flags.Input.isMC:
            truthcutList = flags.Analysis.TruthCutList + flags.Analysis.TruthCategories
            truth_branches, truth_float_variable_names = (
                get_ZQGTruthSelectorAlg_variables(flags)
            )
            branches += truth_branches
            for cut in truthcutList:
                branches += [f"TruthEvents.{cut} \
                            -> ZQG_{cut}"]
                branches += ["TruthEvents.PassTruthCuts \
                            -> ZQG_PassTruthCuts"]

    # trigger variables do not need to be added to variable_names
    # as it is written out in ZQGSelectorAlg
    for cat in ["SLT"]:
        branches += \
            [f"EventInfo.pass_trigger_{cat}_%SYS% -> ZQG_pass_trigger_{cat}_%SYS%"]

    return (
        branches,
        float_variable_names,
        int_variable_names,
        truth_float_variable_names
    )
