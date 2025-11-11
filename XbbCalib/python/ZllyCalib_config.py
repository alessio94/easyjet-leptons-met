from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    JetSelectorAlgCfg, PhotonSelectorAlgCfg
)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def ZllyCalib_cfg(flags, largejetkey, muonkey, electronkey, photonkey,
                  float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="XbbCalibMuons_%SYS%",
                                 minPt=7 * Units.GeV,
                                 maxEta=2.7))

    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="XbbCalibElectrons_%SYS%",
                                     minPt=7 * Units.GeV))

    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(PhotonSelectorAlgCfg(flags,
                                   containerInKey=photonkey,
                                   containerOutKey="XbbCalibPhotons_%SYS%",
                                   minPt=150. * Units.GeV
                                   ))

    cfg.merge(
        JetSelectorAlgCfg(
            flags,
            name="LargeJetSelectorAlg",
            containerInKey=largejetkey,
            containerOutKey="XbbCalibLRJets_%SYS%",
            minPt=flags.Analysis.Large_R_jet.min_pT,
            maxEta=flags.Analysis.Large_R_jet.maxEta,
            minimumAmount=1,
        )
    )

    cfg.addEventAlgo(
        CompFactory.XBBCALIB.ZllyCalibSelectorAlg(
            "ZllyCalibSelectorAlg",
            eventDecisionOutputDecoration="XbbCalib_pass_sr_%SYS%",
            bypass=flags.Analysis.bypass,
        )
    )

    cfg.addEventAlgo(
        CompFactory.XBBCALIB.BaselineVarsZllyCalibAlg(
            "BaselineVarsZllyCalibAlg",
            isMC=flags.Input.isMC,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    return cfg


def ZllyCalib_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsZbbjCalibAlg algorithm
    float_variable_names = []
    int_variable_names = ["nJets", "nElectrons", "nMuons", "nPhotons"]

    # these are the variables that will always be stored by easyjet specific
    # to XbbCalib. However, at this stage we have no varibles that are
    # calculated by this algorithm
    # so we will just make a placeholder list

    # These are the variables always saved with the objects selected
    # by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, \
        object_level_int_variables = \
        get_selected_objects_branches_variables(flags, "XbbCalib")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    for var in object_level_branches:
        branches += [f"EventInfo.{var}_%SYS% -> XbbCalib_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    branches += [
        "EventInfo.XbbCalib_pass_sr_%SYS% -> XbbCalib_pass_SR"
        + flags.Analysis.systematics_suffix_separator
        + "%SYS%"
    ]

    return branches, float_variable_names, int_variable_names
