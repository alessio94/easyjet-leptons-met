from AthenaConfiguration.ComponentFactory import CompFactory
from utils.convertOldConfigHelper import convertSequenceAndGetAlgs

from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence


def makeAndAddJetAnalysisSequence(
    compAcc, dataType, inputContainerName, outputContainerName, workingPoints
):
    jetSequence = makeJetAnalysisSequence(
        dataType,
        jetCollection=inputContainerName,
        postfix="smallR",
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=True,
        enableCutflow=False,
        enableKinematicHistograms=False,
        runFJvtUpdate=False,
        runFJvtSelection=False,
        runJvtSelection=False,
    )

    bTagCalibFile = (
        "xAODBTaggingEfficiency/13TeV/2021-22-13TeV-MC16-CDI-2021-12-02_v2.root"
    )

    for tagger_wp in workingPoints:
        tagger, btag_wp = tagger_wp.split("_", 1)
        ftagSeq = makeFTagAnalysisSequence(
            jetSequence,
            dataType,
            jetCollection=inputContainerName,
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
        # Hack until this is merged:
        # https://gitlab.cern.ch/atlas/athena/-/merge_requests/54939]
        for ftagAlg in ftagSeq:
            if "FTagSelectionAlg" in ftagAlg.getName():
                ftagAlg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
            if "FTagEfficiencyScaleFactorAlg" in ftagAlg.getName():
                ftagAlg.efficiencyTool.ScaleFactorFileName = bTagCalibFile

    jetSequence.configure(
        inputName=inputContainerName,
        outputName=outputContainerName,
    )

    # Convert to new configurables
    jetSequenceCnv, jetAlgsCnv = convertSequenceAndGetAlgs(CompFactory, jetSequence)
    compAcc.addSequence(jetSequenceCnv)
    for jetAlg in jetAlgsCnv:
        compAcc.addEventAlgo(jetAlg, jetSequenceCnv.getName())

    return jetSequenceCnv, jetAlgsCnv


def makeAndAddFatJetAnalysisSequence(
    compAcc, dataType, inputContainerName, outputContainerName
):
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
    # print(largeRrecojetSequence)  # For debugging
    # Convert to new configurables
    largeRrecojetSequenceCnv, largeJetAlgsCnv = convertSequenceAndGetAlgs(
        CompFactory, largeRrecojetSequence
    )
    compAcc.addSequence(largeRrecojetSequenceCnv)
    for largeJetAlg in largeJetAlgsCnv:
        compAcc.addEventAlgo(largeJetAlg, largeRrecojetSequenceCnv.getName())

    return largeRrecojetSequenceCnv, largeJetAlgsCnv
