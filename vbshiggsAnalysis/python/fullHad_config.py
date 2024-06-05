from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def fullHad_cfg(flags, float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()
    # Selection
    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.FullHadSelectorAlg(
            "FullHadSelectorAlg",
            jets="vbshiggsAnalysisJets_%SYS%",
            muons="vbshiggsAnalysisMuons_%SYS%",
            electrons="vbshiggsAnalysisElectrons_%SYS%",
        )
    )
    # calculate final vbshiggs vars
    cfg.addEventAlgo(
        CompFactory.VBSHIGGS.BaselineVarsFullHadAlg(
            "FinalVarsFullHadAlg",
            isMC=flags.Input.isMC,
            jets="vbshiggsAnalysisJets_%SYS%",
            bTagWPDecorName="ftag_select_"
                            + flags.Analysis.small_R_jet.btag_wp,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )
    return cfg


def get_BaselineVarsFullHadAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    int_variable_names += ["nJets", "nBJets", "nCentralJets"]
    return float_variable_names, int_variable_names


def get_BaselineVarsFullHadAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def fullHad_branches(flags):
    branches = []
    # this will be all the variables that are calculated by the
    # BaselineVarsFullHadAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsFullHadAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables
    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> FullHad_{var}_%SYS%"]

    # Variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, \
        object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "FullHad")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    return branches, float_variable_names, int_variable_names
