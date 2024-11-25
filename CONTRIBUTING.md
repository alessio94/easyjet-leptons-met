Contributing Guidelines
=======================

Anyone is welcome to contribute to this code. If you are working in a final state that isn't currently covered by easyjet some contributions are expected.

There is no need to ask for permission or contact the maintainers before making a merge request. The basic procedure is:

- Fork the repository on gitlab
- Clone the code locally and make any changes you need on an
  appropriately named branch
- Make a merge request

For larger changes or general questions about the code, please open an issue on gitlab.

## Merge request guidelines

Merge requests require successful pipelines and an approval by one of the project developers. If you are adding a new top-level script we encourage you to add a regression test in `EasyjetTests`. See the README in that package for more details.

## Extending the ntupler with analysis-specific algorithms

We expose functions for generating the sequences used in `easyjet-ntupler` via the `hub.py` module, such that a custom executable can be defined that extends the basic job with analysis-specific operations. An annotated example for this can be found in [`bbbbAnalysis/bin/bbbb-ntupler`](./bbbbAnalysis/bin/bbbb-ntupler). See [`EasyjetHub/python/README.md`](./EasyjetHub/python/README.md) and [`EasyjetHub/python/hub.py`](./EasyjetHub/python/hub.py) directly for guidance.

## Package structure

The following `CMake` packages are defined in this repository:
- `EasyjetHub`: Core framework code providing analysis-independent algorithms and job steering.
- `EasyjetTests`: CI testing code.
- `*Analysis`: Analysis-specific code. These packages should not depend on each other.

Other packages are utilities used by the above ones.
Most of them are generic enough that they should (eventually) be migrated to the main Athena repository.

All packages should have a `README.md` file of their own.

## General organization of python modules

Please follow the existing module organisation as laid out in [`EasyjetHub/python/README.md`](./EasyjetHub/python/README.md).
In case of ambiguity we are happy to discuss where additional modules or folders can be inserted.

To distinguish modules defined in `easyjet` from those in in `atlas/athena`, modules producing `ComponentAccumulator` configurations are named in snake case, e.g.
```
EasyjetHub.steering.main_sequence_config.py
```
rather than in camel case e.g.
```
AthenaConfiguration.MainSequencesConfig.py`
```
They either end in `config.py` or are located in a directory named `config`.

## Coding style

Limited code formatting rules are enforced via `cppcheck` and `flake8`. We aim to follow `PEP8` python style conventions, except certain cases where it is more natural to follow ATLAS-like camel-case variable names.

If in doubt, follow the style of the surrounding code.

### Athena-specific guidelines

There are some features in C++ and Athena that should be avoided in some areas.

#### Accessors in xAODs

Athena (specifically xAODs) provide a few ways to access auxiliary data (or "decorations"). When algorithms need to account for systematic variations [systematic handles][sh] are the only option. In other cases xAODs give a few options, which are listed in descending order of preference below:

1. [Read][rh] and [write][wh] handles
2. `SG::AuxElement::ConstAccessor<T>` and `SG::AuxElement::Decorator<T>`
3. `object->auxdata<T>` and `object->auxdecor<T>`

Read and write handles are preferred as they integrate well with multithreaded code and thus keep consistency with Atlas production code.
When required `ConstAccessor`s and `Decorator`s should be declared as early as possible: either as class member variables or as global `static const` in the source file.
The last option, `auxdata`/`auxdecor`, should never be used without a very good reason[^1].

You should take care to avoid `Decorator` to access data, e.g.
```c++
SG::AuxElement::Decorator<float> dec("something");
float something = dec(object);
```
this will return an undefined value if `something` does not exist! Instead you should use `ConstAccessor`.

#### Strings in `execute(...)`

We discourage any string access within the `execute` method. Not only is it slow, it also pushes configuration errors which should be caught in initialization back to the execution loop.
Instead you should parse the in initialization and use `enum` or other primitive types within the `execute`.

## Event Selection

Each analysis in EasyJet uses it own custom selector algorithm, e.g. [`XbbCalibSelectorAlg.cxx`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/src/XbbCalibSelectorAlg.cxx). The [`CP::SysFilterReporterCombiner`](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/Algorithms/SystematicsHandles/SystematicsHandles/SysFilterReporterCombiner.h) is the Athena object that controls if the event passes selection and is propagated to the output dumping algorithm. It has to be [set to false at the beginning of each event processing](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/src/XbbCalibSelectorAlg.cxx#L44) and [set to true](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/src/XbbCalibSelectorAlg.cxx#L98) if the event passes the required selections. The selector algorithm is scheduled in analyses specific python configuration  e.g. in [`XbbCalib_config.py`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/python/XbbCalib_config.py#L45-51).

The selection decision is stored as a decoration who's name is configured by the `eventDecisionOutputDecoration` property as done [here](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/python/XbbCalib_config.py#L48). This decoration should be dumped into the ntuple as done [here](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/python/XbbCalib_config.py#L108-109).

When running with systematics, an event that passes the selection for at least one of the requested systematic variations is stored. The decoration will be stored for all systematics ("\_%SYS%" included in the name) which allows to identify events that passed the selection for a given systematics (or the nominal).

[sh]: https://atlassoftwaredocs.web.cern.ch/AnalysisTools/ana_alg_sys_handle/
[rh]: https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/StoreGate/StoreGate/ReadDecorHandle.h
[wh]: https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/StoreGate/StoreGate/WriteDecorHandle.h

[^1]: At the time of writing we can't think of a good reason to use the `object->auxdata<T>` syntax anywhere.
