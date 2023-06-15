from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# yybb analysis chain


def yybb_cfg(flags, smalljetkey, photonkey):
    cfg = ComponentAccumulator()

    # Take leading two photons - independent of btag working point
    cfg.addEventAlgo(
        CompFactory.Easyjet.PhotonSelectorAlg(
            "PhotonSelectorAlg",
            containerInKey=photonkey,
            containerOutKey="yybbAnalysisPhotons",
            LeadPho_ptOverMyy_min=0.35,  # pT/myy
            SubleadPho_ptOverMyy_min=0.25,  # pT/myy
            etaBounds=[
                1.37,
                1.52,
                2.37,
            ],  # photon eta must be: < 1.37, or between 1.52 and 2.37
            truncateAtAmount=2,  # -1 means keep all
            minimumAmount=2,  # -1 means ignores this
            pTsort=True,
        )
    )

    for btag_wp in flags.Analysis.btag_wps:
        # get the 2 leading small R jets
        cfg.addEventAlgo(
            CompFactory.Easyjet.JetSelectorAlg(
                "SmallJetSelectorAlg_" + btag_wp,
                containerInKey=smalljetkey,
                containerOutKey="yybbAnalysisJets_" + btag_wp,
                bTagWP=btag_wp,  # empty string: "" ignores btagging
                minPt=25_000,
                maxEta=2.5,
                truncateAtAmount=2,  # -1 means keep all
                minimumAmount=2,  # -1 means ignores this
                pTsort=True,
            )
        )

        cfg.addEventAlgo(
            CompFactory.HH4B.JetPairingAlg(
                "JetPairingAlg_" + btag_wp,
                containerInKey="yybbAnalysisJets_" + btag_wp,
                containerOutKey="pairedyybbAnalysisJets_" + btag_wp,
                pairingStrategy="minDeltaR",  # so far only minDeltaR
            )
        )

        # calculate final yybb vars
        cfg.addEventAlgo(
            CompFactory.HH4B.BaselineVarsyybbAlg(
                "FinalVarsyybbAlg_" + btag_wp,
                smallRContainerInKey="pairedyybbAnalysisJets_" + btag_wp,
                photonContainerInKey="yybbAnalysisPhotons",
                bTagWP=btag_wp,
            )
        )

    return cfg


def yybb_branches(flags):
    branches = []

    for btag_wp in flags.Analysis.btag_wps:
        yybb_vars = [
            # Jets
            "DeltaR12",
            # Leading photon
            "Leading_Photon_pt",
            "Leading_Photon_eta",
            "Leading_Photon_phi",
            "Leading_Photon_E",
            # Subleading photon
            "Subleading_Photon_pt",
            "Subleading_Photon_eta",
            "Subleading_Photon_phi",
            "Subleading_Photon_E",
            # Diphoton system
            "myy",
        ]

        for var in yybb_vars:
            branch_name = f"EventInfo.{var}_{btag_wp} -> {btag_wp}_{var}"
            branches += [branch_name]

    return branches
