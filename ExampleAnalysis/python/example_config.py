from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    JetSelectorAlgCfg, PhotonSelectorAlgCfg)

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables)


def example_cfg(flags, smalljetkey, largeRjetkey, photonkey,
                float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    cfg.merge(PhotonSelectorAlgCfg(flags,
              containerInKey=photonkey,
              containerOutKey="ExamplePhotons_%SYS%",
              minPt=15. * Units.GeV))

    cfg.merge(JetSelectorAlgCfg(
              flags,
              containerInKey=smalljetkey,
              containerOutKey="ExampleJets_%SYS%",
              minPt=20 * Units.GeV,
              bTagWPDecorName="",
              selectBjet=False))

    cfg.merge(JetSelectorAlgCfg(
              flags,
              name="LargeJetSelectorAlg",
              containerInKey=largeRjetkey,
              containerOutKey="ExampleLargeRJets_%SYS%",
              minimumAmount=1))

    cfg.addEventAlgo(
        CompFactory.EXANA.MjjExampleAlg(
            "MjjExampleAlg"))

    cfg.addEventAlgo(
        CompFactory.EXANA.BaselineVarsExampleAlg(
            "BaselineVarsExampleAlg",
            floatVariableList=float_variables,
            intVariableList=int_variables,
        )
    )

    return cfg


def get_BaselineVarsExampleAlg_variables(flags):
    float_variable_names = ["mjj"]
    int_variable_names = ["n_photons"]

    return float_variable_names, int_variable_names


def example_branches(flags):
    # a list of strings which maps the variable name in the c++ code
    # to the outputname:
    # "EventInfo.<var> -> <outputvarname>"
    branches = ["mjj"]

    # this will be all the variables that are calculated by the
    # BaselineVarsExampleCalibAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsExampleAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    # These are the variables always saved with the objects selected
    # by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, \
        object_level_int_variables = \
        get_selected_objects_branches_variables(flags, "ExampleAnalysis")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    branches = [f"EventInfo.{var}_%SYS% -> example_{var}"
                + flags.Analysis.systematics_suffix_separator + "%SYS%"
                for var in branches]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> baseline_example_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    branches += object_level_branches

    return branches, float_variable_names, int_variable_names
