from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.steering.analysis_configuration import (
    get_trigger_legs_scale_factor_list)


def MuonSelectorAlgCfg(flags, name="MuonSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("isMC", flags.Input.isMC)
    kwargs.setdefault("checkOR", flags.Analysis.do_overlap_removal)
    kwargs.setdefault("doTTVA", flags.Analysis.Muon.trackSelection)
    kwargs.setdefault("muonTriggerSF",
                      get_trigger_legs_scale_factor_list(flags, 'Muon'))

    cfg.addEventAlgo(CompFactory.Easyjet.MuonSelectorAlg(name, **kwargs))
    return cfg


def ElectronSelectorAlgCfg(flags, name="ElectronSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("isMC", flags.Input.isMC)
    kwargs.setdefault("checkOR", flags.Analysis.do_overlap_removal)
    kwargs.setdefault("eleTriggerSF",
                      get_trigger_legs_scale_factor_list(flags, 'Electron'))

    cfg.addEventAlgo(CompFactory.Easyjet.ElectronSelectorAlg(name, **kwargs))
    return cfg


def TauSelectorAlgCfg(flags, name="TauSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("isMC", flags.Input.isMC)
    kwargs.setdefault("checkOR", flags.Analysis.do_overlap_removal)
    kwargs.setdefault("tauTriggerSF",
                      get_trigger_legs_scale_factor_list(flags, 'Tau'))

    cfg.addEventAlgo(CompFactory.Easyjet.TauSelectorAlg(name, **kwargs))
    return cfg


def PhotonSelectorAlgCfg(flags, name="PhotonSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("isMC", flags.Input.isMC)
    kwargs.setdefault("checkOR", flags.Analysis.do_overlap_removal)

    cfg.addEventAlgo(CompFactory.Easyjet.PhotonSelectorAlg(name, **kwargs))
    return cfg


def JetSelectorAlgCfg(flags, name="JetSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("useFJVT", flags.Analysis.small_R_jet.useFJvt)
    kwargs.setdefault("checkOR", flags.Analysis.do_overlap_removal)

    cfg.addEventAlgo(CompFactory.Easyjet.JetSelectorAlg(name, **kwargs))
    return cfg
