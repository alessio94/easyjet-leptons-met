from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def muon_decor_config(flags, **kwargs):
    cfg = ComponentAccumulator()

    muoncoll = flags.Analysis.container_names.input.muons
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonTrackDecoratorAlg(
            f"MuonDecor_{muoncoll}",
            muonsIn=muoncoll,
            isMC=flags.Input.isMC,
            doRetrieveTrack=flags.Analysis.Muon.do_track_decoration,
            doLLP1=(flags.Input.ProcessingTags == ["StreamDAOD_LLP1"]),
            **kwargs
        )
    )

    return cfg
