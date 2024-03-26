from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.steering.utils.name_helper import drop_sys


def electron_sequence(flags, configAcc):

    wps = [(flags.Analysis.Electron.ID, flags.Analysis.Electron.Iso)]
    if 'extra_wps' in flags.Analysis.Electron:
        for wp in flags.Analysis.Electron.extra_wps:
            wps.append((wp[0], wp[1]))

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    if flags.Analysis.Electron.forceFullSimConfig:
        print("WARNING! If not already done, you should get in touch with the")
        print("EGamma group to contribute to the Electron AF3 recommendations as")
        print("you're relying on them")

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.electrons)
    configSeq += makeConfig('Electrons', containerName=output_name)
    configSeq.setOptionValue('.crackVeto', True)
    configSeq.setOptionValue('.forceFullSimConfig',
                             flags.Analysis.Electron.forceFullSimConfig)

    # PID configuration
    for id, iso in wps:
        configSeq += makeConfig('Electrons.WorkingPoint', containerName=output_name,
                                selectionName=id + '_' + iso)
        configSeq.setOptionValue('.likelihoodWP', id)
        configSeq.setOptionValue('.isolationWP', iso)
        configSeq.setOptionValue('.recomputeLikelihood', False)
        configSeq.setOptionValue('.forceFullSimConfig',
                                 flags.Analysis.Electron.forceFullSimConfig)

    # Kinematic selection
    configSeq += makeConfig('Electrons.PtEtaSelection', containerName=output_name,
                            selectionName='selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 4.5e3)
    configSeq.setOptionValue('.maxEta', 2.47)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=output_name)

    configSeq += makeConfig('Thinning', containerName=output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')

    for id, iso in wps:
        label = id + '_' + iso
        configSeq += makeConfig('Thinning', containerName=output_name,
                                configName=f'Thinning_{label}')
        configSeq.setOptionValue('.selectionName', label)
        configSeq.setOptionValue('.outputName', label + output_name)
        configSeq.setOptionValue('.postfix', label)

    return configSeq
