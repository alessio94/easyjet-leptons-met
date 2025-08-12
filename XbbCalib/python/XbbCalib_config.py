from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def XbbCalib_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey,
                 float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(JetSelectorAlgCfg(flags,
                                name="LargeJetSelectorAlg",
                                containerInKey=largejetkey,
                                containerOutKey="XbbCalibLRJets_%SYS%",
                                minPt=flags.Analysis.Large_R_jet.min_pT,
                                maxEta=flags.Analysis.Large_R_jet.max_eta,
                                minimumAmount=1,
                                ))

    cfg.merge(JetSelectorAlgCfg(flags,
                                name="SmallRJet_SelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="XbbCalibJets_%SYS%",
                                minPt=flags.Analysis.Small_R_jet.min_pT,
                                maxEta=2.5,
                                minimumAmount=1,
                                bTagWPDecorName="ftag_select_"
                                + flags.Analysis.Small_R_jet.btag_wp,
                                selectBjet=True,
                                ))

    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="XbbCalibElectrons_%SYS%",
                                     minPt=flags.Analysis.Electron.min_pT,
                                     maxEta=2.47
                                     ))

    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="XbbCalibMuons_%SYS%",
                                 minPt=flags.Analysis.Muon.min_pT,
                                 maxEta=2.5,
                                 ))

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.XBBCALIB.XbbCalibSelectorAlg(
            "XbbCalibSelectorAlg",
            eventDecisionOutputDecoration="XbbCalib_pass_sr_%SYS%",
            bypass=flags.Analysis.bypass,
            minMet=flags.Analysis.MET.min_met,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel
        )
    )

    cfg.addEventAlgo(
        CompFactory.XBBCALIB.BaselineVarsXbbCalibAlg(
            "BaselineVarsXbbCalibAlg",
            floatVariableList=float_variables,
            intVariableList=int_variables,
            isMC=flags.Input.isMC,
            minMet=flags.Analysis.MET.min_met,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            GN2X_WPs=flags.Analysis.Large_R_jet.GN2X_hbb_wps,
        )
    )

    return cfg


def get_BaselineVarsXbbCalibAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for object in [
            'probe_jet_pt',
            'probe_jet_eta',
            'probe_jet_phi',
            'probe_jet_m',
            'probe_jet_phbb',
            'probe_jet_phcc',
            'probe_jet_pqcd',
            'probe_jet_ptop',
            'probe_jet_dhcc',
            'probe_jet_dhbb',
            'tag_jet_pt',
            'tag_jet_eta',
            'tag_jet_phi',
            'tag_jet_m',
            'lepton_pt',
            'lepton_eta',
            'lepton_phi',
            'dRJetLep',
            'tag_lep_m',
            'Lepton1_effSF']:
        float_variable_names.append(object)

    for var in flags.Analysis.Large_R_jet.GN2X_hbb_wps:
        int_variable_names += ["probe_jet_Pass_GN2X_" + var]

    return float_variable_names, int_variable_names


def XbbCalib_branches(flags):
    branches = []

    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsXbbCalibAlg_variables(flags)

    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> XbbCalib_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "XbbCalib")

    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches
    branches += ["EventInfo.XbbCalib_pass_sr_%SYS% -> XbbCalib_pass_SR"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    branches += [flags.Analysis.container_names.output.reco10UFOJet
                 + ".isAnalysisJet_%SYS% -> recojet_antikt10UFO_isAnalysisLRJet"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches, float_variable_names, int_variable_names
