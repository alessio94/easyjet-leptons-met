from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units
from itertools import chain
from AthenaConfiguration.Enums import LHCPeriod

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    PhotonSelectorAlgCfg, JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_lepton_branches_variables,
    get_selected_photon_branches_variables,
)


def llyy_cfg(flags, smalljetkey, muonkey, electronkey, photonkey,
             float_variables=None, int_variables=None):
    keys = ["photons", "leptons", "baseline"]
    if not float_variables:
        float_variables = {key: [] for key in keys}
    if not int_variables:
        int_variables = {key: [] for key in keys}

    cfg = ComponentAccumulator()

    PhotonWPLabel = f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}'
    cfg.merge(PhotonSelectorAlgCfg(flags,
                                   containerInKey=photonkey,
                                   containerOutKey="llyyAnalysisPhotons_%SYS%",
                                   minPt=5 * Units.GeV))

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

    cfg.merge(JetSelectorAlgCfg(flags,
                                containerInKey=smalljetkey,
                                containerOutKey="llyyAnalysisJets_%SYS%",
                                minPt=20 * Units.GeV,
                                selectBjet=False))

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
            saveCutFlow=flags.Analysis.save_cutflow,
            isMC=flags.Input.isMC,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )
    # calculate final llyy vars
    cfg.addEventAlgo(
        CompFactory.HZALLYY.PhotonVarsAlg(
            "PhotonVarsAlg",
            isMC=flags.Input.isMC,
            phWP=PhotonWPLabel,
            photons=photonkey,
            floatVariableList=float_variables['photons'],
            intVariableList=int_variables['photons'],
        )
    )
    # calculate final llyy vars

    cfg.addEventAlgo(
        CompFactory.HZALLYY.LeptonVarsAlg(
            "LeptonVarsAlg",
            isMC=flags.Input.isMC,
            muonWP=MuonWPLabel,
            eleWP=ElectronWPLabel,
            saveDummyEleSF=flags.GeoModel.Run is LHCPeriod.Run2,
            muons=muonkey,
            electrons=electronkey,
            floatVariableList=float_variables['leptons'],
            intVariableList=int_variables['leptons'],
        )
    )
    cfg.addEventAlgo(
        CompFactory.HZALLYY.BaselineVarsAlg(
            "BaselineVarsAlg",
            floatVariableList=float_variables['baseline'],
            intVariableList=int_variables['baseline'],
        )
    )

    return cfg


def get_PhotonVarsAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    int_variable_names = ["nPhotons"]
    return float_variable_names, int_variable_names


def get_LeptonVarsAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    int_variable_names += ["nLeptons", "nElectrons", "nMuons"]
    float_variable_names += ["mll", "pTll", "Etall", "Phill", "dRll",
                             "dEtall", "dPhill"]
    return float_variable_names, int_variable_names


def get_BaselineVarsAlg_variables(flags):

    float_variable_names = []
    int_variable_names = []
    float_variable_names += ["res_myy", "res_pTyy", "res_Etayy", "res_Phiyy",
                             "res_dRyy", "res_dEtayy", "res_dPhiyy", "res_Xyy",
                             "res_Ph1ptOvermyy", "res_Ph2ptOvermyy"]
    float_variable_names += ["res_mH_Za", "res_pTH_Za", "res_EtaH_Za", "res_PhiH_Za",
                             "res_dRH_Za", "res_dEtaH_Za", "res_dPhiH_Za"]
    float_variable_names += ["mer_mH_Za", "mer_pTH_Za", "mer_EtaH_Za", "mer_PhiH_Za",
                             "mer_dRH_Za", "mer_dEtaH_Za", "mer_dPhiH_Za"]
    int_variable_names += ["isResolved_Event", "isMerged_Event"]
    return float_variable_names, int_variable_names


def get_BaselineVarsllyyAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def llyy_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # PhotonVarsllyyAlg algorithm

    all_baseline_variable_names = []
    keys = ["baseline", "photons", "leptons"]
    float_variable_names = {key: [] for key in keys}
    int_variable_names = {key: [] for key in keys}

    # these are the variables that will always be stored by easyjet specific to analysis
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsAlg_variables(flags)
    photon_float_variables, photon_int_variables \
        = get_PhotonVarsAlg_variables(flags)
    lepton_float_variables, lepton_int_variables \
        = get_LeptonVarsAlg_variables(flags)

    float_variable_names['photons'] += photon_float_variables
    int_variable_names['photons'] += photon_int_variables
    float_variable_names['leptons'] += lepton_float_variables
    int_variable_names['leptons'] += lepton_int_variables
    float_variable_names['baseline'] += baseline_float_variables
    int_variable_names['baseline'] += baseline_int_variables

    all_baseline_variable_names += [
        *chain.from_iterable(float_variable_names.values()),
        *chain.from_iterable(int_variable_names.values())]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> llyy_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches = {key: [] for key in keys}
    object_level_float_variables = {key: [] for key in keys}
    object_level_int_variables = {key: [] for key in keys}

    (object_level_branches['photons'],
     object_level_float_variables['photons'],
     object_level_int_variables['photons']) = \
        get_selected_photon_branches_variables(flags, "llyy")

    float_variable_names['photons'] += object_level_float_variables['photons']
    int_variable_names['photons'] += object_level_int_variables["photons"]

    (object_level_branches['leptons'],
     object_level_float_variables['leptons'],
     object_level_int_variables['leptons']) = \
        get_selected_lepton_branches_variables(flags, "llyy")

    float_variable_names['leptons'] += object_level_float_variables['leptons']
    int_variable_names['leptons'] += object_level_int_variables['leptons']

    branches += object_level_branches["leptons"]
    branches += object_level_branches["photons"]

    if (flags.Analysis.save_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> llyy_{cut}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return (branches, float_variable_names, int_variable_names)
