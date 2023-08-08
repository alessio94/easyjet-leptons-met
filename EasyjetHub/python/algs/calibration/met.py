from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def met_sequence_cfg(flags):
    cfg = ComponentAccumulator()
    from MetAnalysisAlgorithms.MetAnalysisSequence import makeMetAnalysisSequence

    met_sequence = makeMetAnalysisSequence(
        flags.Analysis.DataType,
        metSuffix=flags.Analysis.container_names.input.met
    )

    # Small-R jets are mandatory for MET
    inputs = {"jets": flags.Analysis.container_names.output.reco4PFlowJet}
    if flags.Analysis.do_muons:
        inputs["muons"] = flags.Analysis.container_names.output.muons
    if flags.Analysis.do_electrons:
        inputs["electrons"] = flags.Analysis.container_names.output.electrons
    if flags.Analysis.do_photons:
        inputs["photons"] = flags.Analysis.container_names.output.photons
    if flags.Analysis.do_taus:
        inputs["taus"] = flags.Analysis.container_names.output.taus

    met_sequence.configure(
        inputName=inputs,
        outputName=flags.Analysis.container_names.output.met
    )

    cfg.addSequence(CompFactory.AthSequencer(met_sequence.getName()))
    for alg in met_sequence.getGaudiConfig2Components():
        cfg.addEventAlgo(alg, met_sequence.getName())

    return cfg
