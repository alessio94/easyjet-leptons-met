from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# ttHH analysis chain


def ttHH_cfg(flags, smalljetkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=muonkey,
            containerOutKey="ttHHAnalysisMuons",
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
            containerOutKey="ttHHAnalysisElectrons",
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
            "SmallRJet_BTag_SelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="ttHHAnalysisJets_BTag",
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
            "SmallRJet_SelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="ttHHAnalysisJets",
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
        CompFactory.ttHH.JetPairingAlgttHH(
            "JetPairingAlg",
            containerInKey="ttHHAnalysisJets_BTag",
            containerOutKey="pairedttHHAnalysisJets_"
            + flags.Analysis.small_R.btag_wp,
            pairingStrategy="chiSquare",
        )
    )

    cfg.addEventAlgo(
        CompFactory.ttHH.BaselineVarsttHHAlg(
            "FinalVarsttHHAlg",
            smallRJets_BTag_ContainerInKey="pairedttHHAnalysisJets_"
            + flags.Analysis.small_R.btag_wp,
            smallRJets_ContainerInKey="ttHHAnalysisJets",
            muonContainerInKey="ttHHAnalysisMuons",
            electronContainerInKey="ttHHAnalysisElectrons",
        )
    )

    return cfg


def ttHH_branches(flags):
    branches = []

    variables = [
        "n_leptons",
    ]

    if (flags.Analysis.do_ttHH_cutflow):
        for var in variables:
            var_str = "EventInfo.%s -> %s" % (var, var)
            branches.append(var_str)

    # BJets
    btag_variables = ["pt", "eta", "phi", "E"]
    btag_pt_ords = ["B1", "B2", "B3", "B4", "B5", "B6"]
    for pt_ord in btag_pt_ords:
        for kin in btag_variables:
            v = "EventInfo.Jet_%s_%s -> Jet_%s_%s" % \
                (kin, pt_ord, kin, pt_ord)
            branches += [v]

    H_candidate_variables = [
        "H1_m", "H1_pT", "H1_eta", "H1_phi",
        "H2_m", "H2_pT", "H2_eta", "H2_phi"
    ]
    for var in H_candidate_variables:
        var_str = "EventInfo.%s -> %s" % (var, var)
        branches.append(var_str)

    # additional variables
    additional_variables = ["HT", "njets", "nBjets"]
    for var in additional_variables:
        var_str = "EventInfo.%s -> %s" % (var, var)
        branches.append(var_str)

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
        var_str = "EventInfo.Jets_%s -> Jets_%s" % (var, var)
        branches.append(var_str)

    return branches
