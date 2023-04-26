from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
from BJetCalibrationTool.BJetPtCorrectionConfig import makeBJetCalibAnalysisSequence


def jet_sequence_cfg(
    flags,
    datatype,
    incontainername,
    outcontainername,
    muoncontainername,
    is_daod_physlite,
    do_bjet_ptcalib,
):
    cfg = ComponentAccumulator()
    jet_sequence = makeJetAnalysisSequence(
        datatype,
        jetCollection=incontainername,
        postfix="smallR",
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=not is_daod_physlite,
        enableCutflow=False,
        enableKinematicHistograms=False,
        runJvtUpdate=not is_daod_physlite,
        runNNJvtUpdate=not is_daod_physlite,
        runJvtSelection=not is_daod_physlite,
    )

    btag_calib_file = (
        "xAODBTaggingEfficiency/13TeV/2022-22-13TeV-MC20-CDI-2022-07-28_v1.root"
    )
    # This is the container name that is available in the CDI aboce
    jet_btag_containername = "AntiKt4EMPFlowJets"

    # TODO: no DL1d branches in PHYSLITE yet
    if is_daod_physlite:
        working_points = [
            wp.replace("DL1dv00", "DL1r") for wp in flags.Analysis.btag_wps
        ]
    else:
        working_points = flags.Analysis.btag_wps

    for tagger_wp in working_points:
        tagger, btag_wp = tagger_wp.split("_", 1)
        makeFTagAnalysisSequence(
            jet_sequence,
            datatype,
            jetCollection=jet_btag_containername,
            btagWP=btag_wp,
            btagger=tagger,
            generator="Pythia8",
            minPt=20000,
            postfix=f"{jet_btag_containername}_{tagger_wp}",
            preselection=None,
            kinematicSelection=True,
            noEfficiency=False,
            legacyRecommendations=False,
            enableCutflow=False,
        )

    if do_bjet_ptcalib:
        # Pick a reasonable b-tag selection?
        makeBJetCalibAnalysisSequence(
            flags,
            jet_sequence,
            muonName=muoncontainername,
            btagSelDecor="ftag_select_DL1dv00_FixedCutBEff_77",
        )

    jet_sequence.configure(
        inputName=incontainername,
        outputName=outcontainername,
    )

    cfg.addSequence(CompFactory.AthSequencer(jet_sequence.getName()))
    # Hack until this is merged:
    # https://gitlab.cern.ch/atlas/athena/-/merge_requests/54939]
    for alg in jet_sequence.getGaudiConfig2Components():
        if "FTagSelectionAlg" in alg.getName():
            alg.selectionTool.FlvTagCutDefinitionsFileName = btag_calib_file
        if "FTagEfficiencyScaleFactorAlg" in alg.getName():
            alg.efficiencyTool.ScaleFactorFileName = btag_calib_file

        cfg.addEventAlgo(alg, jet_sequence.getName())

    return cfg


# lr = large-R
def lr_jet_sequence_cfg(flags, datatype, incontainername, outcontainername):
    cfg = ComponentAccumulator()
    # with ConfigurableCABehavior(False):
    lr_recojet_sequence = makeJetAnalysisSequence(
        datatype,
        jetCollection=incontainername,
        postfix="largeR",
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=False,
        enableCutflow=False,
        enableKinematicHistograms=False,
        largeRMass="Comb",
    )

    lr_recojet_sequence.configure(
        inputName=incontainername, outputName=outcontainername
    )

    cfg.addSequence(CompFactory.AthSequencer(lr_recojet_sequence.getName()))
    for alg in lr_recojet_sequence.getGaudiConfig2Components():
        if "JetCalibrationAlg" in alg.getName():
            alg.calibrationTool.IsData = datatype == "data"
        cfg.addEventAlgo(alg, lr_recojet_sequence.getName())

    return cfg


def lr_ufo_jet_sequence_cfg(flags, datatype, incontainername, outcontainername):
    cfg = ComponentAccumulator()
    # with ConfigurableCABehavior(False):
    lr_ufo_recojet_sequence = makeJetAnalysisSequence(
        datatype,
        jetCollection=incontainername,
        postfix="largeRUFO",
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=False,
        enableCutflow=False,
        enableKinematicHistograms=False,
        largeRMass="Comb",
    )

    lr_ufo_recojet_sequence.configure(
        inputName=incontainername, outputName=outcontainername
    )

    cfg.addSequence(CompFactory.AthSequencer(lr_ufo_recojet_sequence.getName()))
    for alg in lr_ufo_recojet_sequence.getGaudiConfig2Components():
        if "JetCalibrationAlg" in alg.getName():
            alg.calibrationTool.IsData = datatype == "data"
        cfg.addEventAlgo(alg, lr_ufo_recojet_sequence.getName())

    return cfg


