# Algorithm configuration

The modules contained in this directory and subdirectories are responsible for scheduling algorithms and algorithm sequences in the event loop. Apart from the core infrastructure components handled by `steering/`, all algorithm configurations should be defined here.

## Structure

General algorithms or aggregated configuration go in the top level of the directory.

In particular, `cpalgs_config.py` provides a unified configuration of all CP algorithms performing calibration, preselection and overlap removal, with the possibility to enable/disable certain algorithms via the configuration flags.

If more fine-grained scheduling is required, the algorithm configuration modules can be imported directly from the subfolders.

### CP algorithm configuration

CP algorithm configurations are defined with wrappers for the basic schedulers defined in the packages in [`athena/PhysicsAnalysis/Algorithms`](https://gitlab.cern.ch/atlas/athena/-/tree/master/PhysicsAnalysis/Algorithms).

Selected modifications of the calibration configurations can be made by defining configuration flags, which can then be set in the run configuration YAML files. In the longer term, the full configurations may be placed in the YAML to permit greater flexibility and the possibility to define multiple presets.

#### Internal container naming

The names of `xAOD` object containers read from and written to `StoreGate` by the `easyjet` components are defined in [`steering/container_names.py`](../steering/container_names.py), and may vary depending on the input file format. Hardcoded container names should not be used, apart from containers such as `EventInfo` that are guaranteed to be present and are not calibrated -- or can be used without copying.
