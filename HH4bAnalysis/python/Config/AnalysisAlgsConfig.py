from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.Algs.DiHiggsAnalysis import DiHiggsAnalysisAlgCfg
from HH4bAnalysis.Algs.Electrons import ElectronAnalysisSequenceCfg
from HH4bAnalysis.Algs.Event import (
    EventSelectionAnalysisSequenceCfg,
    GeneratorAnalysisSequenceCfg,
    PileupAnalysisSequenceCfg,
    TriggerAnalysisSequenceCfg,
)
from HH4bAnalysis.Algs.Jets import (
    FatJetAnalysisSequenceCfg,
    JetAnalysisSequenceCfg,
    VRJetAnalysisSequenceCfg,
)
from HH4bAnalysis.Algs.Muons import MuonAnalysisSequenceCfg
from HH4bAnalysis.Algs.Photons import PhotonAnalysisSequenceCfg
from HH4bAnalysis.Algs.Postprocessing import OverlapAnalysisSequenceCfg
from HH4bAnalysis.Config.Base import cache_metadata, update_metadata
from HH4bAnalysis.utils.containerNameHelper import get_container_names
from HH4bAnalysis.utils.inputsHelper import is_physlite
from HH4bAnalysis.utils.logHelper import log


# Generate the algorithm to do the dumping.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def AnalysisAlgsCfg(
    flags,
    btag_wps,
    vr_btag_wps,
    trigger_chains=[],
    do_muons=True,
    metadata_cache=None,
    do_loose=False,
    do_PRW=False,
    prw_files=[],
    lumicalc_files=[],
    do_dihiggs_analysis=False,
):
    if metadata_cache:
        update_metadata(metadata_cache)
    dataType = "mc" if flags.Input.isMC else "data"
    is_daod_physlite = is_physlite(flags)
    log.info(
        f"Self-configured: dataType: '{dataType}', is PHYSLITE? {is_daod_physlite}"
    )

    log.debug(f"Containers available in dataset: {flags.Input.Collections}")

    # TODO: no DL1d branches in PHYSLITE yet
    if is_daod_physlite:
        btag_wps = [wp.replace("DL1dv00", "DL1r") for wp in btag_wps]

    cfg = ComponentAccumulator()

    # Create SystematicsSvc explicitly:
    cfg.addService(CompFactory.getComp("CP::SystematicsSvc")("SystematicsSvc"))

    log.info("Adding trigger analysis algs")
    # Removes events failing trigger and adds variable to EventInfo
    # if trigger passed or not, for example:
    # EventInfo.trigger_name
    cfg.merge(TriggerAnalysisSequenceCfg(flags, dataType, trigger_chains))

    log.info("Add DQ event filter sequence")
    # Remove events failing DQ criteria
    cfg.merge(
        EventSelectionAnalysisSequenceCfg(flags, dataType, grlFiles=[], loose=do_loose)
    )

    log.info(
        f"Do PRW is {do_PRW}. {'Add' if do_PRW else 'Skip'} pileup re-weight sequence"
    )
    if do_PRW:
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

    containers = get_container_names(flags)

    log.info("Add electron seq")
    cfg.merge(
        ElectronAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=containers["inputs"]["electrons"],
            outputContainerName=containers["outputs"]["electrons"],
        )
    )

    log.info("Add photon seq")
    cfg.merge(
        PhotonAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=containers["inputs"]["photons"],
            outputContainerName=containers["outputs"]["photons"],
        )
    )

    if do_muons:
        log.info("Add muon seq")
        cfg.merge(
            MuonAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=containers["inputs"]["muons"],
                outputContainerName=containers["outputs"]["muons"],
            )
        )

    log.info("Add jet seq")
    cfg.merge(
        JetAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=containers["inputs"]["reco4Jet"],
            outputContainerName=containers["outputs"]["reco4Jet"],
            workingPoints=btag_wps,
            is_daod_physlite=is_daod_physlite,
        )
    )

    if is_daod_physlite:
        log.warning("On PHYSLITE, skip large-R jet sequence for now")
    else:
        log.info("Add large-R jet seq")
        cfg.merge(
            FatJetAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=containers["inputs"]["reco10Jet"],
                outputContainerName=containers["outputs"]["reco10Jet"],
            )
        )

    if is_daod_physlite:
        log.warning("On PHYSLITE, skip VR jet sequence for now")
    else:
        log.info("Add VR jet seq")
        cfg.merge(
            VRJetAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=containers["inputs"]["vrJet"],
                outputContainerName=containers["outputs"]["vrJet"],
                workingPoints=vr_btag_wps,
            )
        )

    ########################################################################
    # Begin postprocessing
    ########################################################################

    log.info("Add Overlap Removal sequence")
    overlapInputNames = {
        "electrons": containers["outputs"]["electrons"],
        "photons": containers["outputs"]["photons"],
        "jets": containers["outputs"]["reco4Jet"],
    }
    if do_muons:
        overlapInputNames["muons"] = containers["outputs"]["muons"]

    if not is_daod_physlite:
        overlapInputNames["fatJets"] = containers["outputs"]["reco10Jet"]

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

    if do_dihiggs_analysis:
        cfg.merge(
            DiHiggsAnalysisAlgCfg(
                flags,
                SmallJetKey=containers["outputs"]["reco4Jet"].replace("%SYS%", "NOSYS"),
                LargeJetKey=containers["outputs"]["reco10Jet"].replace(
                    "%SYS%", "NOSYS"
                ),
                btag_wps=btag_wps,
                vr_btag_wps=vr_btag_wps,
            )
        )

    if metadata_cache:
        cache_metadata(metadata_cache)

    return cfg
