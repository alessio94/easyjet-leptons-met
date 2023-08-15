# Import definitions from the modules here for convenience

# Custom command line arguments via an parser extended from `argparse`
# This can be extended with analysis-specific arguments as required.
# Arguments are converted into AthConfigFlags
from EasyjetHub.steering.argument_parser import AnalysisArgumentParser

# Initialisation of standard AthConfigFlags
# File metadata is used for limited autoconfiguration
from EasyjetHub.steering.analysis_configuration import (
    analysis_configuration,
)

# Flags need to be locked to be used.
# Getting the yaml configuration to mesh with the flags requires a
# bit more postprocessing so you can't use flags.lock().
from EasyjetHub.steering.utils.config_flags import lock_merged_config_flags

# Standard sequences needed to run an easyjet job on DAOD input
from EasyjetHub.steering.main_sequence_config import (
    # Basic AthAnalysis services: event loop and input file reading
    core_services_cfg,
    # Event selection operations applied before any calibrations
    preselection_cfg,
    # CP algs calibrating objects and populating StoreGate with
    # the calibrated object collections
    # Other common data augmentation operations (e.g. standard truth info)
    event_building_cfg,
    # Global output file configuration, details steered by flags
    output_cfg,
    # This combines all the previous into a single CA,
    # providing concise setup of a full DAOD->output job
    default_sequence_cfg,
)

# Helper for calling execution of the CA module
# Suppresses uninteresting output from Athena infrastructure
# Clears cutflow data file
from EasyjetHub.steering.run import run_job
