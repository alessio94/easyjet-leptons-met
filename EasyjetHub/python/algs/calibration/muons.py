from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.steering.utils.name_helper import drop_sys
from EasyjetHub.steering.analysis_configuration import get_trigger_chains_scale_factor


def muon_sequence(flags, configAcc):

    wps = [(flags.Analysis.Muon.ID, flags.Analysis.Muon.Iso)]
    if 'extra_wps' in flags.Analysis.Muon:
        for wp in flags.Analysis.Muon.extra_wps:
            wps.append((wp[0], wp[1]))

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    output_name = drop_sys(flags.Analysis.container_names.output.muons)

    if flags.Analysis.Muon.MergeLRT:
        configSeq += makeConfig('Muons.LRTMerging')
        configSeq.setOptionValue('.containerName', 'Muons_LRTMerged')

    configSeq += makeConfig('Muons', containerName=output_name)
    if flags.Analysis.Muon.MergeLRT:
        configSeq.setOptionValue('.inputContainer', 'Muons_LRTMerged')
    configSeq.setOptionValue('.minPt', flags.Analysis.Muon.min_pT)
    configSeq.setOptionValue('.maxEta', flags.Analysis.Muon.max_eta)
    configSeq.setOptionValue('.writeTrackD0Z0', True)
    configSeq.setOptionValue('.decorateTruth', True)

    # PID configuration
    for id, iso in wps:
        configSeq += makeConfig('Muons.WorkingPoint', containerName=output_name,
                                selectionName=id + '_' + iso)
        if "nottva" in id:
            configSeq.setOptionValue('.trackSelection', False)
        else:
            configSeq.setOptionValue('.trackSelection', True)
        quality = id.replace("_nottva", "")
        configSeq.setOptionValue('.quality', quality)
        configSeq.setOptionValue('.isolation', iso)
        configSeq.setOptionValue('.maxD0Significance',
                                 flags.Analysis.Muon.maxD0Significance)
        configSeq.setOptionValue('.maxDeltaZ0SinTheta',
                                 flags.Analysis.Muon.maxDeltaZ0SinTheta)
        configSeq.setOptionValue('.saveCombinedSF', True)

    if flags.Analysis.Small_R_jet.runBJetPtCalib or \
       flags.Analysis.Large_R_jet.runMuonJetPtCorr:
        configSeq += makeConfig('Muons.WorkingPoint', containerName=output_name,
                                selectionName='forBJetCalib')
        configSeq.setOptionValue('.quality', 'Medium')
        configSeq.setOptionValue('.isolation', 'NonIso')
        configSeq.setOptionValue('.trackSelection', False)

    # Muon trigger SF
    trigSF_flags = flags.Analysis.Trigger.scale_factor
    if trigSF_flags.doSF and hasattr(trigSF_flags, 'Muon'):
        configSeq += makeConfig('Muons.TriggerSF')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.muonID', trigSF_flags.Muon.ID)
        configSeq.setOptionValue('.triggerChainsPerYear',
                                 get_trigger_chains_scale_factor(flags, 'Muon'))

    # IFF truth decoration
    if flags.Analysis.Muon.do_IFF_decoration:
        configSeq += makeConfig('Muons.IFFClassification',
                                containerName=output_name)

    # Kinematic selection
    configSeq += makeConfig('Muons.PtEtaSelection', containerName=output_name,
                            selectionName='selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', flags.Analysis.Muon.min_pT)
    configSeq.setOptionValue('.maxEta', flags.Analysis.Muon.max_eta)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=output_name)

    configSeq += makeConfig('Thinning', containerName=output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')

    return configSeq
