from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from TauAnalysisAlgorithms.TauAnalysisConfig import (
    EXPERIMENTAL_TauCombineMuonRemovalConfig)

from EasyjetHub.steering.utils.name_helper import drop_sys
from EasyjetHub.steering.analysis_configuration import get_trigger_chains_scale_factor

from EasyjetHub.algs.calibration.antitaus import HHbbttAntiTauDecoratorBlock


def tau_sequence(flags, configAcc):

    wps = [flags.Analysis.Tau.ID]
    if 'extra_wps' in flags.Analysis.Tau:
        for wp in flags.Analysis.Tau.extra_wps:
            wps.append(wp)

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.taus)

    if flags.Analysis.Tau.addMuonRM:
        configSeq.append(EXPERIMENTAL_TauCombineMuonRemovalConfig())
        configSeq.setOptionValue('.outputTaus', 'TauJets_MuonRmCombined')

    # PID configuration
    configSeq += makeConfig('TauJets')
    configSeq.setOptionValue('.containerName', output_name)
    if flags.Analysis.Tau.addMuonRM:
        configSeq.setOptionValue('.inputContainer', 'TauJets_MuonRmCombined')
    configSeq.setOptionValue('.rerunTruthMatching', False)
    configSeq.setOptionValue('.decorateTruth', True)
    for id in wps:
        configSeq += makeConfig('TauJets.WorkingPoint')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.selectionName', id)
        if "Custom" in id:
            manualMinTauPt = flags.Analysis.Tau.min_pT * 0.001
            configSeq.setOptionValue('.useSelectionConfigFile', False)
            configSeq.setOptionValue('.manual_sel_minpt', manualMinTauPt)
        if "eleid" in id:
            configSeq.setOptionValue('.use_eVeto', True)
        if "GNTau" in id:
            configSeq.setOptionValue('.useGNTau', True)
        if "lowpt" in id:
            configSeq.setOptionValue('.useLowPt', True)
        elif "nopt" in id:
            configSeq.setOptionValue('.dropPtCut', True)
        quality = id.replace("_eleid", "").replace("GNTau", "").replace("RNN", "")
        quality = quality.replace("_lowpt", "").replace("_nopt", "")
        quality = quality.replace("Custom", "")
        configSeq.setOptionValue('.quality', quality)
        configSeq.setOptionValue('.saveCombinedSF', True)

    # Anti-tau selections
    if flags.Analysis.do_bbtt_analysis or flags.Analysis.do_bbbbtt_analysis:
        configSeq.append(HHbbttAntiTauDecoratorBlock())
        configSeq.setOptionValue('.taus', output_name)
        configSeq.setOptionValue('.tauBaselineSelection', flags.Analysis.Tau.ID)
        configSeq.setOptionValue('.tauIDSelection', flags.Analysis.Tau.extra_wps[0])
        configSeq.setOptionValue('.antiTauScoreThreshold',
                                 flags.Analysis.Tau.antiTauScoreThreshold)
        muons = drop_sys(flags.Analysis.container_names.output.muons)
        muons += f'.{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
        configSeq.setOptionValue('.muons', muons)
        electrons = drop_sys(flags.Analysis.container_names.output.electrons)
        electrons += f'.{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
        configSeq.setOptionValue('.electrons', electrons)

    # Tau trigger SF
    trigSF_flags = flags.Analysis.Trigger.scale_factor
    if trigSF_flags.doSF and hasattr(trigSF_flags, 'Tau'):
        configSeq += makeConfig('TauJets.TriggerSF')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.tauID', trigSF_flags.Tau.ID)
        configSeq.setOptionValue('.triggerChainsPerYear',
                                 get_trigger_chains_scale_factor(flags, 'Tau'))

    # Kinematic selection
    configSeq += makeConfig('TauJets.PtEtaSelection')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', flags.Analysis.Tau.min_pT)
    configSeq.setOptionValue('.maxEta', 2.5)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink')
    configSeq.setOptionValue('.containerName', output_name)

    configSeq += makeConfig('Thinning')
    configSeq.setOptionValue('.containerName', output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')

    return configSeq
