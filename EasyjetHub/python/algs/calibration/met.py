from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import makeConfig

from EasyjetHub.steering.utils.name_helper import drop_sys


def met_sequence(flags, configAcc):

    assert flags.Analysis.do_small_R_jets, (
        "Small-R jets are strictly necessary for MET"
    )

    configSeq = ConfigSequence()

    # Suggested for bbtt
    # TODO: Make selections configurable
    container_names = flags.Analysis.container_names
    met_selections = dict(
        electrons=f'{drop_sys(container_names.output.electrons)}.loose',
        photons=f'{drop_sys(container_names.output.photons)}.tight',
        muons=f'{drop_sys(container_names.output.muons)}.medium',
        taus=f'{drop_sys(container_names.output.taus)}.loose',
    )

    configSeq += makeConfig('MissingET', drop_sys(container_names.output.met))
    # Pass all the calibrated jets
    configSeq.setOptionValue('.jets', drop_sys(container_names.allcalib.reco4PFlowJet))
    # Add whatever collections are active in the job
    for objtype, selection in met_selections.items():
        if flags.Analysis[f"do_{objtype}"]:
            configSeq.setOptionValue(f'.{objtype}', selection)

    return configSeq
