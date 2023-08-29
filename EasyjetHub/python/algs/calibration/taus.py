from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import makeConfig

from EasyjetHub.algs.calibration.view_select import makeViewSelectionConfig
from EasyjetHub.steering.utils.name_helper import drop_sys


def tau_sequence(flags, configAcc):

    configSeq = ConfigSequence()

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.taus)

    # PID configuration
    configSeq += makeConfig('TauJets', output_name)
    configSeq.setOptionValue('.rerunTruthMatching', False)
    configSeq += makeConfig('TauJets.Selection', output_name + '.loose')
    configSeq.setOptionValue('.quality', 'Loose')
    configSeq += makeConfig('TauJets.Selection', output_name + '.tight')
    configSeq.setOptionValue('.quality', 'Tight')

    # Kinematic selection
    configSeq += makeConfig('Selection.PtEta', output_name)
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 20e3)
    configSeq.setOptionValue('.maxEta', 2.5)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', f'SystObjectLink.{output_name}')

    # Apply selection as view container
    makeViewSelectionConfig(configSeq, output_name)
    # Add working point selection
    for wp in ['loose','tight']:
        makeViewSelectionConfig(
            configSeq,
            wp + output_name,
            input=output_name,
            original=flags.Analysis.container_names.input.taus,
            selection=wp
        )

    return configSeq
