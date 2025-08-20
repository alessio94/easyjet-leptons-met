from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence

from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import TriggerAnalysisSFBlock
from AthenaConfiguration.Enums import LHCPeriod

from EasyjetHub.steering.utils.name_helper import drop_sys
from EasyjetHub.steering.analysis_configuration import get_trigger_chains_scale_factor
from EasyjetHub.steering.utils.log_helper import log


def triggerSF_sequence(flags):

    trigSF_flags = flags.Analysis.Trigger.scale_factor

    configSeq = ConfigSequence()

    for year in flags.Analysis.Years:
        if not flags.Analysis.TriggerChainsSF[str(year)]:
            log.warning(f"Empty trigger list for scale factors for year {year}")
            log.warning("We assume this is intended, so no trigger SF will be computed")
            return configSeq

    configSeq.append(TriggerAnalysisSFBlock())
    configSeq.setOptionValue('.triggerChainsPerYear',
                             get_trigger_chains_scale_factor(flags))

    # Enable trigger match event filtering
    if flags.Analysis.do_trigger_match_filtering and not trigSF_flags.do_trigger_match:
        log.error('Must enable Trigger.scale_factor.do_trigger_match if using '
                  'do_trigger_match_filtering')
        raise ValueError()
    configSeq.setOptionValue('.noFilter', not (trigSF_flags.do_trigger_match
                             and flags.Analysis.do_trigger_match_filtering))

    # Run only the global lepton/photon trigger matching
    configSeq.setOptionValue('.noEffSF', not trigSF_flags.doSF)

    if hasattr(trigSF_flags, 'Electron'):
        electronID = (trigSF_flags.Electron.ID_Run2
                      if flags.GeoModel.Run is LHCPeriod.Run2 else
                      trigSF_flags.Electron.ID_Run3)
        configSeq.setOptionValue('.electronID', electronID.removesuffix("LH"))
        electronIso = (trigSF_flags.Electron.Iso_Run2
                       if flags.GeoModel.Run is LHCPeriod.Run2 else
                       trigSF_flags.Electron.Iso_Run3)
        configSeq.setOptionValue('.electronIsol', electronIso)
        configSeq.setOptionValue(
            '.electrons', drop_sys(flags.Analysis.container_names.output.electrons))

    if hasattr(trigSF_flags, 'Photon'):
        configSeq.setOptionValue(
            '.photonIsol', trigSF_flags.Photon.Iso.removeprefix("FixedCut"))
        configSeq.setOptionValue(
            '.photons', drop_sys(flags.Analysis.container_names.output.photons))

    if hasattr(trigSF_flags, 'Muon'):
        configSeq.setOptionValue('.muonID', trigSF_flags.Muon.ID)
        configSeq.setOptionValue(
            '.muons', drop_sys(flags.Analysis.container_names.output.muons))

    return configSeq
