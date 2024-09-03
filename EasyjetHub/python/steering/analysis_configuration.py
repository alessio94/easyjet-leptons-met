from pathlib import Path
import yaml
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.Enums import LHCPeriod
from TrigGlobalEfficiencyCorrection.TriggerLeg_DictHelpers import TriggerDict
from AthenaCommon.Constants import INFO

from EasyjetHub.steering.sample_metadata import (
    get_data_type,
    get_run_years,
    update_metadata,
    has_metadata,
    cache_metadata,
)
from EasyjetHub.steering.argument_parser import (
    AnalysisArgumentParser,
    fill_from_args,
)
from EasyjetHub.steering.utils.log_helper import log, setRogueLoggers
from EasyjetHub.steering.utils.config_flags import (
    fill_flags_from_runconfig,
    lock_merged_config_flags,
)


###########################################
# other, less awesome functions
###########################################
def analysis_configuration(parser="default"):

    if parser == "default":
        parser = AnalysisArgumentParser()

    # Initialise the job configuration flags, some of which will be autoconfigured.
    # These are used for steering the job, and include e.g. the input file (list).
    flags = initConfigFlags()

    # Adjust the loop manager to announce the event number less frequently.
    # Makes a big difference if running over many events
    flags.Exec.EventPrintoutInterval = 500

    args = fill_from_args(flags, parser=parser)
    log.setLevel(flags.Exec.OutputLevel)
    if args.config_only and has_metadata(flags):
        log.info("found metadata for input files, skipping config run")
        sys.exit(0)
    setRogueLoggers(flags.Exec.OutputLevel)
    # Write user options to flags.Analysis
    fill_flags_from_runconfig(args, flags, parser.overwrites)
    flags.loadAllDynamicFlags()

    # Determined from input file without explicitly reading metadata
    def is_physlite(flags):
        return flags.Input.ProcessingTags == ["StreamDAOD_PHYSLITE"]

    flags.addFlag("Input.isPHYSLITE", lambda prevFlags: is_physlite(prevFlags))

    flags = flags.cloneAndReplace("Analysis.container_names", (
        "Analysis.full_container_names." + (
            "physlite" if flags.Input.isPHYSLITE else "phys")))

    # disable some sequences for fast-run
    if flags.Analysis.fast_test:
        flags.Analysis.do_overlap_removal = False
        flags.Analysis.do_muons = False
        flags.Analysis.Small_R_jet.runBJetPtCalib = False

        def disable_reco_muons(tree_flags):
            _tree_flags = {k: v for k, v in tree_flags.items()}
            _tree_flags['reco_outputs']['muons'] = ''
            return _tree_flags
        flags.Analysis.ttree_output = [
            disable_reco_muons(f) for f in flags.Analysis.ttree_output
        ]

    log.info(f"Operating on input files {flags.Input.Files}")

    fileMD = GetFileMD(flags.Input.Files)
    if flags.Analysis.cache_metadata:
        update_metadata(Path("metadata.json"))
    flags.addFlag("Input.AMITag", fileMD.get("AMITag", ""))
    flags.addFlag("Input.SimulationFlavour",
                  fileMD.get("SimulationFlavour", ""))
    flags.addFlag("Analysis.DataType", get_data_type)
    flags.addFlag("Analysis.Years", get_run_years)
    log.info(f"Configuring to match dataset from {flags.Analysis.Years}")
    if max(flags.Analysis.Years) <= 2018:
        flags.addFlag("Analysis.Run", 2)
    elif min(flags.Analysis.Years) >= 2022 and max(flags.Analysis.Years) <= 2025:
        flags.addFlag("Analysis.Run", 3)
    elif min(flags.Analysis.Years) >= 2029:
        flags.addFlag("Analysis.Run", 4)
    else:
        raise RuntimeError("Invalid list of years, cannot combine runs")
    log.info(f"Configured years match Run {flags.Analysis.Run}")

    if flags.Analysis.Run == 4:
        flags.GeoModel.Run = LHCPeriod.Run4

    flags.PerfMon.doFullMonMT = flags.Exec.OutputLevel <= INFO

    flags.addFlag(
        "Analysis.TriggerChains",
        lambda prevFlags: get_trigger_chains(prevFlags)
    )
    flags.addFlag(
        "Analysis.TriggerChainsSF",
        lambda prevFlags: get_trigger_chains_scale_factor(prevFlags)
    )

    do_PRW = flags.Input.isMC and not flags.Input.isPHYSLITE
    flags.addFlag("Analysis.doPRW", do_PRW)

    # set HH orthogonality flags, if orthogonality is contained in the given config
    if 'orthogonality' in flags.Analysis:
        setHHOrthFlags(flags)

    log.info(
        f"Self-configured: datatype: '{flags.Analysis.DataType}', "
        f"is PHYSLITE? {flags.Input.isPHYSLITE}"
    )

    if flags.Analysis.cache_metadata:
        cache_metadata(Path("metadata.json"))

    # Flags need to be locked to be used.
    # All values should be specified via the argument parser
    # or in the yaml config.
    # Getting the yaml configuration to mesh with the flags requires a
    # bit more postprocessing so you can't use flags.lock().
    lock_merged_config_flags(flags)

    if args.dump_analysis_config_flags:
        yaml.dump(
            flags.Analysis.as_mutable(),
            open(args.dump_analysis_config_flags, 'w')
        )

    if args.dump_full_config_flags:
        flags_as_dict = flags.asdict()
        # convert the analysis flags back to a dict too
        flags_as_dict['Analysis'] = flags.Analysis.as_mutable()
        yaml.dump(flags_as_dict, open(args.dump_full_config_flags, 'w'))

    return flags, args


