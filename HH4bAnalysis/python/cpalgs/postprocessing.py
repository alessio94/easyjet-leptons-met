from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def overlap_sequence_cfg(
    flags,
    datatype,
    inputnames,
    outputnames,
    doFatJets=True,
    doMuons=True,
):
    cfg = ComponentAccumulator()
    from AsgAnalysisAlgorithms.OverlapAnalysisSequence import (
        makeOverlapAnalysisSequence,
    )

    overlapSequence = makeOverlapAnalysisSequence(
        datatype,
        inputLabel="",
        outputLabel="passesOR",
        linkOverlapObjects=False,
        doEleEleOR=False,
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
        inputName=inputnames,
        outputName=outputnames,
    )
    # print(overlapSequence)  # For debugging

    cfg.addSequence(CompFactory.AthSequencer(overlapSequence.getName()))
    for alg in overlapSequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, overlapSequence.getName())

    return cfg
