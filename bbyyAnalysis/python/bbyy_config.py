from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)


def bbyy_cfg(flags, smalljetkey, photonkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    PhotonWPLabel = f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.PhotonSelectorAlg(
            "PhotonSelectorAlg",
            containerInKey=PhotonWPLabel + photonkey,
            containerOutKey="bbyyAnalysisPhotons_%SYS%",
            photonSF_WP=PhotonWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=MuonWPLabel + muonkey,
            containerOutKey="bbyyAnalysisMuons_%SYS%",
            minPt=10e3,
            muonSF_WP=MuonWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=ElectronWPLabel + electronkey,
            containerOutKey="bbyyAnalysisElectrons_%SYS%",
            minPt=10e3,
            eleSF_WP=ElectronWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "JetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbyyAnalysisJets_%SYS%",
            bTagWPDecorName="",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.BaselineVarsbbyyAlg(
            "BaselineVarsbbyyAlg",
            photons="bbyyAnalysisPhotons_%SYS%",
            photonWP=PhotonWPLabel,
            jets="bbyyAnalysisJets_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            isMC=flags.Input.isMC
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.SelectionFlagsbbyyAlg(
            "SelectionFlagsbbyyAlg",
            photons="bbyyAnalysisPhotons_%SYS%",
            jets="bbyyAnalysisJets_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            muons="bbyyAnalysisMuons_%SYS%",
            electrons="bbyyAnalysisElectrons_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_bbyy_cutflow,
            photonTriggers=flags.Analysis.TriggerChains
        )
    )

    return cfg


def bbyy_branches(flags):
    branches = []

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    branches += get_selected_objects_branches(flags, "bbyy")

    diphoton_variables = ["myy", "pTyy", "dRyy", "Etayy", "Phiyy"]
    for var in diphoton_variables:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_Diphoton_{var}_%SYS%"]

    # BJets
    if flags.Input.isMC:
        branches += ["EventInfo.ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%"
                     " -> weight_ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%",]

        branches += ["EventInfo.jvt_effSF_%SYS% -> weight_jvt_effSF_%SYS%"]

    dibjet_variables = ["mbb", "pTbb", "dRbb", "Etabb", "Phibb"]
    for var in dibjet_variables:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    # di-higgs variables
    dihiggs_variables = [
        "mbbyy", "pTbbyy", "Etabbyy", "Phibbyy", "dRbbyy", "mbbyy_star"
    ]
    for var in dihiggs_variables:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    n_object = ["nPhotons", "nJets", "nCentralJets", "nBJets"]
    for var in n_object:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    # mva variables
    mva_variables = ["HT", "topness","missEt","metphi"]
    for var in mva_variables:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    branches += ["EventInfo.PassAllCuts_%SYS% -> bbyy_PassAllCuts_%SYS%"]

    if (flags.Analysis.save_bbyy_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> bbyy_{cut}_%SYS%"]

    return branches
