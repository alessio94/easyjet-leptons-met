from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AthenaConfiguration.Enums import LHCPeriod

from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.algs.calibration.event_weights import (
    generator_sequence,
    pileup_sequence,
)
from EasyjetHub.algs.calibration.jets import (
    jet_sequence,
    vr_jet_sequence,
    lr_jet_sequence,
    lr_jet_ghost_vr_jet_association_cfg,
)
from EasyjetHub.output.ttree.btag_decor_config import btag_decor_cfg
from EasyjetHub.output.ttree.tau_decor_config import tau_decor_cfg
from EasyjetHub.algs.calibration.muons import muon_sequence
from EasyjetHub.algs.calibration.electrons import electron_sequence
from EasyjetHub.algs.calibration.photons import photon_sequence
from EasyjetHub.algs.calibration.taus import tau_sequence
from EasyjetHub.algs.calibration.met import met_sequence
from EasyjetHub.algs.postprocessing.overlap_removal import overlap_sequence
from EasyjetHub.steering.utils.log_helper import log
from EasyjetHub.steering.utils.systematics_helper import consolidate_systematics_regex

from EasyjetHub.algs.event_counter_config import event_counter_cfg

# Map object types to sequence configurators
analysis_seqs = {
    "muons":        muon_sequence,
    "electrons":    electron_sequence,
    "photons":      photon_sequence,
    "taus":         tau_sequence,
    "small_R_jets": jet_sequence,
    "VR_jets":      vr_jet_sequence,
}


def cpalgs_cfg(flags):

    log.debug(f"Containers available in dataset: {flags.Input.Collections}")

    cfg = ComponentAccumulator()
    cfg.merge(event_counter_cfg("n_input"))

    # Create SystematicsSvc explicitly:
    sysSvc = CompFactory.CP.SystematicsSvc("SystematicsSvc")
    cfg.addService(sysSvc)
    if flags.Analysis.do_CP_systematics:
        sysSvc.sigmaRecommended = 1
        systs = consolidate_systematics_regex(flags.Analysis.systematics_regex)

        log.info("Systematics regex:")
        log.info(systs)
        sysSvc.systematicsRegex = systs

        syslistalg = CompFactory.CP.SysListDumperAlg(
            "SysList",
            histogramName="systematics",
            systematicsService=sysSvc,
        )
        cfg.addEventAlgo(syslistalg)

    # Create SelectionNameSvc explicitly
    selectionSvc = CompFactory.CP.SelectionNameSvc("SelectionNameSvc")
    cfg.addService(selectionSvc)

    # Aggregate the configured CP algs in one ConfigSequence,
    # which will handle the container names, copying etc
    configSeq = ConfigSequence()

    # Define the sequence holding all the calibration
    # Activate the full configuration, which stitches together
    # the ConfigBlocks with interstitial container names etc
    calibSeq = CompFactory.AthSequencer('CPAlgSequence')
    configAccumulator = ConfigAccumulator(
        flags.Analysis.DataType,
        calibSeq,
        isPhyslite=flags.Input.isPHYSLITE,
        geometry=getattr(LHCPeriod,f'Run{flags.Analysis.Run}'),
    )

    if not flags.Analysis.disable_calib:
        if flags.Analysis.doPRW:
            log.info("Adding PRW sequence")
            # Adds variable to EventInfo if for pileup weight, for example:
            # EventInfo.PileWeight_%SYS$
            configSeq += pileup_sequence(flags)

        if flags.Analysis.DataType != "data":
            log.info("Adding generator analysis sequence")
            # Adds variable to EventInfo if for generator weight, for example:
            # EventInfo.generatorWeight_%SYS%
            configSeq += generator_sequence(flags)

        for objtype in [
            "electrons",
            "photons",
            "muons",
            "taus",
            "small_R_jets",
            "VR_jets"
        ]:
            if flags.Analysis[f"do_{objtype}"]:
                log.info(f"Adding {objtype} seq")
                if objtype == 'small_R_jets':
                    # Schedule the alg to decorate btag info onto jets
                    # rather than accessing from xAOD::BTagging
                    # We perform the decoration on the uncalibrated jets
                    # so as to avoid any systematics-dependence or filtering
                    cfg.merge(btag_decor_cfg(flags))
                elif objtype == 'taus':
                    # Schedule the alg to decorate taus with nProng info
                    cfg.merge(tau_decor_cfg(flags))

                # Append the configured CP calibration sequence for
                # the given object type
                # Pass the configAccumulator because we may need to
                # provide some container name info (e.g. for VR jets)
                configSeq += analysis_seqs[objtype](flags,configAccumulator)

        if flags.Analysis.do_large_R_Topo_jets:
            log.info("Adding large-R jet seq")
            configSeq += lr_jet_sequence(
                flags,
                lr_jet_type="Topo",
                configAcc=configAccumulator,
            )

        if flags.Analysis.do_large_R_UFO_jets:
            log.info("Adding UFO large-R jet seq")
            configSeq += lr_jet_sequence(
                flags,
                lr_jet_type="UFO",
                configAcc=configAccumulator,
            )

        if flags.Analysis.do_met:
            log.info("Adding MET seq")
            configSeq += met_sequence(flags,configAccumulator)

    ########################################################################
    # Begin postprocessing
    ########################################################################

    if flags.Analysis.do_overlap_removal:
        log.info("Adding Overlap Removal sequence")

        configSeq += overlap_sequence(flags)

    configSeq.fullConfigure(configAccumulator)

    cfg.merge(configAccumulator.CA)

    # Need to run the jet association after the VR jet calibration
    # Could conceivably set this up as a ConfigSequence instead
    if not flags.Analysis.disable_calib:

        if flags.Analysis.do_VR_jets:
            if flags.Analysis.do_large_R_Topo_jets:
                cfg.merge(
                    lr_jet_ghost_vr_jet_association_cfg(
                        flags,
                        lr_jet_type="Topo",
                    )
                )

            if flags.Analysis.do_large_R_UFO_jets:
                cfg.merge(
                    lr_jet_ghost_vr_jet_association_cfg(
                        flags,
                        lr_jet_type="UFO",
                    )
                )

    return cfg
