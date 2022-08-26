from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence


def JetAnalysisSequenceCfg(
    flags,
    dataType,
    inputContainerName,
    outputContainerName,
    workingPoints,
    is_daod_physlite,
):
    cfg = ComponentAccumulator()
    jetSequence = makeJetAnalysisSequence(
        dataType,
        jetCollection=inputContainerName,
        postfix="smallR",
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=not is_daod_physlite,
        enableCutflow=False,
        enableKinematicHistograms=False,
        runJvtUpdate=True,
        runJvtSelection=True,
    )

    bTagCalibFile = (
        "xAODBTaggingEfficiency/13TeV/2022-22-13TeV-MC20-CDI-2022-07-28_v1.root"
    )
    # This is the container name that is available in the CDI aboce
    jetBTagContainerName = "AntiKt4EMPFlowJets"

    for tagger_wp in workingPoints:
        tagger, btag_wp = tagger_wp.split("_", 1)
        makeFTagAnalysisSequence(
            jetSequence,
            dataType,
            jetCollection=jetBTagContainerName,
            btagWP=btag_wp,
            btagger=tagger,
            generator="Pythia8",
            minPt=20000,
            postfix="",
            preselection=None,
            kinematicSelection=False,
            noEfficiency=False,
            legacyRecommendations=False,
            enableCutflow=False,
        )

    jetSequence.configure(
        inputName=inputContainerName,
        outputName=outputContainerName,
    )

    cfg.addSequence(CompFactory.AthSequencer(jetSequence.getName()))
    # Hack until this is merged:
    # https://gitlab.cern.ch/atlas/athena/-/merge_requests/54939]
    for alg in jetSequence.getGaudiConfig2Components():
        if "FTagSelectionAlg" in alg.getName():
            alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
        if "FTagEfficiencyScaleFactorAlg" in alg.getName():
            alg.efficiencyTool.ScaleFactorFileName = bTagCalibFile

        cfg.addEventAlgo(alg, jetSequence.getName())

    return cfg


def FatJetAnalysisSequenceCfg(flags, dataType, inputContainerName, outputContainerName):
    cfg = ComponentAccumulator()
    # with ConfigurableRun3Behavior(False):
    largeRrecojetSequence = makeJetAnalysisSequence(
        dataType,
        jetCollection=inputContainerName,
        postfix="largeR",
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=False,
        enableCutflow=False,
        enableKinematicHistograms=False,
        largeRMass="Comb",
    )

    largeRrecojetSequence.configure(
        inputName=inputContainerName, outputName=outputContainerName
    )

    cfg.addSequence(CompFactory.AthSequencer(largeRrecojetSequence.getName()))
    for alg in largeRrecojetSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, largeRrecojetSequence.getName())

    return cfg


def VRJetAnalysisSequenceCfg(
    flags, dataType, inputContainerName, outputContainerName, workingPoints
):
    cfg = ComponentAccumulator()

    def createVRJetSequence():
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

    vrJetSequence = createVRJetSequence()

    bTagCalibFile = (
        "xAODBTaggingEfficiency/13TeV/2021-22-13TeV-MC16-CDI-2021-12-02_v2.root"
    )
    # This is the container name that is available in the CDI aboce
    vrJetBTagContainerName = "AntiKtVR30Rmax4Rmin02TrackJets"
    for tagger_wp in workingPoints:
        tagger, btag_wp = tagger_wp.split("_", 1)
        makeFTagAnalysisSequence(
            vrJetSequence,
            dataType,
            jetCollection=vrJetBTagContainerName,
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

    vrJetSequence.configure(
        inputName=inputContainerName,
        outputName=outputContainerName,
    )

    cfg.addSequence(CompFactory.AthSequencer(vrJetSequence.getName()))
    # Hack until this is merged:
    # https://gitlab.cern.ch/atlas/athena/-/merge_requests/54939]
    for alg in vrJetSequence.getGaudiConfig2Components():
        if "FTagSelectionAlg" in alg.getName():
            alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
        if "FTagEfficiencyScaleFactorAlg" in alg.getName():
            alg.efficiencyTool.ScaleFactorFileName = bTagCalibFile

        cfg.addEventAlgo(alg, vrJetSequence.getName())

    return cfg
