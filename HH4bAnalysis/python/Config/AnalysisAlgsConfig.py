from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.Algs.BoostedAnalysis import BoostedAnalysisCfg
from HH4bAnalysis.Algs.Electrons import ElectronAnalysisSequenceCfg
from HH4bAnalysis.Algs.Event import (
    EventSelectionAnalysisSequenceCfg,
    GeneratorAnalysisSequenceCfg,
    PileupAnalysisSequenceCfg,
    TriggerAnalysisSequenceCfg,
)
from HH4bAnalysis.Algs.Jets import (
    FatJetAnalysisSequenceCfg,
    FatUFOJetAnalysisSequenceCfg,
    JetAnalysisSequenceCfg,
    LargeJetGhostVRJetAssociationAlgCfg,
    LargeUFOJetGhostVRJetAssociationAlgCfg,
    VRJetAnalysisSequenceCfg,
)
from HH4bAnalysis.Algs.Muons import MuonAnalysisSequenceCfg
from HH4bAnalysis.Algs.Photons import PhotonAnalysisSequenceCfg
from HH4bAnalysis.Algs.Postprocessing import OverlapAnalysisSequenceCfg
from HH4bAnalysis.Algs.ResolvedAnalysis import ResolvedAnalysisCfg
from HH4bAnalysis.Algs.TruthInformation import TruthParticleInformationAlgCfg
from HH4bAnalysis.utils.containerNameHelper import get_container_names
from HH4bAnalysis.utils.inputsHelper import is_physlite
from HH4bAnalysis.utils.logHelper import log

from HH4bAnalysis.Config.EventCounterConfig import eventCounterCfg
from HH4bAnalysis.Config.JetParentDecoratorConfig import jetParentDecoratorCfg


