from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.steering.utils.name_helper import drop_sys
from EasyjetHub.steering.analysis_configuration import get_trigger_chains_scale_factor


def tau_sequence(flags, configAcc):

    wps = ['Baseline', flags.Analysis.Tau.ID]
    if 'extra_wps' in flags.Analysis.Tau:
        for wp in flags.Analysis.Tau.extra_wps:
            wps.append(wp)

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.taus)

    # PID configuration
    configSeq += makeConfig('TauJets', containerName=output_name)
    configSeq.setOptionValue('.rerunTruthMatching', False)
    configSeq.setOptionValue('.decorateTruth', True)
    for id in wps:
        configSeq += makeConfig('TauJets.WorkingPoint',
                                containerName=output_name,
                                selectionName=id)
        configSeq.setOptionValue('.quality', id)

    # Tau trigger SF
    trigSF_flags = flags.Analysis.trigger.scale_factor
    if hasattr(trigSF_flags, 'Tau'):
        configSeq += makeConfig('TauJets.TriggerSF')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.tauID', trigSF_flags.Tau.ID)
        configSeq.setOptionValue('.triggerChainsPerYear',
                                 get_trigger_chains_scale_factor(flags, 'Tau'))

    # Kinematic selection
    configSeq += makeConfig('TauJets.PtEtaSelection', containerName=output_name,
                            selectionName='selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 20e3)
    configSeq.setOptionValue('.maxEta', 2.5)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=output_name)

    configSeq += makeConfig('Thinning', containerName=output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')

    for id in wps:
        configSeq += makeConfig('Thinning', containerName=output_name,
                                configName=f'Thinning_{id}')
        configSeq.setOptionValue('.selectionName', id)
        configSeq.setOptionValue('.outputName', id + output_name)
        configSeq.setOptionValue('.postfix', id)

    return configSeq
