from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# yybb analysis chain


def yybb_cfg(flags, smalljetkey, photonkey, muonkey, electronkey):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.Easyjet.PhotonSelectorAlg(
            "PhotonSelectorAlg",
            containerInKey=photonkey,
            containerOutKey="yybbAnalysisPhotons_%SYS%",
            LeadPho_ptOverMyy_min=-1,  # pT/myy
            SubleadPho_ptOverMyy_min=-1,  # pT/myy
            etaBounds=[
                1.37,
                1.52,
                2.37
            ],  # photon eta must be: < 1.37, or between 1.52 and 2.37
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
            pTsort=True,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=muonkey,
            containerOutKey="yybbAnalysisMuons_%SYS%",
            minPt=10e3,
            maxEta=2.7,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
            pTsort=True,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=electronkey,
            containerOutKey="yybbAnalysisElectrons_%SYS%",
            minPt=10e3,
            minEtaVeto=1.37,
            maxEtaVeto=1.52,
            maxEta=2.47,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
            pTsort=True,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "JetBTag_SelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="yybbAnalysisJets_BTag_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
            minPt=25e3,
            maxEta=2.5,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
            maximumAmount=99,
            pTsort=True,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "JetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="yybbAnalysisJets_%SYS%",
            bTagWPDecorName="",
            minPt=25e3,
            maxEta=4.4,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
            maximumAmount=99,
            pTsort=True,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.BaselineVarsyybbAlg(
            "BaselineVarsyybbAlg",
            photons="yybbAnalysisPhotons_%SYS%",
            bjets="yybbAnalysisJets_BTag_%SYS%",
            jets="yybbAnalysisJets_%SYS%",
            isMC=flags.Input.isMC
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.SelectionFlagsyybbAlg(
            "SelectionFlagsyybbAlg",
            photons="yybbAnalysisPhotons_%SYS%",
            bjets="yybbAnalysisJets_BTag_%SYS%",
            jets="yybbAnalysisJets_%SYS%",
            muons="yybbAnalysisMuons_%SYS%",
            electrons="yybbAnalysisElectrons_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_yybb_cutflow,
            photonTriggers=flags.Analysis.TriggerChains
        )
    )

    return cfg


def yybb_branches(flags):
    branches = []

    # Photons
    photon_kinematics = ["pt", "eta", "phi", "E"]
    pt_ords = ["Leading", "Subleading"]
    for pt_ord in pt_ords:
        for kin in photon_kinematics:
            branches += [f"EventInfo.{pt_ord}_Photon_{kin}_%SYS% -> %SYS%_{pt_ord}_Photon_{kin}"]  # noqa

    branches += ["EventInfo.nPhotons_%SYS% -> %SYS%_nPhotons"]

    diphoton_variables = ["myy", "pTyy", "dRyy", "Etayy", "Phiyy"]
    for var in diphoton_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    # BJets
    btag_variables = ["pt", "eta", "phi", "E", "truthLabel"]
    btag_pt_ords = ["b1", "b2"]
    for pt_ord in btag_pt_ords:
        for kin in btag_variables:
            branches += [f"EventInfo.Jet_{kin}_{pt_ord}_%SYS% -> %SYS%_Jet_{kin}_{pt_ord}"]  # noqa

    branches += ["EventInfo.nJets_%SYS% -> %SYS%_nJets"]
    branches += ["EventInfo.nBJets_%SYS% -> %SYS%_nBJets"]

    dibjet_variables = ["mbb", "pTbb", "dRbb", "Etabb", "Phibb"]
    for var in dibjet_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    # di-higgs variables
    dihiggs_variables = [
        "mbbyy", "pTbbyy", "Etabbyy", "Phibbyy", "dRbbyy", "mbbyy_star"
    ]
    for var in dihiggs_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    # mva variables
    mva_variables = ["HT"]
    for var in mva_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    branches += ["EventInfo.PassAllCuts_%SYS% -> %SYS%_PassAllCuts"]

    if (flags.Analysis.save_yybb_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> %SYS%_{cut}"]

    return branches
