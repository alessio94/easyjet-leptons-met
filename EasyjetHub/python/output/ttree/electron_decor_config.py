from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def electron_decor_config(flags, **kwargs):
    cfg = ComponentAccumulator()

    electroncoll = flags.Analysis.container_names.input.electrons
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronDecoratorAlg(
            f"ElectronDecor_{electroncoll}",
            electronsIn=electroncoll,
            isMC=flags.Input.isMC,
            doRetrieveTrack=flags.Analysis.Electron.do_track_decoration,
            **kwargs
        )
    )

    return cfg
