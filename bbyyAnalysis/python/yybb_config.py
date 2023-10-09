from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# yybb analysis chain


def yybb_cfg(flags, smalljetkey, photonkey, muonkey, electronkey):
    cfg = ComponentAccumulator()

    # photons
    cfg.addEventAlgo(
        CompFactory.Easyjet.PhotonSelectorAlg(
            "PhotonSelectorAlg",
            containerInKey=photonkey,
            containerOutKey="yybbAnalysisPhotons",
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
            containerOutKey="yybbAnalysisMuons",
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
            containerOutKey="yybbAnalysisElectrons",
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
            containerOutKey="yybbAnalysisJets_BTag",
            bTagWPDecorName="ftag_select_DL1dv01_FixedCutBEff_77",
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
            containerOutKey="yybbAnalysisJets",
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
            "FinalVarsyybbAlg",
            photonContainerInKey="yybbAnalysisPhotons",
            smallRJets_BTag_ContainerInKey="yybbAnalysisJets_BTag",
            smallRJets_ContainerInKey="yybbAnalysisJets",
            muonContainerInKey="yybbAnalysisMuons",
            electronContainerInKey="yybbAnalysisElectrons",
        )
    )

    return cfg


def yybb_branches(flags):
    branches = []

    variables = [
        "N_LOOSE_PHOTONS", "TWO_TIGHTID_PHOTONS", "TWO_ISO_PHOTONS",
        "PASS_RELPT_CUT", "MASSCUT", "isPassed","N_LEPTONS_CUT",
        "LESS_THAN_SIX_CENTRAL_JETS","EXACTLY_TWO_B_JETS"
    ]

    if (flags.Analysis.do_yybb_cutflow):
        for var in variables:
            var_str = "EventInfo.%s -> %s" % (var, var)
            branches.append(var_str)

    # Photons
    photon_kinematics = ["pt", "eta", "phi", "E"]
    pt_ords = ["Leading", "Subleading"]
    for pt_ord in pt_ords:
        for kin in photon_kinematics:
            v = "EventInfo.%s_Photon_%s -> %s_Photon_%s" % \
                (pt_ord,  kin, pt_ord,  kin)
            branches += [v]

    diphoton_variables = ["myy", "pTyy", "dRyy", "Etayy", "Phiyy"]
    for var in diphoton_variables:
        var_str = "EventInfo.%s -> %s" % (var, var)
        branches.append(var_str)

    # BJets
    btag_variables = ["pt", "eta", "phi", "E"]
    btag_pt_ords = ["B1", "B2"]
    for pt_ord in btag_pt_ords:
        for kin in btag_variables:
            v = "EventInfo.Jet_%s_%s -> Jet_%s_%s" % \
                (kin, pt_ord, kin, pt_ord)
            branches += [v]

    dibjet_variables = ["mBB", "pTBB", "dRBB", "EtaBB", "PhiBB"]
    for var in dibjet_variables:
        var_str = "EventInfo.%s -> %s" % (var, var)
        branches.append(var_str)

    # di-higgs variables
    dihiggs_variables = [
        "mBByy", "pTBByy", "EtaBByy", "PhiBByy", "dRBByy", "mBByy_star"
    ]
    for var in dihiggs_variables:
        var_str = "EventInfo.%s -> %s" % (var, var)
        branches.append(var_str)

    # mva variables
    mva_variables = ["Ht"]
    for var in mva_variables:
        var_str = "EventInfo.%s -> %s" % (var, var)
        branches.append(var_str)

    return branches
