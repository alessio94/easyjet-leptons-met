from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def overlap_sequence_cfg(
    flags,
    datatype,
    inputnames,
    outputnames,
    **kwargs
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
        enableUserPriority=False,
        bJetLabel="",
        boostedLeptons=False,
        postfix="",
        shallowViewOutput=True,
        enableCutflow=False,
        **kwargs
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
