from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

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

    cfg.merge(JetSelectorAlgCfg(flags, name="LargeJetSelectorAlg",
                                containerInKey=largejetkey,
                                containerOutKey="XbbCalibLRJets_%SYS%",
                                minPt=flags.Analysis.Large_R_jet.min_pT,
                                maxEta=2.0,
                                minimumAmount=1,
                                ))

    cfg.merge(JetSelectorAlgCfg(flags, name="SmallRJet_SelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="XbbCalibJets_%SYS%",
                                minPt=flags.Analysis.Small_R_jet.min_pT,
                                maxEta=2.5,
                                minimumAmount=1,
                                useJVT=True,
                                ))

    LooseElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="XbbCalibElectrons_%SYS%",
                                     looseEleWP=LooseElectronWPLabel,
                                     minPt=70 * Units.GeV,
                                     maxEta=2.47,
                                     ))

    LooseMuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="XbbCalibMuons_%SYS%",
                                 looseMuonWP=LooseMuonWPLabel,
                                 minPt=70 * Units.GeV,
                                 maxEta=2.5,
                                 ))

    cfg.addEventAlgo(
        CompFactory.XBBCALIB.XbbCalibSelectorAlg(
            "XbbCalibSelectorAlg",
            eventDecisionOutputDecoration="XbbCalib_pass_sr_%SYS%",
            bypass=flags.Analysis.bypass,
        )
    )

    cfg.addEventAlgo(
        CompFactory.XBBCALIB.BaselineVarsXbbCalibAlg(
            "BaselineVarsXbbCalibAlg",
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    return cfg


def get_BaselineVarsXbbCalibAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    for object in ["probe_jet", "tag_jet"]:
        for var in ["pt", "eta", "phi"]:
            float_variable_names.append(f"{object}_{var}")

    return float_variable_names, int_variable_names


def XbbCalib_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsXbbCalibAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to XbbCalib
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsXbbCalibAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables
    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> XbbCalib_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
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
