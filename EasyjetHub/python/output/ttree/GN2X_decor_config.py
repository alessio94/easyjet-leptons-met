from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units


def GN2XSelectionJsonToolCfg(flags, name="GN2XSelectionJsonTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("MaxEta", 2.5)
    kwargs.setdefault("MinPt", 20 * Units.GeV)
    kwargs.setdefault("TaggerName", "GN2Xv01")
    kwargs.setdefault("JetAuthor", flags.Analysis.container_names.input.reco10UFOJet)

    from AthenaCommon.Utils.unixtools import find_datafile
    kwargs.setdefault("JsonConfigFile", find_datafile(
        "EasyjetHub/Xbb_lookup_table_prelim_Oct30_2024.json"))

    cfg.setPrivateTools(CompFactory.BTaggingSelectionJsonTool(name, **kwargs))
    return cfg


def GN2XEfficiencyJsonToolCfg(flags, name="GN2XEfficiencyJsonTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("MaxEta", 2.5)
    kwargs.setdefault("MinPt", 20 * Units.GeV)
    kwargs.setdefault("TaggerName", "GN2Xv01")
    kwargs.setdefault("JetAuthor", flags.Analysis.container_names.input.reco10UFOJet)

    from AthenaCommon.Utils.unixtools import find_datafile
    kwargs.setdefault("JsonConfigFile", find_datafile(
        "EasyjetHub/Xbb_lookup_table_prelim_Oct30_2024.json"))

    cfg.setPrivateTools(CompFactory.BTaggingEfficiencyJsonTool(name, **kwargs))
    return cfg


def GN2X_decor_cfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("jetsIn", flags.Analysis.container_names.input.reco10UFOJet)
    # Disabling SF for now to avoid warnings with preliminary json
    kwargs.setdefault("isMC", False)
    kwargs.setdefault("doSyst", flags.Analysis.do_CP_systematics)
    for wp in flags.Analysis.Large_R_jet.GN2X_hbb_wps:
        if kwargs["isMC"]:
            kwargs["EfficiencyTool"] = cfg.popToolsAndMerge(
                GN2XEfficiencyJsonToolCfg(flags, OperatingPoint=wp))
        cfg.addEventAlgo(CompFactory.Easyjet.GN2XTaggingDecoratorAlg(
            "GN2X_" + wp + "_DecoratorAlg",
            SelectionTool=cfg.popToolsAndMerge(
                GN2XSelectionJsonToolCfg(flags, OperatingPoint=wp)),
            WPname=wp,
            **kwargs))

    return cfg
