from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

TAGGER_CONFIGS = {
    "ANN": {
        "calib_area": "JSSWTopTaggerANN/Rel21/March2023/",
        "config_file": "JSSANN{wp}Tagger_AntiKt10UFOCSSKSoftDrop_Mar23pol3.dat"
    },
    "DNN": {
        "calib_area": "JSSWTopTaggerDNN/Rel21/February2022/",
        "config_file": "DNNTagger_AntiKt10UFOSD_TopContained{wp}_Oct30.dat"
    }
}


def wtagger_cfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    jetcoll = flags.Analysis.container_names.input.reco10UFOJet
    wtag_type = flags.Analysis.Large_R_jet.wtag_type
    wtag_wp = flags.Analysis.Large_R_jet.wtag_wp
    tagger_config = TAGGER_CONFIGS.get(wtag_type)

    kwargs.setdefault("ContainerName", jetcoll)
    kwargs.setdefault("ConfigFile", tagger_config["config_file"].format(wp=wtag_wp))
    kwargs.setdefault("CalibArea", tagger_config["calib_area"])
    kwargs.setdefault("IsMC", flags.Input.isMC)

    if wtag_type == "ANN":
        cfg.setPrivateTools(CompFactory.JSSWTopTaggerANN(**kwargs))
    elif wtag_type == "DNN":
        cfg.setPrivateTools(CompFactory.JSSWTopTaggerDNN(**kwargs))
    return cfg


def wtag_decor_cfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    jetcoll = flags.Analysis.container_names.input.reco10UFOJet
    cfg.addEventAlgo(
        CompFactory.Easyjet.WTaggingDecoratorAlg(
            f"WTagDecor_{jetcoll}",
            jetsIn=jetcoll,
            Wtagger=cfg.popToolsAndMerge(wtagger_cfg(flags)),
            **kwargs,
        )
    )

    return cfg
