from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonSelectorAlgCfg(flags, name="MuonSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    muon_WPs = [f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}']
    consider_extra_wp = any(len(wp) == 4 for wp in flags.Analysis.Muon.extra_wps)

    for wp in flags.Analysis.Muon.extra_wps:
        if len(wp) == 4:
            muon_WPs += [f'{wp[0]}_{wp[1]}_{wp[2]}_{wp[3]}'.replace('.', 'p')]
        elif consider_extra_wp:
            wp_str = f"{wp[0]}_{wp[1]}_{flags.Analysis.Muon.maxD0Significance}"
            + f"_{flags.Analysis.Muon.maxDeltaZ0SinTheta}"
            muon_WPs += [wp_str.replace('.', 'p')]
        else:
            muon_WPs += [f'{wp[0]}_{wp[1]}']
    kwargs.setdefault("muonWPs", muon_WPs)
    kwargs.setdefault("muonAmount", flags.Analysis.Muon.amount)

    if not flags.Analysis.Muon.do_thinning:
        kwargs.setdefault("baselineSelectionName", "")

    cfg.addEventAlgo(CompFactory.Easyjet.MuonSelectorAlg(name, **kwargs))
    return cfg


def ElectronSelectorAlgCfg(flags, name="ElectronSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    ele_WPs = [f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}']
    ele_WPs += [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Electron.extra_wps]
    kwargs.setdefault("eleWPs", ele_WPs)
    kwargs.setdefault("electronAmount", flags.Analysis.Electron.amount)

    if not flags.Analysis.Electron.do_thinning:
        kwargs.setdefault("baselineSelectionName", "")

    cfg.addEventAlgo(CompFactory.Easyjet.ElectronSelectorAlg(name, **kwargs))
    return cfg


def TruthElectronSelectorAlgCfg(flags, name="TruthElectronSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("decoration",
                      flags.Analysis.container_names.input.truthelectrons
                      + ".isTruthElectron")
    kwargs.setdefault("decorOutName", "TruthEvents.nElectrons")

    cfg.addEventAlgo(CompFactory.Easyjet.TruthElectronSelectorAlg(name, **kwargs))
    return cfg


def TruthMuonSelectorAlgCfg(flags, name="TruthMuonSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("decoration",
                      flags.Analysis.container_names.input.truthmuons
                      + ".isTruthMuon")
    kwargs.setdefault("decorOutName", "TruthEvents.nMuons")

    cfg.addEventAlgo(CompFactory.Easyjet.TruthMuonSelectorAlg(name, **kwargs))
    return cfg


def LeptonOrderingAlgCfg(flags, name="LeptonOrderingAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("leptonAmount", flags.Analysis.Lepton.amount)

    cfg.addEventAlgo(CompFactory.Easyjet.LeptonOrderingAlg(name, **kwargs))
    return cfg


def TruthLeptonOrderingAlgCfg(flags, name="TruthLeptonOrderingAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("leptonAmount", flags.Analysis.Lepton.amount)
    kwargs.setdefault("isTruthElectronDecoration",
                      flags.Analysis.container_names.input.truthelectrons
                      + ".isTruthElectron")
    kwargs.setdefault("isTruthMuonDecoration",
                      flags.Analysis.container_names.input.truthmuons
                      + ".isTruthMuon")

    cfg.addEventAlgo(CompFactory.Easyjet.TruthLeptonOrderingAlg(name, **kwargs))
    return cfg


def TauSelectorAlgCfg(flags, name="TauSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    tau_WPs = [flags.Analysis.Tau.ID]
    tau_WPs += flags.Analysis.Tau.extra_wps
    kwargs.setdefault("tauWPs", tau_WPs)
    kwargs.setdefault("keepAntiTaus",
                      flags.Analysis.OverlapRemoval.doTauAntiTauJet)
    kwargs.setdefault("tauAmount", flags.Analysis.Tau.amount)

    if not flags.Analysis.Tau.do_thinning:
        kwargs.setdefault("baselineSelectionName", "")

    cfg.addEventAlgo(CompFactory.Easyjet.TauSelectorAlg(name, **kwargs))
    return cfg


def PhotonSelectorAlgCfg(flags, name="PhotonSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    ph_WPs = [f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}']
    ph_WPs += [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Photon.extra_wps]
    kwargs.setdefault("photonWPs", ph_WPs)
    kwargs.setdefault("photonAmount", flags.Analysis.Photon.amount)

    if not flags.Analysis.Photon.do_thinning:
        kwargs.setdefault("baselineSelectionName", "")

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


def TruthJetSelectorAlgCfg(flags, name="TruthJetSelectorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("bjetAmount", flags.Analysis.Small_R_jet.amount_bjet)
    kwargs.setdefault("cjetAmount", flags.Analysis.Small_R_jet.amount_cjet)

    cfg.addEventAlgo(CompFactory.Easyjet.TruthJetSelectorAlg(name, **kwargs))
    return cfg
