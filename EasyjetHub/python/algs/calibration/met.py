from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def met_sequence_cfg(flags, containers):
    cfg = ComponentAccumulator()
    from MetAnalysisAlgorithms.MetAnalysisSequence import makeMetAnalysisSequence

    met_sequence = makeMetAnalysisSequence(
        flags.Analysis.DataType,
        metSuffix=containers["inputs"]["met"]
    )

    # Small-R jets are mandatory for MET
    inputs = {"jets": containers["outputs"]["reco4PFlowJet"]}
    if flags.Analysis.do_muons:
        inputs["muons"] = containers["outputs"]["muons"]
    if flags.Analysis.do_electrons:
        inputs["electrons"] = containers["outputs"]["electrons"]
    if flags.Analysis.do_photons:
        inputs["photons"] = containers["outputs"]["photons"]
    if flags.Analysis.do_taus:
        inputs["taus"] = containers["outputs"]["taus"]

    met_sequence.configure(
        inputName=inputs,
        outputName=containers["outputs"]["met"]
    )

    cfg.addSequence(CompFactory.AthSequencer(met_sequence.getName()))
    for alg in met_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, met_sequence.getName())

    return cfg
