from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# ttHH analysis chain


def ttHH_cfg(flags, smalljetkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=MuonWPLabel + muonkey,
            containerOutKey="ttHHAnalysisMuons_%SYS%",
            minPt=10e3,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=ElectronWPLabel + electronkey,
            containerOutKey="ttHHAnalysisElectrons_%SYS%",
            minPt=10e3,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallRJet_BTag_SelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="ttHHAnalysisJets_BTag_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            maxEta=2.5,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallRJet_SelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="ttHHAnalysisJets_%SYS%",
            bTagWPDecorName="",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.JetPairingAlgttHH(
            "JetPairingAlgHH",
            containerInKey="ttHHAnalysisJets_BTag_%SYS%",
            containerOutKey="pairedttHHAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            pairingStrategy="chiSquare",
        )
    )

    # TODO: store variables from HZ and ZZ pairing
    '''
    cfg.addEventAlgo(
        CompFactory.ttHH.JetPairingAlgttHH(
            "JetPairingAlgHZ",
            containerInKey="ttHHAnalysisJets_BTag_%SYS%",
            containerOutKey="pairedttHZAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            pairingStrategy="chiSquare",
            targetMass1=125 * Units.GeV,
            targetMass2=91.2 * Units.GeV,
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.JetPairingAlgttHH(
            "JetPairingAlgZZ",
            containerInKey="ttHHAnalysisJets_BTag_%SYS%",
            containerOutKey="pairedttZZAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            pairingStrategy="chiSquare",
            targetMass1=91.2 * Units.GeV,
            targetMass2=91.2 * Units.GeV,
        )
    )'''

    cfg.addEventAlgo(
        CompFactory.ttHH.BaselineVarsttHHAlg(
            "BaselineVarsttHHAlg",
            bjets="pairedttHHAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            jets="ttHHAnalysisJets_%SYS%",
            muons="ttHHAnalysisMuons_%SYS%",
            electrons="ttHHAnalysisElectrons_%SYS%",
            isMC=flags.Input.isMC
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.SelectionFlagsttHHAlg(
            "SelectionFlagsttHHAlg",
            bjets="pairedttHHAnalysisJets_"
            + flags.Analysis.small_R_jet.btag_wp + "_%SYS%",
            jets="ttHHAnalysisJets_%SYS%",
            muons="ttHHAnalysisMuons_%SYS%",
            electrons="ttHHAnalysisElectrons_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_ttHH_cutflow,
            triggers=flags.Analysis.TriggerChains,
            nLeptons=flags.Analysis.nLeptons
        )
    )

    return cfg


def ttHH_branches(flags):
    branches = []

    if (flags.Analysis.save_ttHH_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> %SYS%_{cut}"]

    # BJets
    btag_variables = ["pt", "eta", "phi", "E", "truthLabel"]
    btag_pt_ords = ["b1", "b2", "b3", "b4", "b5", "b6"]
    for pt_ord in btag_pt_ords:
        for kin in btag_variables:
            branches += [f"EventInfo.Jet_{kin}_{pt_ord}_%SYS% -> %SYS%_Jet_{kin}_{pt_ord}"] # noqa

    H_candidate_variables = [
        "H1_m", "H1_pt", "H1_eta", "H1_phi",
        "H2_m", "H2_pt", "H2_eta", "H2_phi"
    ]
    for var in H_candidate_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    # additional variables
    additional_variables = ["HT", "nJets", "nBJets"]
    for var in additional_variables:
        branches += [f"EventInfo.{var}_%SYS% -> %SYS%_{var}"]

    angular_variables = [
        "DeltaR12", "DeltaR34", "DeltaR56",
        "DeltaR1234", "DeltaR3456", "DeltaR5612",
        "DeltaPhi12", "DeltaPhi34", "DeltaPhi56",
        "DeltaPhi1234", "DeltaPhi3456", "DeltaPhi5612",
        "DeltaEta12", "DeltaEta34", "DeltaEta56",
        "DeltaEta1234", "DeltaEta3456", "DeltaEta5612",
        "DeltaRMax", "DeltaRMin", "DeltaRMean",
        "DeltaEtaMax", "DeltaEtaMin", "DeltaEtaMean"
    ]

    for var in angular_variables:
        branches += [f"EventInfo.Jets_{var}_%SYS% -> %SYS%_Jets_{var}"]

    branches += ["EventInfo.PassAllCuts_%SYS% -> %SYS%_PassAllCuts"]

    lepton_variables = ["pt", "eta", "phi", "E"]
    leptons = [
        "Leading_Electron", "Subleading_Electron",
        "Leading_Muon", "Subleading_Muon"
    ]

    for lep in leptons:
        for var in lepton_variables:
            branches += [f"EventInfo.{lep}_{var}_%SYS% -> %SYS%_{lep}_{var}"]

    leptonPair_variables = ["pt", "eta", "phi", "m", "dR"]
    leptonPairs = [
        "ee", "mumu", "emu"
    ]

    for lep in leptonPairs:
        for var in leptonPair_variables:
            branches += [f"EventInfo.{lep}_{var}_%SYS% -> %SYS%_{lep}_{var}"]

    return branches
