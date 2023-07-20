# TTree output configuration

All output writing is done via the TTree algorithms provided in [`PhysicsAnalysis/AsgAnalysisAlgorithms`](https://gitlab.cern.ch/atlas/athena/blob/main/PhysicsAnalysis/Algorithms/AsgAnalysisAlgorithms/):
- `TreeMakerAlg` creates the TTree
- `AsgxAODNTupleMakerAlg` is the main algorithm that defines a translation of `xAOD` variables to output branches
- `TreeFillerAlg` handles the actual operation of reading `xAOD` data and filling the `TTree` branches
Further details are in the [atlassoftwaredocs AnalysisSWTutorial](https://atlassoftwaredocs.web.cern.ch/AnalysisSWTutorial/basic_trees/).

There are two routes to adding branches into the output file:
- A structured set of branches is defined in [`minituple_config.py`](./minituple_config.py), and is steerable via configuration flags. The detailed branch lists and configuration logic are defined in modules in this directory, corresponding to each of the object containers.
- Arbitrary additions can be made by setting the `Analysis.extra_output_branches` flag, which is a list of output branch expressions following the syntax below.

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

Setting the `do_overlap_removal` flag inserts `OR` into the output prefix, and uses the overlap-removed container:
```python
AntiKt4EMPFlowJets_OR.pt -> pf_jets_OR_NOSYS_pt
# etc
```

The `required_flags` argument asserts that all flags listed by name are `True`, and provides a means to ensure that necessary calibration or decoration algorithms have been scheduled to provide the information needed to write the configured output branches.