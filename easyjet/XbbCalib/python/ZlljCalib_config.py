from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg, JetSelectorAlgCfg,
)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)

copiedVariablePrefix = "Zcand_"

floatsToCopy = ["pt_balance", "dR", "dPhi", "dEta",
                "leplep_LR_ptsymmetry", "leplep_LR_dEta"]
intsToCopy = ["isMuonPair"]


def ZlljCalib_cfg(flags, largejetkey, muonkey, electronkey,
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
        CompFactory.XBBCALIB.ZlljCalibSelectorAlg(
            "ZlljCalibSelectorAlg",
            eventDecisionOutputDecoration="XbbCalib_pass_sr_%SYS%",
            bypass=flags.Analysis.bypass,
        )
    )

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.XBBCALIB.BaselineVarsZlljCalibAlg(
            "BaselineVarsZlljCalibAlg",
            isMC=flags.Input.isMC,
            eleWP=ElectronWPLabel,
            muonWP=MuonWPLabel,
            floatsToCopy=floatsToCopy,
            intsToCopy=intsToCopy,
            copiedVariablePrefix=copiedVariablePrefix,
        )
    )

    return cfg


def get_BaselineVarsZlljCalibAlg_variables(flags):
    float_variable_names = []
    int_variable_names = ["lrjets_n", "electrons_n", "muons_n"]

    for object in ["Zcand"]:
        for var in ["pt", "eta", "phi", "m"]:
            float_variable_names.append(f"{object}_{var}")

    float_variable_names += [
        f'{copiedVariablePrefix}{x}' for x in floatsToCopy
    ]
    int_variable_names += [
        f'{copiedVariablePrefix}{x}' for x in intsToCopy
    ]

    return float_variable_names, int_variable_names


def ZlljCalib_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsZbbjCalibAlg algorithm
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
    for object in ["Electron1", "Electron2", "Muon1", "Muon2"]:
        for var in ["pt", "eta", "phi", "m", "effSF"]:
            float_variable_names.append(f"{object}_{var}")

    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsZlljCalibAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables
    # for object in ["ll"]:
    #     for var in ["pt", "eta", "phi", "m", "dR", "dPhi", "dEta"]:
    #         float_variable_names.append(f"{object}_{var}")

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> ZlljCalib_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    object_level_branches, object_level_float_variables, \
        object_level_int_variables = \
        get_selected_objects_branches_variables(flags, "ZlljCalib")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    return branches, float_variable_names, int_variable_names
