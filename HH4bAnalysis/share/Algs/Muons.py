from AthenaConfiguration.ComponentFactory import CompFactory
from utils.convertOldConfigHelper import convertSequenceAndGetAlgs

# Include, and then set up the muon analysis algorithm sequence:
from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence


def makeAndAddMuonAnalysisSequence(
    compAcc, dataType, inputContainerName, outputContainerName
):
    muonSequence = makeMuonAnalysisSequence(
        dataType,
        workingPoint="Loose.NonIso",
        postfix="loose",
        deepCopyOutput=False,
        shallowViewOutput=True,
        ptSelectionOutput=False,
        qualitySelectionOutput=True,
        enableCutflow=False,
        enableKinematicHistograms=False,
    )
    muonSequence.configure(inputName=inputContainerName, outputName=outputContainerName)
    # print(muonLooseSequence)  # For debugging
    # Convert to new configurables
    muonSequenceCnv, muonAlgsCnv = convertSequenceAndGetAlgs(CompFactory, muonSequence)
    compAcc.addSequence(muonSequenceCnv)
    for muonAlg in muonAlgsCnv:
        compAcc.addEventAlgo(muonAlg, muonSequenceCnv.getName())

    return muonSequenceCnv, muonAlgsCnv