# vr = variable R
def vr_jet_sequence_cfg(flags, datatype, incontainername, outcontainername):
    cfg = ComponentAccumulator()

    def create_vr_jet_sequence():
        from AnaAlgorithm.AnaAlgSequence import AnaAlgSequence
        from AnaAlgorithm.DualUseConfig import createAlgorithm

        postfix = "VR"
        seq = AnaAlgSequence("JetAnalysisSequence" + postfix)
        # Set up an algorithm that makes a view container
        alg = createAlgorithm(
            "CP::AsgViewFromSelectionAlg", "VRJetSelectionAlg" + postfix
        )
        seq.append(
            alg,
            inputPropName="input",
            outputPropName="output",
            stageName="selection",
            dynConfig={},
        )
        return seq

    vr_jet_sequence = create_vr_jet_sequence()

    btag_calib_file = (
        "xAODBTaggingEfficiency/13TeV/2021-22-13TeV-MC16-CDI-2021-12-02_v2.root"
    )
    # This is the container name that is available in the CDI aboce
    vr_jet_btag_containername = "AntiKtVR30Rmax4Rmin02TrackJets"
    for tagger_wp in flags.Analysis.vr_btag_wps:
        tagger, btag_wp = tagger_wp.split("_", 1)
        makeFTagAnalysisSequence(
            vr_jet_sequence,
            datatype,
            jetCollection=vr_jet_btag_containername,
            btagWP=btag_wp,
            btagger=tagger,
            minPt=10e3,
            postfix=btag_wp,
            preselection=None,
            kinematicSelection=True,
            noEfficiency=False,
            legacyRecommendations=False,
            enableCutflow=False,
        )

    vr_jet_sequence.configure(
        inputName=incontainername,
        outputName=outcontainername,
    )

    cfg.addSequence(CompFactory.AthSequencer(vr_jet_sequence.getName()))
    # Hack until this is merged:
    # https://gitlab.cern.ch/atlas/athena/-/merge_requests/54939]
    for alg in vr_jet_sequence.getGaudiConfig2Components():
        if "FTagSelectionAlg" in alg.getName():
            alg.selectionTool.FlvTagCutDefinitionsFileName = btag_calib_file
        if "FTagEfficiencyScaleFactorAlg" in alg.getName():
            alg.efficiencyTool.ScaleFactorFileName = btag_calib_file

        cfg.addEventAlgo(alg, vr_jet_sequence.getName())

    return cfg


def lr_jet_ghost_vr_jet_association_cfg(
    flags,
    inlrjet_containername,
):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.LargeJetGhostVRJetAssociationAlg(
            "LargeJetGhostVRJetAssociationAlg",
            isMC=flags.Input.isMC,
            LargeJetInKey=inlrjet_containername,
            workingPoints=flags.Analysis.vr_btag_wps,
        )
    )

    return cfg


def lr_jet_ghost_vr_jet_association_branches(flags, inlrjet_containername):
    branches = []

    vr_vars = [
        "goodVRTrackJets",
        "minRelativeDeltaRToVRJet",
        "leadingVRTrackJetsPt",
        "leadingVRTrackJetsEta",
        "leadingVRTrackJetsPhi",
        "leadingVRTrackJetsM",
        "leadingVRTrackJetsDeltaR12",
        "leadingVRTrackJetsDeltaR13",
        "leadingVRTrackJetsDeltaR32",
    ]
    for var in vr_vars:
        branches += [f"{inlrjet_containername}.{var} -> recojet_antikt10_%SYS%_{var}"]
    branches += [
        f"{inlrjet_containername}.leadingVRTrackJetsBtag_{wp} -> "
        f"recojet_antikt10_%SYS%_leadingVRTrackJetsBtag_{wp}"
        for wp in flags.Analysis.vr_btag_wps
    ]
    branches += [
        "EventInfo.passRelativeDeltaRToVRJetCut -> passRelativeDeltaRToVRJetCut"
    ]
    if flags.Input.isMC:
        branches += [
            f"{inlrjet_containername}.VRTrackJetsTruthLabel -> "
            f"HadronConeExclTruthLabelID_%SYS%"
        ]

    return branches


def lr_ufo_jet_ghost_vr_jet_association_cfg(
    flags,
    inlrufojet_containername,
):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.LargeJetGhostVRJetAssociationAlg(
            "LargeUFOJetGhostVRJetAssociationAlg",
            isMC=flags.Input.isMC,
            LargeJetInKey=inlrufojet_containername,
            workingPoints=flags.Analysis.vr_btag_wps,
        )
    )

    return cfg


def lr_ufo_jet_ghost_vr_jet_association_branches(flags, inlrufojet_containername):
    branches = []

    ufo_vars = [
        "goodVRTrackJets",
        "minRelativeDeltaRToVRJet",
        "leadingVRTrackJetsPt",
        "leadingVRTrackJetsEta",
        "leadingVRTrackJetsPhi",
        "leadingVRTrackJetsM",
        "leadingVRTrackJetsDeltaR12",
        "leadingVRTrackJetsDeltaR13",
        "leadingVRTrackJetsDeltaR32",
    ]
    for var in ufo_vars:
        branches += [
            f"{inlrufojet_containername}.{var} -> recoUFOjet_antikt10_%SYS%_{var}"
        ]
    branches += [
        f"{inlrufojet_containername}.leadingVRTrackJetsBtag_{wp} -> "
        f"recoUFOjet_antikt10_%SYS%_leadingVRTrackJetsBtag_{wp}"
        for wp in flags.Analysis.vr_btag_wps
    ]
    if flags.Input.isMC:
        branches += [
            f"{inlrufojet_containername}.VRTrackJetsTruthLabel -> "
            f"UFO_R10_HadronConeExclTruthLabelID_%SYS%"
        ]

    return branches
