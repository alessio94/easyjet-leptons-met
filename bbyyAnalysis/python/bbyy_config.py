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
            PCBTDecorName="ftag_quantile_" + flags.Analysis.small_R_jet.btag_extra_wps[0], # noqa
            pTsort=False,
            PCBTsort=True,
            bTagWPDecorName="",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.BaselineVarsbbyyAlg(
            "BaselineVarsbbyyAlg",
            photons="bbyyAnalysisPhotons_%SYS%",
            photonWP=PhotonWPLabel,
            muons="bbyyAnalysisMuons_%SYS%",
            electrons="bbyyAnalysisElectrons_%SYS%",
            jets="bbyyAnalysisJets_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            PCBTDecorName="ftag_quantile_" + flags.Analysis.small_R_jet.btag_extra_wps[0],  # noqa
            isMC=flags.Input.isMC
        )
    )

    selection_name = flags.Analysis.selection_name

    cfg.addEventAlgo(
        CompFactory.HHBBYY.bbyySelectorAlg(
            "bbyySelectorAlg",
            photons="bbyyAnalysisPhotons_%SYS%",
            jets="bbyyAnalysisJets_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            muons="bbyyAnalysisMuons_%SYS%",
            electrons="bbyyAnalysisElectrons_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_bbyy_cutflow,
            photonTriggers=flags.Analysis.TriggerChains,
            eventDecisionOutputDecoration=f"bbyy_pass_{selection_name}_%SYS%",
            isMC=flags.Input.isMC,
            bypass=flags.Analysis.bypass,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.EventInfoGlobalAlg(
            isMC=flags.Input.isMC,
            Years=flags.Analysis.Years,
        )
    )
    if flags.Analysis.do_resonant_PNN:
        cfg.addEventAlgo(
            CompFactory.SHBBYY.ResonantPNNbbyyAlg(
                "ResonantPNNbbyyAlg",
                photons="bbyyAnalysisPhotons_%SYS%",
                jets="bbyyAnalysisJets_%SYS%",
                bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
                mX_mS_pairs=flags.Analysis.mX_mS_pairs,
                mS_values=flags.Analysis.mS_values,
                mX_values=flags.Analysis.mX_values,
                mX_1bjet=flags.Analysis.mX_1bjet
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
        btag_wps = [flags.Analysis.small_R_jet.btag_wp]
        if 'btag_extra_wps' in flags.Analysis.small_R_jet:
            btag_wps += flags.Analysis.small_R_jet.btag_extra_wps

        for wp in btag_wps:
            branches += [f"EventInfo.ftag_effSF_{wp}_%SYS%"
                         f" -> weight_ftag_effSF_{wp}_%SYS%",]

        branches += ["EventInfo.jvt_effSF_%SYS% -> weight_jvt_effSF_%SYS%"]

    dibjet_variables = ["mbb", "pTbb", "dRbb", "Etabb", "Phibb"]
    for var in dibjet_variables:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    # HbbCandidate jets
    Hbbcandidate_jets_variables = []
    for i in range(1,3):
        prefix = f"HbbCandidate_Jet{i}_"
        for var in ["pt", "phi", "eta", "E", "truthLabel", "pcbt"]:
            Hbbcandidate_jets_variables += [prefix + var]

    for var in Hbbcandidate_jets_variables:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    # di-higgs variables
    dihiggs_variables = [
        "mbbyy", "pTbbyy", "Etabbyy", "Phibbyy", "dRbbyy",
        "mbbyy_star"
    ]
    for var in dihiggs_variables:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    n_object = ["nPhotons", "nJets", "nCentralJets", "nBJets", "nLeptons"]
    for var in n_object:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    # mva variables
    mva_variables = ["HT", "topness", "sphericityT", "planarFlow",
                     "pTBalance", "missEt", "metphi"]
    for var in mva_variables:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    # VBFJets
    vbfjet_variables = [
        "Jet_vbf1_pt", "Jet_vbf1_eta", "Jet_vbf1_phi", "Jet_vbf1_E",
        "Jet_vbf2_pt", "Jet_vbf2_eta", "Jet_vbf2_phi", "Jet_vbf2_E",
        "m_vbfjj", "eta_vbfjj"
    ]
    for var in vbfjet_variables:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    s_name = flags.Analysis.selection_name
    branches += \
        [f"EventInfo.bbyy_pass_{s_name}_%SYS% -> bbyy_pass_{s_name}_%SYS%"]

    if (flags.Analysis.save_bbyy_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> bbyy_{cut}_%SYS%"]

    branches += ["EventInfo.dataTakingYear -> dataTakingYear"]

    if flags.Analysis.do_resonant_PNN:
        PNN_ScoreLabel = "SH_PNN_Score"
        PNN_1bjet_ScoreLabel = "SH_PNN_Score_1bjet"

        for pair in flags.Analysis.mX_mS_pairs:
            m_X = pair[0]
            m_S = pair[-1]
            var = str(PNN_ScoreLabel + "_X" + str(m_X) + "_S" + str(m_S))
            branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

        for m_X in flags.Analysis.mX_values:
            for m_S in flags.Analysis.mS_values:
                if m_X > 500 and m_S < 70:
                    continue
                if m_X - m_S <= 125:
                    continue
                var = str(PNN_ScoreLabel + "_X" + str(m_X) + "_S" + str(m_S))
                branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

        for m_X in flags.Analysis.mX_1bjet:
            var = str(PNN_1bjet_ScoreLabel + "_X" + str(m_X))
            branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}_%SYS%"]

    return branches
