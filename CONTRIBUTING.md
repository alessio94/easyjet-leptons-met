Contributing Guidelines
=======================

Anyone is welcome to contribute to this code. If you are working in a final state that isn't currently covered by easyjet some contributions are expected.

There is no need to ask for permission or contact the maintainers before making a merge request. The basic procedure is:

- Fork the repository on gitlab
- Clone the code locally and make any changes you need on an
  appropriately named branch
- Make a merge request

For larger changes or general questions about the code, please open an issue on gitlab.

## Coding style

Limited code formatting rules are enforced via `cppcheck` and `flake8`. We aim to follow `PEP8` python style conventions, except certain cases where it is more natural to follow ATLAS-like camel-case variable names.

If in doubt, follow the style of the surrounding code.

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
