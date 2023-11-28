from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units


# bbll analysis chain

def bbll_cfg(flags, smalljetkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=MuonWPLabel + muonkey,
            containerOutKey="bbllAnalysisMuons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
            minPt=9 * Units.GeV,
        )
    )

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=ElectronWPLabel + electronkey,
            containerOutKey="bbllAnalysisElectrons_%SYS%",
            checkOR=flags.Analysis.do_overlap_removal,
            minPt=10 * Units.GeV,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbllAnalysisJets_%SYS%",
            bTagWPDecorName="",
            minPt=20 * Units.GeV,
            minimumAmount=2,  # -1 means ignores this
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    # Selection
    cfg.addEventAlgo(
        CompFactory.HHBBLL.HHbbllSelectorAlg(
            "HHbbllSelectorAlg",
            jets="bbllAnalysisJets_%SYS%",
            muons="bbllAnalysisMuons_%SYS%",
            electrons="bbllAnalysisElectrons_%SYS%",
            met="AnalysisMET_%SYS%",
            eventDecisionOutputDecoration="bbll_pass_sr_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
        )
    )
    # MMC decoration
    if flags.Analysis.do_mmc:
        cfg.addEventAlgo(
            CompFactory.HHBBLL.MMCDecoratorAlg(
                "MMCDecoratorAlg",
                jets="bbllAnalysisJets_%SYS%",
                muons="bbllAnalysisMuons_%SYS%",
                electrons="bbllAnalysisElectrons_%SYS%",
                met="AnalysisMET_%SYS%",
            )
        )

    # calculate final bbll vars
    cfg.addEventAlgo(
        CompFactory.HHBBLL.BaselineVarsbbllAlg(
            "FinalVarsbbllAlg",
            jets="bbllAnalysisJets_%SYS%",
            muons="bbllAnalysisMuons_%SYS%",
            electrons="bbllAnalysisElectrons_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R.btag_wp,
        )
    )

    return cfg


def bbll_branches(flags):
    branches = []

    bbll_vars = [
        "TWO_ISO_MUONS", "TWO_ISO_ELECTRONS",
        "Mee", "Mmumu", "Pass_ee",
        "EXACTLY_TWO_B_JETS", "Pass_mumu",
        # Leading electron
        "Leading_Electron_pt",
        "Leading_Electron_eta",
        "Leading_Electron_phi",
        "Leading_Electron_E",
        # Subleading electron
        "Subleading_Electron_pt",
        "Subleading_Electron_eta",
        "Subleading_Electron_phi",
        "Subleading_Electron_E",
        # Leading muon
        "Leading_Muon_pt",
        "Leading_Muon_eta",
        "Leading_Muon_phi",
        "Leading_Muon_E",
        # Subleading muon
        "Subleading_Muon_pt",
        "Subleading_Muon_eta",
        "Subleading_Muon_phi",
        "Subleading_Muon_E",
        # Leading jet
        "Leading_Jet_pt",
        "Leading_Jet_eta",
        "Leading_Jet_phi",
        "Leading_Jet_E",
        # Subleading jet
        "Subleading_Jet_pt",
        "Subleading_Jet_eta",
        "Subleading_Jet_phi",
        "Subleading_Jet_E"
    ]

    for tree_flags in flags.Analysis.ttree_output:
        for var in bbll_vars:
            if tree_flags['write_object_systs_only_for_pt'] and "pt" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbll_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbll_%SYS%_{var}"]

    dilepton_variables = ["dRee", "Etaee", "Phiee",
                          "dRmumu", "Etamumu", "Phimumu"]
    for tree_flags in flags.Analysis.ttree_output:
        for var in dilepton_variables:
            if tree_flags['write_object_systs_only_for_pt'] and "pt" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbll_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> bbll_%SYS%_{var}"]

    branches += ["EventInfo.mee_%SYS% -> %SYS%_mee"]
    branches += ["EventInfo.pTee_%SYS% -> %SYS%_pTee"]
    branches += ["EventInfo.mmumu_%SYS% -> %SYS%_mmumu"]
    branches += ["EventInfo.pTmumu_%SYS% -> %SYS%_pTmumu"]

    # BJets
    btag_variables = ["pt", "eta", "phi", "E"]
    btag_pt_ords = ["b1", "b2"]
    for tree_flags in flags.Analysis.ttree_output:
        for pt_ord in btag_pt_ords:
            for kin in btag_variables:
                if tree_flags['write_object_systs_only_for_pt'] and "pt" not in var:
                    branches += [
                        f"EventInfo.Jet_{kin}_{pt_ord}_NOSYS -> bbll_Jet_{kin}_{pt_ord}"
                    ]
                else:
                    branches += [
                        f"EventInfo.Jet_{kin}_{pt_ord}_%SYS% -> "
                        f"%SYS%_Jet_{kin}_{pt_ord}"
                    ]
    branches += ["EventInfo.nBJets_%SYS% -> %SYS%_nBJets"]
    branches += ["EventInfo.nElectrons_%SYS% -> %SYS%_nElectrons"]
    branches += ["EventInfo.nMuons_%SYS% -> %SYS%_nMuons"]
    branches += ["EventInfo.nJets_%SYS% -> %SYS%_nJets"]

    dibjet_variables = ["dRbb", "Etabb", "Phibb"]
    for tree_flags in flags.Analysis.ttree_output:
        for var in dibjet_variables:
            if tree_flags['write_object_systs_only_for_pt'] and "pt" not in var:
                branches += [f"EventInfo.{var}_NOSYS -> bbll_{var}"]
            else:
                branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    branches += ["EventInfo.mbb_%SYS% -> %SYS%_mbb"]
    branches += ["EventInfo.pTbb_%SYS% -> %SYS%_pTbb"]

    if flags.Analysis.do_mmc:
        for var in ["status", "pt", "eta", "phi", "m"]:
            if tree_flags['write_object_systs_only_for_pt'] and var != "pt":
                branches += [f"EventInfo.mmc_{var}_NOSYS -> mmc_{var}"]
            else:
                branches += [f"EventInfo.mmc_{var}_%SYS% -> mmc_%SYS%_{var}"]

    branches += ["EventInfo.bbll_pass_sr_%SYS% -> bbll_pass_SR_%SYS%"]

    return branches
