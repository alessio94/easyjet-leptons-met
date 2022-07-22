from AthenaConfiguration.ComponentFactory import CompFactory
from utils.convertOldConfigHelper import convertSequenceAndGetAlgs

# Include, and then set up the overlap analysis algorithm sequence:
from AsgAnalysisAlgorithms.OverlapAnalysisSequence import (
    makeOverlapAnalysisSequence,
)


def makeAndAddOverlapAnalysisSequence(compAcc, dataType, inputNames, outputNames):
    overlapSequence = makeOverlapAnalysisSequence(
        dataType,
        inputLabel="",
        outputLabel="passesOR",
        linkOverlapObjects=False,
        doEleEleOR=False,
        doMuPFJetOR=True,
        doTaus=False,
        doElectrons=True,
        doMuons=True,
        doJets=True,
        doPhotons=True,
        doFatJets=True,
        enableUserPriority=False,
        bJetLabel="",
        boostedLeptons=False,
        postfix="",
        shallowViewOutput=True,
        enableCutflow=False,
    )
    overlapSequence.configure(
        inputName=inputNames,
        outputName=outputNames,
    )
    # print(overlapSequence)  # For debugging

    # Convert to new configurables
    overlapSequenceCnv, overlapAlgsCnv = convertSequenceAndGetAlgs(
        CompFactory, overlapSequence
    )
    compAcc.addSequence(overlapSequenceCnv)
    for overlapAlg in overlapAlgsCnv:
        compAcc.addEventAlgo(overlapAlg, overlapSequenceCnv.getName())

    return overlapSequenceCnv, overlapAlgsCnv
