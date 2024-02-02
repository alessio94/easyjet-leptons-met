from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.algs.calibration.view_select import makeViewSelectionConfig
from EasyjetHub.steering.utils.name_helper import drop_sys


def tau_sequence(flags, configAcc):

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.taus)

    # PID configuration
    configSeq += makeConfig('TauJets', containerName=output_name)
    configSeq.setOptionValue('.rerunTruthMatching', False)
    # Baseline always needed for TauAntiTauJet OR
    configSeq += makeConfig('TauJets.WorkingPoint', containerName=output_name,
                            selectionName='baseline')
    configSeq.setOptionValue('.quality', 'Baseline')

    configSeq += makeConfig('TauJets.WorkingPoint', containerName=output_name,
                            selectionName=flags.Analysis.Tau.ID)
    configSeq.setOptionValue('.quality', flags.Analysis.Tau.ID)

    # Kinematic selection
    configSeq += makeConfig('TauJets.PtEtaSelection', containerName=output_name)
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 20e3)
    configSeq.setOptionValue('.maxEta', 2.5)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=output_name)

    # Apply selection as view container
    makeViewSelectionConfig(configSeq, output_name)
    # Add working point selection
    for wp in ['baseline', flags.Analysis.Tau.ID]:
        makeViewSelectionConfig(
            configSeq,
            wp + output_name,
            input=output_name,
            original=flags.Analysis.container_names.input.taus,
            selection=wp
        )

    return configSeq
