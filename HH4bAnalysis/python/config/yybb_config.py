from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# yybb analysis chain


def yybb_cfg(flags, smalljetkey, photonkey):
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

    # jets
    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "SmallJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="yybbAnalysisJets",
            minPt=-1,
            maxEta=6,
            truncateAtAmount=-1,  # -1 means keep all
            minimumAmount=-1,  # -1 means ignores this
        )
    )

    # calculate final yybb vars
    cfg.addEventAlgo(
        CompFactory.HH4B.BaselineVarsyybbAlg(
            "FinalVarsyybbAlg",
            smallRContainerInKey="yybbAnalysisJets",
            photonContainerInKey="yybbAnalysisPhotons",
        )
    )

    return cfg


def yybb_branches(flags):
    branches = []
    variables = [
        "N_LOOSE_PHOTONS", "TWO_TIGHTID_PHOTONS", "TWO_ISO_PHOTONS",
        "PASS_RELPT_CUT", "MASSCUT", "myy", "isPassed",
        "LESS_THAN_SIX_CENTRAL_JETS_CUT"
    ]
    for var in variables:
        var_str = "EventInfo.%s -> %s" % (var, var)
        branches.append(var_str)

    kinematics = ["pt", "eta", "phi", "E"]
    pt_ords = ["Leading", "Subleading"]
    for kin in kinematics:
        for pt_ord in pt_ords:
            v = "EventInfo.%s_Photon_%s -> %s_Photon_%s" % (pt_ord, kin, pt_ord, kin)
            j = "EventInfo.%s_Jet_%s -> %s_Jet_%s" % (pt_ord, kin, pt_ord, kin)
            branches += [v]
            branches += [j]

    return branches
