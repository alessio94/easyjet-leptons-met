from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod

from EasyjetHub.steering.utils.name_helper import drop_sys


def photon_sequence(flags, configAcc):

    wps = [(flags.Analysis.Photon.ID, flags.Analysis.Photon.Iso)]
    if 'extra_wps' in flags.Analysis.Photon:
        for wp in flags.Analysis.Photon.extra_wps:
            wps.append((wp[0], wp[1]))

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    if flags.Analysis.Photon.forceFullSimConfig \
       and flags.Analysis.DataType is DataType.FastSim:
        print("WARNING! If not already done, you should get in touch with the")
        print("EGamma group to contribute to the Photon AF3 recommendations as")
        print("you're relying on them")

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.photons)
    configSeq += makeConfig('Photons', containerName=output_name)
    configSeq.setOptionValue('.recomputeIsEM', False)
    configSeq.setOptionValue('.crackVeto', True)
    configSeq.setOptionValue('.forceFullSimConfig',
                             flags.Analysis.Photon.forceFullSimConfig
                             and flags.Analysis.DataType is DataType.FastSim)

    # PID configuration
    for id, iso in wps:
        configSeq += makeConfig('Photons.WorkingPoint', containerName=output_name,
                                selectionName=id + '_' + iso)
        configSeq.setOptionValue('.qualityWP', id)
        configSeq.setOptionValue('.isolationWP', iso)
        configSeq.setOptionValue('.forceFullSimConfig',
                                 flags.Analysis.Photon.forceFullSimConfig
                                 and flags.Analysis.DataType is DataType.FastSim)

        # No Run 2 SF yet
        if flags.GeoModel.Run is LHCPeriod.Run2 and flags.Input.isMC:
            print("WARNING! Run 2 photon SF are not available yet")
            configSeq.setOptionValue('.noEffSF', True)

    # Kinematic selection
    configSeq += makeConfig('Photons.PtEtaSelection', containerName=output_name,
                            selectionName='selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 10e3)
    configSeq.setOptionValue('.maxEta', 2.37)

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
