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
            minPt=7_000,
            maxEta=2.7,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
            pTsort=True,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=electronkey,
            containerOutKey="yybbAnalysisElectrons",
            minPt=7_000,
            minEtaVeto=1.37,
            maxEtaVeto=1.52,
            maxEta=2.47,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
            pTsort=True,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="yybbAnalysisJets",
            bTagWPDecorName="ftag_select_DL1dv01_FixedCutBEff_77",
            minPt=-1,
            maxEta=6,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
            maximumAmount=99,
            pTsort=True,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg_No_WP",
            containerInKey=smalljetkey,
            containerOutKey="yybbAnalysisJets_No_WP",
            bTagWPDecorName="",
            minPt=-1,
            maxEta=6,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
            maximumAmount=99,
            pTsort=True,
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.BaselineVarsyybbAlg(
            "FinalVarsyybbAlg",
            photonContainerInKey="yybbAnalysisPhotons",
            smallRContainerInKey="yybbAnalysisJets",
            smallRContainerInKey_No_WP="yybbAnalysisJets_No_WP",
            muonContainerInKey="yybbAnalysisMuons",
            electronContainerInKey="yybbAnalysisElectrons",
        )
    )

    return cfg


def yybb_branches(flags):
    branches = []

    variables = [
        "N_LOOSE_PHOTONS", "TWO_TIGHTID_PHOTONS", "TWO_ISO_PHOTONS",
        "PASS_RELPT_CUT", "MASSCUT", "myy", "isPassed","N_LEPTONS_CUT",
        "LESS_THAN_SIX_CENTRAL_JETS","EXACTLY_TWO_B_JETS"
    ]

    if (flags.Analysis.do_yybb_cutflow):
        for var in variables:
            var_str = "EventInfo.%s -> %s" % (var, var)
            branches.append(var_str)

    kinematics = ["pt", "eta", "phi", "E"]
    pt_ords = ["Leading", "Subleading"]
    particles = ["Photon", "Jet"]

    for pt_ord in pt_ords:
        for p in particles:
            for kin in kinematics:
                v = "EventInfo.%s_%s_%s -> %s_%s_%s" % \
                    (pt_ord, p, kin, pt_ord, p, kin)
                branches += [v]

    return branches
