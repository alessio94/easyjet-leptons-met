from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonSelectorAlgCfg(flags, name="MuonSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    muon_WPs = [f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}']
    muon_WPs += [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Muon.extra_wps]
    kwargs.setdefault("muonWPs", muon_WPs)
    kwargs.setdefault("muonAmount", flags.Analysis.Muon.amount)

    cfg.addEventAlgo(CompFactory.Easyjet.MuonSelectorAlg(name, **kwargs))
    return cfg


def ElectronSelectorAlgCfg(flags, name="ElectronSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    ele_WPs = [f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}']
    ele_WPs += [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Electron.extra_wps]
    kwargs.setdefault("eleWPs", ele_WPs)
    kwargs.setdefault("electronAmount", flags.Analysis.Electron.amount)

    cfg.addEventAlgo(CompFactory.Easyjet.ElectronSelectorAlg(name, **kwargs))
    return cfg


def LeptonOrderingAlgCfg(flags, name="LeptonOrderingAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("leptonAmount", flags.Analysis.Lepton.amount)

    cfg.addEventAlgo(CompFactory.Easyjet.LeptonOrderingAlg(name, **kwargs))
    return cfg


def TauSelectorAlgCfg(flags, name="TauSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    tau_WPs = [flags.Analysis.Tau.ID]
    tau_WPs += flags.Analysis.Tau.extra_wps
    kwargs.setdefault("tauWPs", tau_WPs)
    kwargs.setdefault("keepAntiTaus",
                      flags.Analysis.OverlapRemoval.doTauAntiTauJet)
    kwargs.setdefault("tauAmount", flags.Analysis.Tau.amount)

    cfg.addEventAlgo(CompFactory.Easyjet.TauSelectorAlg(name, **kwargs))
    return cfg


def PhotonSelectorAlgCfg(flags, name="PhotonSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    ph_WPs = [f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}']
    ph_WPs += [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Photon.extra_wps]
    kwargs.setdefault("photonWPs", ph_WPs)
    kwargs.setdefault("photonAmount", flags.Analysis.Photon.amount)

    cfg.addEventAlgo(CompFactory.Easyjet.PhotonSelectorAlg(name, **kwargs))
    return cfg


def JetSelectorAlgCfg(flags, name="JetSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    isSmallRJet = "AntiKt4" in kwargs["containerInKey"]
    isLargeRJet = "AntiKt10" in kwargs["containerInKey"]
    if kwargs.get("bTagWPDecorName", ""):
        kwargs.setdefault("bjetAmount", flags.Analysis.Small_R_jet.amount_bjet)
    if (isSmallRJet and flags.Analysis.Small_R_jet.runBJetPtCalib) or \
       (not isSmallRJet and flags.Analysis.Large_R_jet.runMuonJetPtCorr):
        kwargs.setdefault("nmuons", "n_muons_%SYS%")
    if isLargeRJet and not flags.Analysis.Large_R_jet.do_thinning:
        kwargs.setdefault("baselineSelectionName", "")

    cfg.addEventAlgo(CompFactory.Easyjet.JetSelectorAlg(name, **kwargs))
    return cfg
