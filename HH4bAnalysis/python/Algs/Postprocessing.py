from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def OverlapAnalysisSequenceCfg(
    flags,
    dataType,
    inputNames,
    outputNames,
    doFatJets=True,
    doMuons=True,
):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.OverlapAnalysisSequence import (
        makeOverlapAnalysisSequence,
    )

    overlapSequence = makeOverlapAnalysisSequence(
        dataType,
        inputLabel="",
        outputLabel="passesOR",
        linkOverlapObjects=False,
        doEleEleOR=False,
        doMuPFJetOR=False,
        doTaus=False,
        doElectrons=True,
        doMuons=doMuons,
        doJets=True,
        doPhotons=True,
        doFatJets=doFatJets,
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

    cfg.addSequence(CompFactory.AthSequencer(overlapSequence.getName()))
    for alg in overlapSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, overlapSequence.getName())

    return cfg