def get_trigger_chains(flags):

    trigger_year_list = flags.Analysis.trigger_year
    if trigger_year_list == "Auto":
        trigger_year_list = flags.Analysis.Years
        log.info(
            "Self-configured trigger list for years: "
            f"{', '.join(str(year) for year in trigger_year_list) or None}"
        )

    trigger_chains = set()
    if flags.hasCategory("Analysis.Trigger"):
        try:
            for year in trigger_year_list:
                trigger_chains |= set(
                    flags.Analysis.Trigger.selection.chains[str(year)])
        except KeyError as err:
            log.error(f"Trigger chains for {year} not defined.")
            raise err

    return list(trigger_chains)


def get_trigger_chains_scale_factor(flags, obj=None):
    if not flags.Analysis.Trigger.scale_factor.doSF:
        return {}

    if obj:
        if not hasattr(flags.Analysis.Trigger.scale_factor, obj):
            return {}
        triggerChains = getattr(flags.Analysis.Trigger.scale_factor, obj).chains
    else:
        triggerChains = (
            flags.Analysis.Trigger.scale_factor.chains
            if hasattr(flags.Analysis.Trigger.scale_factor, "chains") else
            flags.Analysis.Trigger.selection.chains)

    triggerChainsDict = {
        str(year): [trigger for trigger in triggerChains[str(year)]]
        for year in flags.Analysis.Years}
    return triggerChainsDict


def get_trigger_legs_scale_factor_list(flags, obj=None):
    triggerChainsPerYear = get_trigger_chains_scale_factor(flags, obj)
    if not triggerChainsPerYear:
        return []

    trigger_legs = set()
    triggerDict = TriggerDict()
    for year in flags.Analysis.Years:
        triggerChains = triggerChainsPerYear[str(year)]

        if obj == "Electron" or obj == "Muon":
            for chain in triggerChains:
                chain = chain.replace("HLT_", "").replace(" || ", "_OR_")
                legs = triggerDict[chain]
                prefix = 'e' if obj == "Electron" else 'mu'
                if len(legs) == 0:
                    if chain.startswith(prefix) and chain[len(prefix)].isdigit:
                        trigger_legs.add(chain)
                else:
                    for leg in legs:
                        if leg.startswith(prefix) and leg[len(prefix)].isdigit:
                            trigger_legs.add(leg)

        else:
            for chain in triggerChains:
                chain = chain.replace("HLT_", "")
                trigger_legs.add(chain)

    return list(trigger_legs)


