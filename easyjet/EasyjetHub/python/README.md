# Python code structure

## Top level configuration

The entry point for configuring an `easyjet` job is `hub.py`, which provides access to top-level imports from `steering`, which can be used to set up a minimal executable extensible with analysis-specific features. The basic `easyjet-ntupler` executable makes use of this code while adding some additional features to facilitate unit tests.

To get a job running in the fewest possible lines of code, an executable could be written as:
```python
from EasyjetHub import hub
# Fill the configuration flags from the
# parsed arguments
# The flags are locked so that the configuration of job subcomponents
# cannot modify them silently/unpredictably.
# This step additionally makes a type substitution of the category
# `flags.Analysis`, making iteration over the analysis flags easier.
flags, args = hub.analysis_configuration()
# Get a standard ComponentAccumulator with the following infrastructure:
# - basic services for event loop, messaging etc
# - apply preselection on triggers and data quality
# - build event info with calibrations etc
seqname = "EasyjetSeq"
cfg = hub.default_sequence_cfg(flags, seqname)
# Add output sequence, with content and format defined by flags
cfg.merge(hub.output_cfg(flags, seqname))
# Execute the job
hub.run_job(flags, args, cfg).isSuccess()
```

### Directories
- `algs/`: Modules configuring algorithms that run in the event loop
  - `calibration/`
    - CP alg configuration modules for four-vector and scale factor calibrations
    - See `/algs/calibration/README.md` for an overview of custom operation
  - `postprocessing/`
    - Configuration modules for operations performed after calibration
  - `preselection/`
    - Configuration modules for selection operations performed before object calibrations
  - `truth/`
    - Configuration modules for algorithms extracting truth information
- `output/`: Modules configuring file output writing
  - `h5/`
    - HDF5 output
  - `ttree/`
    - ROOT TTree output
    - See [`output/ttree/README.md`](./output/ttree/README.md) for documentation on defining branch lists for a container
  - `xaod.py` provides basic support for writing an `xAOD` output file
- `steering/`: Modules configuring the global job state, providing features such as:
  - Argument parsing
  - Standard configuration flags
  - Extraction of commonly needed input files e.g. Good Run Lists and Pileup Reweighting files
  - Internal object container names
  - Context-dependent trigger lists
  - `utils/`: Helper modules used in the steering code
