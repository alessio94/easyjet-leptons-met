from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod

from EasyjetHub.steering.utils.name_helper import drop_sys
from EasyjetHub.steering.analysis_configuration import get_trigger_chains_scale_factor


def electron_sequence(flags, configAcc):

    wps = [(flags.Analysis.Electron.ID, flags.Analysis.Electron.Iso)]
    if 'extra_wps' in flags.Analysis.Electron:
        for wp in flags.Analysis.Electron.extra_wps:
            wps.append((wp[0], wp[1]))

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    if flags.Analysis.Electron.forceFullSimConfig \
       and flags.Analysis.DataType is DataType.FastSim:
        print("WARNING! If not already done, you should get in touch with the")
        print("EGamma group to contribute to the Electron AF3 recommendations as")
        print("you're relying on them")

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.electrons)
    configSeq += makeConfig('Electrons', containerName=output_name)
    configSeq.setOptionValue('.crackVeto', True)
    configSeq.setOptionValue('.forceFullSimConfig',
                             flags.Analysis.Electron.forceFullSimConfig
                             and flags.Analysis.DataType is DataType.FastSim)
    configSeq.setOptionValue('.decorrelationModel',
                             flags.Analysis.Electron.correlationModelScale)
    configSeq.setOptionValue('.writeTrackD0Z0', True)

    # PID configuration
    for id, iso in wps:
        configSeq += makeConfig('Electrons.WorkingPoint', containerName=output_name,
                                selectionName=id + '_' + iso)
        if "nottva" in id:
            configSeq.setOptionValue('.trackSelection', False)
        else:
            configSeq.setOptionValue('.trackSelection', True)
        id = id.replace("_nottva", "")
        configSeq.setOptionValue('.identificationWP', id)
        configSeq.setOptionValue('.isolationWP', iso)
        configSeq.setOptionValue('.forceFullSimConfig',
                                 flags.Analysis.Electron.forceFullSimConfig
                                 and flags.Analysis.DataType is DataType.FastSim)
        configSeq.setOptionValue('.maxD0Significance',
                                 flags.Analysis.Electron.maxD0Significance)
        configSeq.setOptionValue('.maxDeltaZ0SinTheta',
                                 flags.Analysis.Electron.maxDeltaZ0SinTheta)
        configSeq.setOptionValue('.chargeIDSelectionRun2',
                                 flags.Analysis.Electron.chargeIDSelectionRun2
                                 and flags.GeoModel.Run is LHCPeriod.Run2)
        configSeq.setOptionValue('.correlationModelId',
                                 flags.Analysis.Electron.correlationModelId)
        configSeq.setOptionValue('.correlationModelIso',
                                 flags.Analysis.Electron.correlationModelIso)
        # Only TOTAL correlation model is currently supported
        # for reconstruction efficiency correction in Run 3
        configSeq.setOptionValue('.correlationModelReco',
                                 flags.Analysis.Electron.correlationModelReco)
        configSeq.setOptionValue('.saveCombinedSF', True)
        # No DNN SF yet
        if "DNN" in id and flags.Input.isMC:
            print("WARNING! Electron DNN ID does not have SF available yet")
            configSeq.setOptionValue('.noEffSF', True)
        # No Run 2 SF yet
        if flags.GeoModel.Run is LHCPeriod.Run2 and flags.Input.isMC:
            print("WARNING! Run 2 electron SF are not available yet")
            configSeq.setOptionValue('.noEffSF', True)

    # Electron trigger SF
    trigSF_flags = flags.Analysis.Trigger.scale_factor
    if trigSF_flags.doSF and hasattr(trigSF_flags, 'Electron'):
        configSeq += makeConfig('Electrons.TriggerSF')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.electronID', trigSF_flags.Electron.ID)
        configSeq.setOptionValue('.electronIsol', trigSF_flags.Electron.Iso)
        configSeq.setOptionValue('.triggerChainsPerYear',
                                 get_trigger_chains_scale_factor(flags, 'Electron'))

    # IFF truth decoration
    if flags.Analysis.Electron.do_IFF_decoration:
        configSeq += makeConfig('Electrons.IFFClassification',
                                containerName=output_name)

    # Kinematic selection
    configSeq += makeConfig('Electrons.PtEtaSelection', containerName=output_name,
                            selectionName='selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', flags.Analysis.Electron.min_pT)
    configSeq.setOptionValue('.maxEta', flags.Analysis.Electron.max_eta)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=output_name)

    configSeq += makeConfig('Thinning', containerName=output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')

    return configSeq
