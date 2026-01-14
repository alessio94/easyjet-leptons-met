# TTree output configuration

All output writing is done via the TTree algorithms provided in [`PhysicsAnalysis/AsgAnalysisAlgorithms`](https://gitlab.cern.ch/atlas/athena/blob/main/PhysicsAnalysis/Algorithms/AsgAnalysisAlgorithms/):
- `TreeMakerAlg` creates the TTree
- `AsgxAODNTupleMakerAlg` is the main algorithm that defines a translation of `xAOD` variables to output branches
- `TreeFillerAlg` handles the actual operation of reading `xAOD` data and filling the `TTree` branches
Further details are in the [atlassoftwaredocs AnalysisSWTutorial](https://atlassoftwaredocs.web.cern.ch/AnalysisSWTutorial/basic_trees/).

In the basic job run by `easyjet-ntupler`, the full TTree creation is handled by the `minituple_cfg()` function in [`minituple_config.py`](./minituple_config.py). For convenience, the function [`EasyjetHub.steering.main_sequence_config.output_cfg`](../../steering/main_sequence_config.py), accessible also as `EasyjetHub.hub.output_cfg`, will call `minituple_cfg()` if any TTree output file is specified (via `-O` or `--out-file`), while also setting up support for `h5` and `xAOD` output formats. `output_cfg()` is the recommended wrapper around `minituple_cfg()` that should be used by analyses.

Each job is permitted to define a list of TTree configurations, which populate the `flags.Analysis.ttree_configs` list. For each of these configurations:
- A structured set of branches is defined when calling `minituple_cfg()`, passing in the current TTree configuration. The detailed branch lists and configuration logic are defined in modules in this directory, corresponding to each of the object containers. The template TTree configuration is defined in [`EasyjetHub/share/AnalysisMiniTree-config.yaml`](../../../share/AnalysisMiniTree-config.yaml), including the tree name and the lists of containers and details to write. The template has all flags defaulted to off, so all desired content should be explicitly switched on (see e.g. [`EasyjetHub/share/RunConfig.yaml`](../../../share/RunConfig.yaml)).
- Arbitrary additions can be made in two ways:
  1. A fixed set of branches can be added setting the `extra_output_branches` flag under the TTree config in the yaml.
  2. Mainly for cases where the branch list needs to be generated dynamically with python code, `minituple_cfg()` also takes an `extra_output_branches` argument, which is combined with any list set in yaml.
The syntax for `extra_output_branches` is specified below.

Other arguments to `minituple_cfg()` permit direct configuration of the tree name, target ROOT file and the directory to which the tree is written. [^1]


### Tree configuration in yaml

A single TTree configuration object contains these flags, among others:
- `tree_name`, `stream_name`: specification to `THistSvc` how to place the TTree in the output file
- `reco_outputs`, `truth_outputs`: mappings, specifying the set of containers in StoreGate to be written out. Each can either be specified explicitly, or with reference to a flag under `flags.Analysis`. The flags will be searched first. E.g. consider the following fragment:
```yaml
reco_outputs:
  small_R_jets: container_names.output.reco4PFlowJet
  electrons: AnalysisElectrons
```
  - The job will try to write the jet container stored as `flags.Analysis.container_names.output.reco4PFlowJet`[^1] -- and will break if the flag is not defined.
  - The job will try to write the electron container `AnalysisElectrons` (unless there exists a flag called `flags.Analysis.AnalysisElectrons`, in which case the value of this flag would be used -- this should be avoided, as potentially misleading).
- `container_options`: flags specifying details to be added for specific containers, e.g.
```yaml
collection_options:
  small_R_jets:
    btag_info: True
  large_R_jets:
    substructure_info: True
```
- `slim_variables_with_syst`: Forwarded to all `BranchManager` objects steering the configuration of this TTree.

For the full set of flags, see [`EasyjetHub/share/AnalysisMiniTree-config.yaml`](../../../share/AnalysisMiniTree-config.yaml).

[^1]: For convenience and to maintain flexibility, the python configuration operates on input/output containers specified by alias according to the input format in `EasyjetHub/share/container-names-DAOD_PHYS(LITE).yaml`.[^2] The `container_names.input/output` flags should always be specified, but additional `container_names` subflags could be used for analysis-specific customisation.

[^2]: When operating on `DAOD_PHYSLITE`, calibrations can be turned off, in which case the `output` containers are not produced.


### Output syntax

The basic syntax for defining an output branch is as follows:
```
[xAOD container name].[aux variable name] -> [output branch name]
```
Note that this code is not powerful enough to call `xAOD` interface methods, and can only translate existing branches from the `xAOD` auxiliary data to the output tree. E.g. on an unmodified AOD/DAOD:
```python
Electrons.pt -> electrons_pt
# Works, as pt is a variable in the Electrons container

Electrons.px -> electrons_px
# Does not work, as px is not stored in Electrons
# The code cannot call Electrons[i].px()
```
For writing arbitrary variables, it is necessary to add a decoration to some `xAOD` object that holds the relevant information. Typically, one would decorate the the object container with information pertaining to the full container. Information defined once per event can instead be decorated on `EventInfo`.


## BranchManager helper class

The `BranchManager` class defined in [`branch_manager.py`](./branch_manager.py) assists with handling more complex branch logic, in particular handling of object overlap removal and systematic variations. This helps keep all naming conventions consistent across collections and job configurations.

An example (fictional)branch manager setup might look like:
```python
jet_branches = BranchManager(
    input_container='AntiKt4EMPFlowJets',
    output_prefix='pf_jets'
    do_overlap_removal=False
    systematics_option=SystOption.ALL_SYST
    required_flags=['Analysis.do_pf_jets']
    variables=['pt','eta','phi','m','NNJvt']
)
```
which would format branches like:
```python
# nominal
AntiKt4EMPFlowJets.pt -> pf_jets_NOSYS_pt
AntiKt4EMPFlowJets.eta -> pf_jets_NOSYS_eta
AntiKt4EMPFlowJets.phi -> pf_jets_NOSYS_phi
AntiKt4EMPFlowJets.m -> pf_jets_NOSYS_m
AntiKt4EMPFlowJets.NNJvt -> pf_jets_NOSYS_NNJvt
# variation 1 (JET__JES__1up)
AntiKt4EMPFlowJets.pt -> pf_jets_JET__JES__1up_pt
AntiKt4EMPFlowJets.eta -> pf_jets_JET__JES__1up_eta
AntiKt4EMPFlowJets.phi -> pf_jets_JET__JES__1up_phi
AntiKt4EMPFlowJets.m -> pf_jets_JET__JES__1up_m
AntiKt4EMPFlowJets.NNJvt -> pf_jets_JET__JES__1up_NNJvt
# variation 2 (JET_JES__1down)
# etc ...
```

Systematics can be switched only only for certain variables using the `systs_only_for` attribute of the `BranchManager`. 
Conversely, systematics can we switched off for certain variables using the `syst_not_for` attribute.   The former takes 
precedence.  Setting the `slim_variables_with_syst` flag only stores systematic variations for pt/Et-related branches.

Setting the `do_overlap_removal` flag inserts `OR` into the output prefix, and uses the overlap-removed container:
```python
AntiKt4EMPFlowJets_OR.pt -> pf_jets_OR_NOSYS_pt
# etc
```

The `required_flags` argument asserts that all flags listed by name are `True`, and provides a means to ensure that necessary calibration or decoration algorithms have been scheduled to provide the information needed to write the configured output branches.