def setHHOrthFlags(flags):
    # If running orthogonality checks, need to ensure you process
    # the collections required by the orthogonality algorithm,
    # which may or may not be specified to run in your given ntupler
    # In this case, only assuming the HH orthogonality check exists.
    # This can be configured differently if different
    # orthogonality checks and algorithms are introduced
    if flags.Analysis.orthogonality.do_orth_check:

        # need to process small R jets and photons for the HH orthogonality checks
        # Also need to set all other objects to True and define a baseline ID
        # because of overlap removal
        flags.Analysis.do_small_R_jets = True
        flags.Analysis.do_photons = True
        flags.Analysis.do_electrons = True
        flags.Analysis.do_muons = True
        flags.Analysis.do_met = True
        flags.Analysis.do_taus = True

        # if nominal analysis is running a WP different
        # from the orthogonality WP, should make sure both
        # are run so that they're accessible by the time you
        # get to the analysis and orthogonality algorithms

        # b-jets
        HHBjet_WP = flags.Analysis.orthogonality.HHBjet.btag_wp
        # Take WP from orthogonality configuration if
        # the collection was not already going to be run
        if flags.Analysis.Small_R_jet.btag_wp == "":
            flags.Analysis.Small_R_jet.btag_wp = HHBjet_WP
        # WP is there but it differs from the orthogonality WP
        elif (flags.Analysis.Small_R_jet.btag_wp != ""
              and flags.Analysis.Small_R_jet.btag_wp != HHBjet_WP):
            flags.Analysis.Small_R_jet.btag_extra_wps += [HHBjet_WP]

        # Photons (ID and Iso)
        HHPhoton_ID = flags.Analysis.orthogonality.HHPhoton.ID
        HHPhoton_Iso = flags.Analysis.orthogonality.HHPhoton.Iso
        if flags.Analysis.Photon.ID == "" and flags.Analysis.Photon.Iso == "":
            flags.Analysis.Photon.ID = HHPhoton_ID
            flags.Analysis.Photon.Iso = HHPhoton_Iso
        elif flags.Analysis.Photon.ID != HHPhoton_ID or flags.Analysis.Photon.Iso != HHPhoton_Iso: # noqa
            flags.Analysis.Photon.extra_wps += [(HHPhoton_ID, HHPhoton_Iso)]

        # Electrons (ID and Iso)
        HHElectron_ID = flags.Analysis.orthogonality.HHElectron.ID
        HHElectron_Iso = flags.Analysis.orthogonality.HHElectron.Iso
        if flags.Analysis.Electron.ID == "" and flags.Analysis.Electron.Iso == "":
            flags.Analysis.Electron.ID = HHElectron_ID
            flags.Analysis.Electron.Iso = HHElectron_Iso
        elif flags.Analysis.Electron.ID != HHElectron_ID or flags.Analysis.Electron.Iso != HHElectron_Iso: # noqa
            flags.Analysis.Electron.extra_wps += [(HHElectron_ID, HHElectron_Iso)]

        # Muon (ID and Iso)
        HHMuon_ID = flags.Analysis.orthogonality.HHMuon.ID
        HHMuon_Iso = flags.Analysis.orthogonality.HHMuon.Iso
        if flags.Analysis.Muon.ID == "" and flags.Analysis.Muon.Iso == "":
            flags.Analysis.Muon.ID = HHMuon_ID
            flags.Analysis.Muon.Iso = HHMuon_Iso
        elif flags.Analysis.Muon.ID != HHMuon_ID or flags.Analysis.Muon.Iso != HHMuon_Iso: # noqa
            flags.Analysis.Muon.extra_wps += [(HHMuon_ID, HHMuon_Iso)]

        # Taus (ID only, no Iso)
        HHTau_ID = flags.Analysis.orthogonality.HHTau.ID
        if flags.Analysis.Tau.ID == "":
            flags.Analysis.Tau.ID = HHTau_ID
        elif flags.Analysis.Tau.ID != HHTau_ID:
            flags.Analysis.Tau.extra_wps += [HHTau_ID]
