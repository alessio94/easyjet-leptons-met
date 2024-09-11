from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.output.ttree.TruthClassificationToolConfig import (
    TruthClassificationToolConfig)


def electron_decor_config(flags, **kwargs):
    cfg = ComponentAccumulator()

    electroncoll = flags.Analysis.container_names.input.electrons
    doRetrieveTrack = flags.Analysis.Electron.doRetrieveTrack
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronDecoratorAlg(
            f"ElectronDecor_{electroncoll}",
            electronsIn=electroncoll,
            isMC=flags.Input.isMC,
            doRetrieveTrack=doRetrieveTrack,
            truthClassificationTool=cfg.popToolsAndMerge(
                TruthClassificationToolConfig(flags)),
            **kwargs
        )
    )

    return cfg
