from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from BJetCalibrationTool.BJetPtCorrectionConfig import makeBJetCalibAnalysisSequence
from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence


def jet_sequence_cfg(
    flags,
    containers,
):
    cfg = ComponentAccumulator()
    jet_sequence = makeJetAnalysisSequence(
        flags.Analysis.DataType,
        jetCollection=containers["inputs"]["reco4PFlowJet"],
        postfix="smallR",
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=not flags.Input.isPHYSLITE,
        enableCutflow=False,
        enableKinematicHistograms=False,
        runJvtUpdate=not flags.Input.isPHYSLITE,
        runNNJvtUpdate=not flags.Input.isPHYSLITE,
        runJvtSelection=not flags.Input.isPHYSLITE,
    )

    btag_calib_file = (
        "xAODBTaggingEfficiency/13TeV/2022-22-13TeV-MC20-CDI-2022-07-28_v1.root"
    )
    # This is the container name that is available in the CDI above
    jet_btag_name = "AntiKt4EMPFlowJets"

    for tagger_wp in flags.Analysis.btag_wps:
        tagger, btag_wp = tagger_wp.split("_", 1)
        makeFTagAnalysisSequence(
            jet_sequence,
            flags.Analysis.DataType,
            jetCollection=jet_btag_name,
            btagWP=btag_wp,
            btagger=tagger,
            generator="Pythia8",
            minPt=20000,
            postfix=f"{jet_btag_name}_{tagger_wp}",
            preselection=None,
            kinematicSelection=True,
            noEfficiency=False,
            legacyRecommendations=False,
            enableCutflow=False,
        )

    # Run this by default, but will fail if muon and btag calib sequences not run
    # TODO: Add a toggle?
    if flags.Analysis.do_muons:
        # Pick a reasonable b-tag selection?
        makeBJetCalibAnalysisSequence(
            flags,
            jet_sequence,
            muonName=containers["outputs"]["muons"],
            btagSelDecor="ftag_select_DL1dv01_FixedCutBEff_77",
        )

    jet_sequence.configure(
        inputName=containers["inputs"]["reco4PFlowJet"],
        outputName=containers["outputs"]["reco4PFlowJet"],
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
def lr_jet_sequence_cfg(flags, containers, lr_jet_type):
    cfg = ComponentAccumulator()
    lr_recojet_sequence = makeJetAnalysisSequence(
        flags.Analysis.DataType,
        jetCollection=containers["inputs"][f"reco10{lr_jet_type}Jet"],
        postfix="largeR" + lr_jet_type,
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=False,
        enableCutflow=False,
        enableKinematicHistograms=False,
    )

    lr_recojet_sequence.configure(
        inputName=containers["inputs"][f"reco10{lr_jet_type}Jet"],
        outputName=containers["outputs"][f"reco10{lr_jet_type}Jet"],
    )

    cfg.addSequence(CompFactory.AthSequencer(lr_recojet_sequence.getName()))
    for alg in lr_recojet_sequence.getGaudiConfig2Components():
        if "JetCalibrationAlg" in alg.getName():
            alg.calibrationTool.IsData = flags.Analysis.DataType == "data"
        cfg.addEventAlgo(alg, lr_recojet_sequence.getName())

    return cfg


# vr = variable R
def vr_jet_sequence_cfg(flags, containers):
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
    vr_jet_btag_name = "AntiKtVR30Rmax4Rmin02TrackJets"
    for tagger_wp in flags.Analysis.vr_btag_wps:
        tagger, btag_wp = tagger_wp.split("_", 1)
        makeFTagAnalysisSequence(
            vr_jet_sequence,
            flags.Analysis.DataType,
            jetCollection=vr_jet_btag_name,
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
        inputName=containers["inputs"]["vrJet"],
        outputName=containers["outputs"]["vrJet"],
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
    containers,
    lr_jet_type,
):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.Easyjet.LargeJetGhostVRJetAssociationAlg(
            f"Large{lr_jet_type}JetGhostVRJetAssociationAlg",
            isMC=flags.Input.isMC,
            LargeJetInKey=containers["outputs"][f"reco10{lr_jet_type}Jet"].replace(
                "%SYS%", "NOSYS"
            ),
            workingPoints=flags.Analysis.vr_btag_wps,
            EventInfoDecorSuffix=lr_jet_type,
        )
    )

    return cfg