# Generate the algorithm to do the dumping.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def AnalysisAlgsCfg(
    flags,
    dataType,
    trigger_chains=[],
    do_muons=True,
    do_PRW=False,
    prw_files=[],
    lumicalc_files=[],
    grl_files=[],
):
    is_daod_physlite = is_physlite(flags)

    log.debug(f"Containers available in dataset: {flags.Input.Collections}")

    cfg = ComponentAccumulator()
    cfg.merge(eventCounterCfg('n_input'))

    # Create SystematicsSvc explicitly:
    sysSvc = CompFactory.CP.SystematicsSvc("SystematicsSvc")
    cfg.addService(sysSvc)
    if flags.Analysis.do_CP_systematics:
        sysSvc.sigmaRecommended = 1

    log.info("Adding trigger analysis algs")
    # Removes events failing trigger and adds variable to EventInfo
    # if trigger passed or not, for example:
    # EventInfo.trigger_name
    cfg.merge(TriggerAnalysisSequenceCfg(flags, dataType, trigger_chains))
    cfg.merge(eventCounterCfg('n_trigger'))

    log.info("Add DQ event filter sequence")
    # Remove events failing DQ criteria
    cfg.merge(
        EventSelectionAnalysisSequenceCfg(
            flags, dataType, grlFiles=grl_files, loose=flags.Analysis.loose_jet_cleaning
        )
    )
    cfg.merge(eventCounterCfg('n_data_quality'))

    containers = get_container_names(flags)

    if not flags.Analysis.disable_calib:
        if do_PRW:
            log.info("Adding PRW sequence")
            # Adds variable to EventInfo if for pileup weight, for example:
            # EventInfo.PileWeight_%SYS$
            cfg.merge(
                PileupAnalysisSequenceCfg(
                    flags,
                    dataType=dataType,
                    prwFiles=prw_files,
                    lumicalcFiles=lumicalc_files,
                )
            )
            log.info("Adding generator analysis sequence")
            # Adds variable to EventInfo if for generator weight, for example:
            # EventInfo.generatorWeight_%SYS%
            cfg.merge(GeneratorAnalysisSequenceCfg(flags, dataType))

        log.info("Adding electron seq")
        cfg.merge(
            ElectronAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=containers["inputs"]["electrons"],
                outputContainerName=containers["outputs"]["electrons"],
            )
        )

        log.info("Adding photon seq")
        cfg.merge(
            PhotonAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=containers["inputs"]["photons"],
                outputContainerName=containers["outputs"]["photons"],
            )
        )

        if do_muons:
            log.info("Adding muon seq")
            cfg.merge(
                MuonAnalysisSequenceCfg(
                    flags,
                    dataType=dataType,
                    inputContainerName=containers["inputs"]["muons"],
                    outputContainerName=containers["outputs"]["muons"],
                )
            )

        if flags.Input.isMC:
            cfg.merge(jetParentDecoratorCfg(flags))

        log.info("Adding small-R jet seq")
        muoncont = containers["outputs"]["muons"]
        if not do_muons:
            muoncont = containers["inputs"]["muons"]
        cfg.merge(
            JetAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=containers["inputs"]["reco4Jet"],
                outputContainerName=containers["outputs"]["reco4Jet"],
                # Need muons for b-jet pt correction
                muonContainerName=muoncont,
                do_bjet_ptcalib=do_muons,
                is_daod_physlite=is_daod_physlite,
            )
        )
        cfg.merge(eventCounterCfg('n_small_r'))

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip large-R jet sequence for now")
        else:
            log.info("Adding large-R jet seq")
            cfg.merge(
                FatJetAnalysisSequenceCfg(
                    flags,
                    dataType=dataType,
                    inputContainerName=containers["inputs"]["reco10Jet"],
                    outputContainerName=containers["outputs"]["reco10Jet"],
                )
            )
            cfg.merge(eventCounterCfg('n_large_r'))

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip  UFO large-R jet sequence for now")
        else:
            log.info("Adding UFO large-R jet seq")
            cfg.merge(
                FatUFOJetAnalysisSequenceCfg(
                    flags,
                    dataType=dataType,
                    inputContainerName=containers["inputs"]["reco10UFOJet"],
                    outputContainerName=containers["outputs"]["reco10UFOJet"],
                )
            )
            cfg.merge(eventCounterCfg('n_large_r_ufo'))

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip VR jet sequence for now")
        else:
            log.info("Adding VR jet seq")
            cfg.merge(
                VRJetAnalysisSequenceCfg(
                    flags,
                    dataType=dataType,
                    inputContainerName=containers["inputs"]["vrJet"],
                    outputContainerName=containers["outputs"]["vrJet"],
                )
            )
            cfg.merge(eventCounterCfg('n_vr'))

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip ghost assocciation VR jet sequence for now")
        else:
            cfg.merge(
                LargeJetGhostVRJetAssociationAlgCfg(
                    flags,
                    inputLargeRJetContainerName=containers["outputs"][
                        "reco10Jet"
                    ].replace("%SYS%", "NOSYS"),
                )
            )

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip ghost assocciation VR jet sequence for now")
        else:
            cfg.merge(
                LargeUFOJetGhostVRJetAssociationAlgCfg(
                    flags,
                    inputLargeRUFOJetContainerName=containers["outputs"][
                        "reco10UFOJet"
                    ].replace("%SYS%", "NOSYS"),
                )
            )

        if flags.Input.isMC:
            log.info("Adding truth particle info seq")
            cfg.merge(
                TruthParticleInformationAlgCfg(
                    flags,
                    inputSMContainerName=containers["inputs"]["truthSMParticles"],
                    inputBSMContainerName=containers["inputs"]["truthBSMParticles"],
                    outputContainerName=containers["outputs"]["truthParticles"],
                )
            )
            cfg.merge(eventCounterCfg('n_truth_particle'))

    ########################################################################
    # Begin postprocessing
    ########################################################################

    log.info("Adding Overlap Removal sequence")
    overlapInputNames = {
        "electrons": containers["outputs"]["electrons"],
        "photons": containers["outputs"]["photons"],
        "jets": containers["outputs"]["reco4Jet"],
    }
    if do_muons:
        overlapInputNames["muons"] = containers["outputs"]["muons"]

    if not is_daod_physlite:
        overlapInputNames["fatJets"] = containers["outputs"]["reco10Jet"]
        overlapInputNames["fatUFOJets"] = containers["outputs"]["reco10UFOJet"]

    overlapOutputNames = {k: f"{v}_OR" for k, v in overlapInputNames.items()}

    cfg.merge(
        OverlapAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputNames=overlapInputNames,
            outputNames=overlapOutputNames,
            doFatJets=not is_daod_physlite,
            doMuons=do_muons,
        )
    )
    cfg.merge(eventCounterCfg('n_overlap'))

    if flags.Analysis.do_resolved_dihiggs_analysis and not flags.Analysis.disable_calib:
        cfg.merge(
            ResolvedAnalysisCfg(
                flags,
                SmallJetKey=containers["outputs"]["reco4Jet"].replace("%SYS%", "NOSYS"),
            )
        )
        cfg.merge(eventCounterCfg('n_resolved'))
    if flags.Analysis.do_boosted_dihiggs_analysis and not flags.Analysis.disable_calib:
        cfg.merge(
            BoostedAnalysisCfg(
                flags,
                LargeJetKey=containers["outputs"]["reco10Jet"].replace(
                    "%SYS%", "NOSYS"
                ),
            )
        )
        cfg.merge(eventCounterCfg('n_merged'))

    return cfg
