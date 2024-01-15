from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# bbyy analysis chain


def bbyy_cfg(flags, smalljetkey, photonkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    PhotonWPLabel = f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.PhotonSelectorAlg(
            "PhotonSelectorAlg",
            containerInKey=PhotonWPLabel + photonkey,
            containerOutKey="bbyyAnalysisPhotons_%SYS%",
            pTsort=True,
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

    # Photons
    photon_kinematics = ["pt", "eta", "phi", "E"]
    pt_ords = ["Leading", "Subleading"]
    for pt_ord in pt_ords:
        for kin in photon_kinematics:
            branches += [f"EventInfo.{pt_ord}_Photon_{kin}_%SYS% -> %SYS%_{pt_ord}_Photon_{kin}"]  # noqa

    diphoton_variables = ["myy", "pTyy", "dRyy", "Etayy", "Phiyy",
                          "Photon1_ptOvermyy", "Photon2_ptOvermyy"]
    for var in diphoton_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    # BJets
    btag_variables = ["pt", "eta", "phi", "E", "truthLabel"]
    btag_pt_ords = ["b1", "b2"]
    for pt_ord in btag_pt_ords:
        for kin in btag_variables:
            branches += [f"EventInfo.Jet_{kin}_{pt_ord}_%SYS% -> %SYS%_Jet_{kin}_{pt_ord}"]  # noqa

    if flags.Input.isMC:
        branches += ["EventInfo.ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%"
                     " -> weight_ftag_effSF_"
                     f"{flags.Analysis.small_R_jet.btag_wp}_%SYS%",]

        branches += ["EventInfo.jvt_effSF_%SYS% -> weight_jvt_effSF_%SYS%"]

    dibjet_variables = ["mbb", "pTbb", "dRbb", "Etabb", "Phibb"]
    for var in dibjet_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    # di-higgs variables
    dihiggs_variables = [
        "mbbyy", "pTbbyy", "Etabbyy", "Phibbyy", "dRbbyy", "mbbyy_star"
    ]
    for var in dihiggs_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    n_object = ["nPhotons", "nJets", "nCentralJets", "nBJets"]
    for var in n_object:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    # mva variables
    mva_variables = ["HT", "topness","missEt","metphi"]
    for var in mva_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    branches += ["EventInfo.PassAllCuts_%SYS% -> %SYS%_PassAllCuts"]

    if (flags.Analysis.save_bbyy_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> %SYS%_{cut}"]

    return branches
