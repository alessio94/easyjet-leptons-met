from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence

from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import TriggerAnalysisSFBlock

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
    # Disabling the trigger matching requirement
    configSeq.setOptionValue('.noFilter', True)

    if hasattr(trigSF_flags, 'Electron'):
        configSeq.setOptionValue('.electronID',
                                 trigSF_flags.Electron.ID.removesuffix("LH"))
        configSeq.setOptionValue('.electronIsol', trigSF_flags.Electron.Iso)
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
