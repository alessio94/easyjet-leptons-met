from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
# from AthenaCommon.Constants import VERBOSE
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, JetSelectorAlgCfg, PhotonSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)
from EasyjetHub.steering.sample_metadata import is_at_least

copiedVariablePrefix = "Zcand_"


def _working_points(flags):
    wps = flags.Analysis.Large_R_jet.GN2X_hbb_wps
    return [f"xbb_select_GN2Xv01_{x}" for x in wps]


def _taggers(flags):
    by_tag = flags.Analysis.Large_R_jet.variables_in_ptag
    return by_tag.p7017 if is_at_least(flags, "p7017") else []


def ZbbyCalib_cfg(flags, smalljetkey, largejetkey, muonkey, electronkey, photonkey,
                  float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(
        JetSelectorAlgCfg(
            flags,
            name="LargeJetSelectorAlg",
            containerInKey=largejetkey,
            containerOutKey="XbbCalibLRJets_%SYS%",
            minPt=flags.Analysis.Large_R_jet.min_pT,
            maxEta=flags.Analysis.Large_R_jet.max_eta,
            minMass=flags.Analysis.Large_R_jet.min_m,
            minimumAmount=1,
        )
    )
    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="XbbCalibElectrons_%SYS%",
                                     minPt=70 * Units.GeV,
                                     ))

    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="XbbCalibMuons_%SYS%",
                                 minPt=70 * Units.GeV,
                                 maxEta=2.5,
                                 ))

    cfg.merge(PhotonSelectorAlgCfg(flags,
                                   containerInKey=photonkey,
                                   containerOutKey="XbbCalibPhotons_%SYS%",
                                   minPt=150. * Units.GeV
                                   ))

    cfg.addEventAlgo(
        CompFactory.XBBCALIB.ZbbyCalibSelectorAlg(
            "ZbbyCalibSelectorAlg",
            eventDecisionOutputDecoration="XbbCalib_pass_sr_%SYS%",
            bypass=flags.Analysis.bypass,
        )
    )
    # signal jets selection
    cfg.addEventAlgo(
        CompFactory.XBBCALIB.ZcandSelectorAlg(
            "ZcandSelectorAlg",
            lrjets="XbbCalibLRJets_%SYS%",
            photons="XbbCalibPhotons_%SYS%"
        )
    )

    cfg.addEventAlgo(
        CompFactory.XBBCALIB.BaselineVarsZbbyCalibAlg(
            "BaselineVarsZbbyCalibAlg",
            floatsToCopy=_taggers(flags),
            intsToCopy=_working_points(flags),
            copiedVariablePrefix=copiedVariablePrefix,
        )
    )

    return cfg


def get_BaselineVarsZbbyCalibAlg_variables(flags):
    float_variable_names = []
    int_variable_names = ["photons_n", "lrjets_n"]

    for object in ["Zcand", "photon"]:
        for var in ["pt", "eta", "phi", "m"]:
            float_variable_names.append(f"{object}_{var}")

    float_variable_names += [
        f'{copiedVariablePrefix}{x}' for x in _taggers(flags)
    ]

    int_variable_names += [
        f'{copiedVariablePrefix}{x}' for x in _working_points(flags)
    ]

    return float_variable_names, int_variable_names


def ZbbyCalib_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsZbbyCalibAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific
    # to XbbCalib. However, at this stage we have no varibles that are
    # calculated by this algorithm
    # so we will just make a placeholder list

    # These are the variables always saved with the objects selected
    # by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsZbbyCalibAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> ZbbyCalib_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables = \
        get_selected_objects_branches_variables(flags, "ZbbCalib")

    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    # branches += ["EventInfo.ZbbCalib_pass_sr_%SYS% -> ZbbCalib_pass_SR"
    #              + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches, float_variable_names, int_variable_names
