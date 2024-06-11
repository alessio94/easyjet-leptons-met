from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def orth_cfg(flags, smalljetkey, photonkey):
    cfg = ComponentAccumulator()

    PhotonWPLabel = f'{flags.Analysis.orthogonality.HHPhoton.ID}_{flags.Analysis.orthogonality.HHPhoton.Iso}' # noqa
    BTagWPLabel = f'ftag_select_{flags.Analysis.orthogonality.HHBjet.btag_wp}'

    cfg.addEventAlgo(
        CompFactory.Easyjet.PhotonSelectorAlg(
            "OrthPhotonSelectorAlg",
            containerInKey=PhotonWPLabel + photonkey,
            containerOutKey="OrthAnalysisPhotons_%SYS%",
            photonSF_WP=PhotonWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "OrthJetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="OrthAnalysisBJets_%SYS%",
            bTagWPDecorName=BTagWPLabel,
            selectBjet=True,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.OrthogonalityAlg(
            "OrthogonalityAlg",
            photons="OrthAnalysisPhotons_%SYS%",
            bjets="OrthAnalysisBJets_%SYS%",
        )
    )

    return cfg


def orth_branches(flags):
    branches = []

    orth_vars = ["orth_pass_bbyy", "orth_pass_bbbb", "orth_pass_bbtt"]
    for var in orth_vars:
        branches += [f"EventInfo.{var}_%SYS% -> {var}_%SYS%"]

    return branches
