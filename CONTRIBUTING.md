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

The merge requests will run some validation pipelines. The resulting plots are stored in https://easyjet-validation.web.cern.ch/.

### Testing changes

Please use the `easyjet-test` utility to run the CI tests before committing changes and especially for debugging pipeline errors.
```sh
> easyjet-test
usage: easyjet-test [-h] [-l <level>] [-L <log-file>] [-d <dir>] <mode>
```
You can get detailed help with `easyjet-test -h`, and should at least run the modes (analysis + dataset) that you have touched with your changes.
For a comprehensive test, run `easyjet-test all-exit-early`, but this may take time and some computing power.

#### Input file access

- If you have a local EOS mount, the input file access does not need any extra input.
- You can also access the input files with `XRootD` if you have a valid grid certificate.
  - By default, the files will be downloaded with `xrdcp` but note that you should specify a target directory to avoid redownloading to `/tmp` on each run.
Future runs on the same file will not repeat the download.
  - Alternatively, you can run with `EASYJET_STREAM_XROOTD=1 easyjet-test [mode]` to read the input files directly over the remote connection.
- If neither of these is available, or the input file is not on EOS, then the script will try to retrieve a small input file with `wget`.

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

This includes the use of `std::unordered_map<std::string, T>` for keyed access to anything (e.g. `Read/WriteDecorHandles`) in the event loop.
In almost all cases it suffices to use parallel lists of string and `Handle(Key)`.
Specifically, for lists of `DecorHandleKey`, use [`SG::Read/WriteDecorHandleKeyArray`](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/StoreGate/StoreGate/WriteDecorHandleKey.h) which makes it easier to initialise:
```c++
/// In the header
// For direct configuration
SG::WriteDecorHandleKeyArray<xAOD::SomethingContainer, float> m_decorKeysA{this, "SomethingKeys", {}, "List of decorations on something"};
// For construction using a loop with reference to other properties
Gaudi::Property<int> m_number_of_objects = {this, "NSomething", 0, "The number of things to decorate"}; // or generate from any other source
// Strictly speaking initialising as follows makes the Athena scheduler blind to this, so use
// the syntax above if the existence of this decoration could be relevant for
// deciding if downstream algorithms run.
SG::WriteDecorHandleKeyArray<xAOD::SomethingContainer, float> m_decorKeysB{{}, this};

/// In the initialize() method:
// No need to iterate over all keys
ATH_CHECK(m_decorKeysA.initialize());
// Construct from loop
for(int i, i<m_number_of_objects, ++i) {
  m_decorKeysB.emplace_back(m_inputContainer, "thing_at_" + std::to_string(i));
}
 TH_CHECK(m_decorKeysB.initialize());

/// In the execute() method:
// Same for both cases, you likely want to create a vector of handles at event scope.
// The decorations will be locked as soon as a handle goes out of scope.
// In AthReentrantAlgorithm the EventContext is available directly as `ctx`.
// In AthAlgorithm it can be retrieved with `getContext()`.
std::vector<SG::WriteDecorHandle<xAOD::SomethingContainer, float> > decorHandlesA = m_decorKeysA.makeHandles(ctx);
// It is also acceptable to create individual handles in a loop
// and this may be more convenient in non-reentrant algorithms,
// but be careful not to do this inside a loop over a container.
for(int i, i<m_number_of_objects, ++i) {
  SG::WriteDecorHandle<xAOD::SomethingContainer, float> > handle(m_decorKeysB);
}
```

This syntax is similar for `CPP:SysWriteDecorHandleArray`, but initialisation is from a list of strings and there is no key/handle separation:
```c++
/// In the header
// For construction using a loop with reference to other properties
Gaudi::Property<int> m_number_of_objects = {this, "NSomething", 0, "The number of things to decorate"}; // or generate from any other source
CP::SysWriteDecorHandleArray<float> m_decorHandles{{}, this};

/// In the initialize() method:
// Construct from loop
std::vector<std::string> decor_keys;
for(int i, i<m_number_of_objects, ++i) {
  decor_keys.emplace_back("thing_at_" + std::to_string(i));
}
m_decorHandles = CP::SysWriteDecorHandleArray<float>(list_of_keys, this);
ATH_CHECK(m_decorHandles.initialize());

/// In the execute() method:
for(int i, i<m_number_of_objects, ++i) {
  m_decorHandles.at(i).set(*collection, value, sys);
}
```


## Event Selection

Each analysis in EasyJet uses it own custom selector algorithm, e.g. [`XbbCalibSelectorAlg.cxx`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/src/XbbCalibSelectorAlg.cxx). The [`CP::SysFilterReporterCombiner`](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/Algorithms/SystematicsHandles/SystematicsHandles/SysFilterReporterCombiner.h) is the Athena object that controls if the event passes selection and is propagated to the output dumping algorithm. It has to be [set to false at the beginning of each event processing](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/src/XbbCalibSelectorAlg.cxx#L44) and [set to true](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/src/XbbCalibSelectorAlg.cxx#L98) if the event passes the required selections. The selector algorithm is scheduled in analyses specific python configuration  e.g. in [`XbbCalib_config.py`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/python/XbbCalib_config.py#L45-51).

The selection decision is stored as a decoration who's name is configured by the `eventDecisionOutputDecoration` property as done [here](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/python/XbbCalib_config.py#L48). This decoration should be dumped into the ntuple as done [here](https://gitlab.cern.ch/easyjet/easyjet/-/blob/d10059f7606d20ae018509fc233d6df677d14b3e/XbbCalib/python/XbbCalib_config.py#L108-109).

When running with systematics, an event that passes the selection for at least one of the requested systematic variations is stored. The decoration will be stored for all systematics ("\_%SYS%" included in the name) which allows to identify events that passed the selection for a given systematics (or the nominal).

[sh]: https://atlassoftwaredocs.web.cern.ch/AnalysisTools/ana_alg_sys_handle/
[rh]: https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/StoreGate/StoreGate/ReadDecorHandle.h
[wh]: https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/StoreGate/StoreGate/WriteDecorHandle.h

[^1]: At the time of writing we can't think of a good reason to use the `object->auxdata<T>` syntax anywhere.
