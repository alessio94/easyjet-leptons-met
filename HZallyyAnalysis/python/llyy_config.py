from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units
from AthenaConfiguration.Enums import LHCPeriod

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    PhotonSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def llyy_cfg(flags, muonkey, electronkey, photonkey,
             float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []
    cfg = ComponentAccumulator()

    PhotonWPLabel = f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}'
    cfg.merge(PhotonSelectorAlgCfg(flags,
                                   containerInKey=photonkey,
                                   containerOutKey="llyyAnalysisPhotons_%SYS%",
                                   minPt=22. * Units.GeV))

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="llyyAnalysisMuons_%SYS%",
                                 minPt=9 * Units.GeV))
    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="llyyAnalysisElectrons_%SYS%",
                                     minPt=9 * Units.GeV))

    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))
    # cfg.merge(JetSelectorAlgCfg(flags,
    #                            containerInKey=smalljetkey,
    #                            containerOutKey="llyyAnalysisJets_%SYS%",
    #                            minPt=20 * Units.GeV,
    #                            minimumAmount=2))  # -1 means ignores this

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    # Selection
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    # Selection
    cfg.addEventAlgo(
        CompFactory.HZALLYY.HZAllyySelectorAlg(
            "HZAllyySelectorAlg",
            eventDecisionOutputDecoration="llyy_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_llyy_cutflow,
            isMC=flags.Input.isMC,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )
    # calculate final llyy vars
    cfg.addEventAlgo(
        CompFactory.HZALLYY.BaselineVarsllyyAlg(
            "FinalVarsllyyAlg",
            isMC=flags.Input.isMC,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            saveDummyEleSF=flags.GeoModel.Run is LHCPeriod.Run2,
            phWP=PhotonWPLabel,
            saveDummyPhotonSF=flags.GeoModel.Run is LHCPeriod.Run2,
            muons=muonkey,
            electrons=electronkey,
            photons=photonkey,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    return cfg


def get_BaselineVarsllyyAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

# for object in ["ll", "bb"]:
    for object in ["ll", "yy", "H_Za"]:
        for var in ["m", "pT", "dR", "Eta", "Phi"]:
            float_variable_names.append(f"{var}{object}")
    int_variable_names += ["nElectrons", "nMuons", "nPhotons"]

    return float_variable_names, int_variable_names


def get_BaselineVarsllyyAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def llyy_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsllyyAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to HHbbtt
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsllyyAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    if flags.Analysis.do_mmc:
        # do not append mmc variables to float_variable_names
        # or int_variable_names as they are stored by the
        # mmc algortithm not BaselineVarsllyyAlg
        for var in ["status", "pt", "eta", "phi", "m"]:
            all_baseline_variable_names.append(f"mmc_{var}")

    float_NW_variable_names = []
    float_PNN_variable_names = []
    if flags.Analysis.NeutrinoWeighting.doNW:
        # do not append TopReco variables to float_variable_names
        # or int_variable_names as they are stored by the
        # TopReco algortithm not BaselineVarsllyyAlg
        all_baseline_variable_names.append("NW_solutions")
        for var in ["neutrinoweight"]:
            float_NW_variable_names.append(f"NW_{var}")
    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsllyyAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables
    if flags.Analysis.do_resonant_PNN:
        for m_X in flags.Analysis.mX_values:
            float_PNN_variable_names.append(f"PNN_Score_X{m_X}")
    all_baseline_variable_names += [
        *float_variable_names,
        *int_variable_names]
    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> llyy_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "llyy")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    branches += ["EventInfo.llyy_pass_sr_%SYS% -> llyy_pass_SR"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    if (flags.Analysis.save_llyy_cutflow):
        cutList = flags.Analysis.CutList + flags.Analysis.Categories
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> llyy_{cut}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # trigger variables do not need to be added to variable_names
    # as it is written out in HHllyySelectorAlg
    for cat in ["SLT", "DLT", "ASLT1_em", "ASLT1_me", "ASLT2"]:
        branches += \
            [f"EventInfo.pass_trigger_{cat}_%SYS% -> llyy_pass_trigger_{cat}"
             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return (branches, float_variable_names, int_variable_names)
