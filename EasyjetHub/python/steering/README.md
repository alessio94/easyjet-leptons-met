# Easyjet configuration

An `easyjet` job is steered primarily by configuration written in `yaml`, with selected details being set by command line arguments.

Within the python configuration code, the top-level configuration is passed around to configuration functions using the Athena [`AthConfigFlags`](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaConfiguration/python/AthConfigFlags.py) construct (assigned to the variable `flags`), allowing these functions awareness of job-wide settings/context. For input/output file configuration, some flags are added to `flags.Input/Output`, while settings governing the event loop are propagated to `flags.Exec`. Otherwise, all configuration specific to `easyjet` is held in the `flags.Analysis` category.

There are two phases to the configuration:
1. Flags are fully defined by parsing yaml and command line arguments. They are immediately locked so no further manipulation is possible.
2. Flags are passed to configuration functions to generate the analysis algorithm sequence.


## Configuration in yaml

The input `easyjet` RunConfig file should define the vast majority of the `flags.Analysis` settings. It is specified via the `-c (--run-config)` argument, defaulting to [`EasyjetHub/RunConfig.yaml`](../share/base-config.yaml/share/RunConfig.yaml)

The core configuration is defined in [`EasyjetHub/share/base-config.yaml`](../share/base-config.yaml/share/base-config.yaml). A full analysis configuration will typically begin with an `include` of this configuration, after which the specific settings can be overridden as necessary, for example:
```yaml
include: EasyjetHub/base-config.yaml
do_trigger_filtering: true
systematics_regex: [".*"]
```
Files to be included need to be installed via `CMake`, and then can be specified as `[package]/[filename]`.

The `yaml` parsing implemented in `easyjet` permits nesting of `include:` statements, constructing a structured configuration flag hierarchy. The hierarchy should be defined at the level of the including file, e.g.
```yaml
container_names:
  include: container-names-DAOD_PHYS.yaml
```
which expands to
```yaml
container_names:
  inputs:
    reco4PFlowJet: 'AntiKt4EMPFlowJets'
    ...
```

In addition to the associative definitions above, where all values are accessed by keys (parsed initially to a python `dict`), sequential definitions (parsed initially to a python `list`) are also permitted. Both simple and complex lists are possible:
```yaml
truthDecayModes: ["bbbb"]
btag_wps:
    - GN120220509_FixedCutBEff_70
    - GN120220509_FixedCutBEff_77
ttree_output:
  - # List item 1
    tree_name: 'PFlowJetTree'
    reco_outputs:
      small_R_jets: 'container_names.input.reco4PFlowJet'
      large_R_UFO_jets: 'container_names.input.reco10UFOJet'
  - # List item 2
    tree_name: 'TopoJetTree'
    reco_outputs:
      small_R_jets: 'container_names.input.reco4TopoJet'
      large_R_UFO_jets: 'container_names.input.reco10TopoJet'
```

### Illegal yaml values

Some potential pitfalls in writing the `yaml` are as follows:
- `None` is an invalid value for `AthConfigFlags` and will be rejected. For default 'inactive' behaviour, it is recommended to set `[]` or `''` as appropriate, indicating the type of the flag.
- `None` can also appear if defining two keys in succeeding lines without assigning a value to the first
```yaml
key1:
key2:
  'blah'

# Produces {'key1':None, 'key2':'blah'}
```
- Use either sequential or associative definitions. Assigning a name to a list entry is not possible. For example, this will fail:
```yaml
ttree_output:
  - PFlowJetTree:
    tree_name: 'PFlowJetTree'
    ...
# Produces:
# { 'ttree_output: [
#   'PFlowJetTree': None,
#   'tree_name': 'PFlowJetTree',
#   ...
# ] }
```

## Command line arguments

The `yaml` configuration is complemented by a limited set of command line arguments, which are primarily for setting any parameters that may change fairly frequently, e.g. input and output file names, or be helpful for debugging, e.g. the maximum number of events to run, or the regex(es) used to select systematic variations. For the basic `easyjet-ntupler` executable, these are fully defined in [`EasyjetHub/steering/argument_parser.py`](../steering/argument_parser.py).

