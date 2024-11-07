from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

TAGGER_CONFIGS = {
    "ANN": {
        "calib_area": "Winter2024_R22_PreRecs/JSSWTopTaggerANN/",
        "config_file": "JSSANN{wp}Tagger_AntiKt10UFOCSSKSoftDrop_Mar23pol3.dat"
    },
    "DNN": {
        "calib_area": "Winter2024_R22_PreRecs/JSSWTopTaggerDNN/",
        "config_file": "JSSDNN{wp}Tagger_AntiKt10UFOCSSKSoftDrop_pol3.dat"
    }
}


def wtagger_cfg(flags, WP, **kwargs):
    cfg = ComponentAccumulator()

    jetcoll = flags.Analysis.container_names.input.reco10UFOJet
    wtag_type = flags.Analysis.Large_R_jet.wtag_type
    tagger_config = TAGGER_CONFIGS.get(wtag_type)

    kwargs.setdefault("ContainerName", jetcoll)
    kwargs.setdefault("ConfigFile", tagger_config["config_file"].format(wp=WP))
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
    wtag_wp = flags.Analysis.Large_R_jet.wtag_wp
    wtag_extra_wps = list(flags.Analysis.Large_R_jet.wtag_extra_wps or [])

    for wp in [wtag_wp] + wtag_extra_wps:
        cfg.addEventAlgo(
            CompFactory.Easyjet.WTaggingDecoratorAlg(
                f"WTagDecor{wp}_{jetcoll}",
                jetsIn=jetcoll,
                Wtagger=cfg.popToolsAndMerge(wtagger_cfg(flags, wp)),
                **kwargs,
            )
        )

    return cfg