For custom analysis executables, additional arguments can be defined by extending the parser prior to the generation of the configuration flags:
```python
parser = AnalysisArgumentParser()
# Arguments set with `add_argument` need to be explicitly handled,
# being retrieved from the `args` namespace
parser.add_argument(
    '--greeting',
    default='hello',
    help='A friendly message to start the job',
)
# Arguments set with `add_analysis_argument` are directly
# inserted into flags.Analysis, and are better for
# transmission throughout the analysis configuration
parser.add_analysis_argument(
    '--extra-verbose-output',
    action='store_true',
    help='Print even more verbose outputs than steered by LogLevel',
)

# Fill the configuration flags from the
# parsed arguments
# Returned flags are locked.
flags, args = analysis_configuration(parser)

print(args.greeting)

if flags.Analysis.extra_verbose_output:
    print('Hope you have enough space for this log file!')
```

### Overwriting yaml flags from the command line

In limited cases, it may be convenient to have a flag defined in the yaml that sets the behaviour in most jobs, but have the capability to modify it from the command line. This is supported by adding `overwrite=True` to `parser.add_analysis_argument`. In the case of boolean arguments, there is a further requirement that the value be set explicitly from the command line to avoid ambiguity, as default values in the parser and yaml could diverge. A boolean overwrite argument is therefore specified like:
```python
self.add_analysis_arg(
    "-y", "--do-CP-systematics",
    # Options to assist with coherent handling of bool overwrites
    **AnalysisArgumentParser.oropt
)

# Set on command like as "--do-CP-systematics true"
# Any capitalisation of 'true'/'false' is permitted
```


## The `ConfigItem` class

During the flag creation, the `flags.Analysis` category is transformed from `AthConfigFlags` into a custom class, `ConfigItem`. This is primarily a technical detail that most users need not pay much attention to. The differences between a `ConfigItem` instance and a locked `AthConfigFlags` instance are mainly that:
- It is easier to iterate over `ConfigItem`, as this class provides a python `dict`-like interface including `keys()`, `values()` and `items()` functions.
- `ConfigItem` supports lists of `ConfigItem`, whereas `AthConfigFlags` is entirely associative. Practically speaking, a list of `yaml` objects is only accessible as a `list[dict]` in `AthConfigFlags`, which terminates the object-like (`flags.a.b`) access as soon as a list appears.
- Where a subcategory of `AthConfigFlags` e.g. `flags.Input` returns a `FlagAddress` with access to the entire hierarchy, no backwards access is possible in `ConfigItem` -- `flags.Analysis.container_names.input` cannot be used to access `flags.Analysis.container_names.output`.
- `ConfigItem` works only with static assignment, whereas `AthConfigFlags` can define values or categories dynamically via generators that are evaluated only on flag/category access.

Some consequences are that:
- All `AthConfigFlags` dynamic flags are forced to be evaluated before conversion to `ConfigItem`, so the whole configuration is complete and explicit at the point of locking.
- It may be practical to extract a subset of `flags.Analysis` as a `ConfigItem` as input to a function, but if this function requires knowledge of any information higher up the hierarchy (e.g. `flags.Input`), then the `flags` object needs to be passed in as well. It is usually more concise, and promotes traceability of the flag structure, to pass the full `flags` construct rather than to define multiple function arguments whose values are set from `flags`. E.g.
```python
def get_small_R_jet_branches(flags, tree_flags):
  if flags.Input.isPHYSLITE:
    # Do PHYSLITE things
  for wp in flags.Analysis.btagging_WPs:
    # Add some wp info to branches
  ...

# Calling this function as
branches = get_small_R_jet_branches(flags, tree_flags)
```
is preferable to
```python
def get_small_R_jet_branches(tree_flags, is_PHYSLITE, btagging_WPs):
  ...

# Calling this function as
branches = get_small_R_jet_branches(tree_flags, flags.Input.isPHYSLITE, flags.Analysis.btagging_WPs)
```
