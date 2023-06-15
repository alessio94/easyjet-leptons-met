# HH4bAnalysis

HH to 4b Analysis Framework initiated by Humboldt-Universität zu Berlin ERC Project.

# Installation

*The instructions below with `setupATLAS` and `asetup` assume you are working on a CERN CentOS terminal, e.g. lxplus or a Singularity container on an institute cluster. Alternative instructions for using Docker images are given below.*

First, in a new working directory (we'll refer to this as `$WORKDIR` -- feel free to make an alias with `export WORKDIR=.`) clone the repository (you are also welcome to fork a copy in case you might want to develop on top and contribute to improving it!):

```
# Copy-paste this repo's URL, choosing your preferred authentication scheme, e.g. for lxplus or institute cluster
git clone --recursive ssh://git@gitlab.cern.ch:7999/easyjet/hh4b-analysis.git
```
Note the `--recursive` argument, which is needed to get the submodules in the package.

Now, compile the package

```
mkdir build
cd build
source ../hh4b-analysis/setup.sh
cmake ../hh4b-analysis/
make
source */setup.sh
```

**NOTE: If you are using release 24.2.2 and want to submit grid jobs, you need to run `cmake` with the flag `-DATLAS_USE_CUSTOM_CPACK_INSTALL_SCRIPT=TRUE`. More details [change](https://gitlab.cern.ch/atlas/atlasexternals/-/merge_requests/1012).**

*If you are working in a container (described [below](#athanalysis-in-docker)), source the `/release_setup.sh` script, instead of the `setupATLAS; asetup` commands.*


## Updating

You may occasionally have to resync the submodules after an
update. You can run

```
git submodule update --init --recursive
```

When changing branches with `git checkout` or `git switch`, also be sure to use the `--recurse-submodule` option to keep the submodules current.

# Running on files

To make some exploratory pileup and invariant mass plots, as well as getting a tree of variables, run with the example runconfig `hh4b-analysis/EasyjetHub/share/RunConfig.yaml`.:

```
easyjet-ntupler data.myinputfile.DAOD_PHYS.pool.root --run-config [path-to-runconfig] --evtMax 10 --out-file analysis-variables.root
```
>Your build also installs this one into the build area so you can also do --run-config ${EasyjetHub_DIR}/data/EasyjetHub/RunConfig.yaml
Feel free to increase the number of events, though beware of how many events may be in your file in case it takes a long time.
You should find a new ROOT files, `analysis-variables.root`.

To process Monte Carlo or PHYSLITE samples the command is exactly the same: configuration is automatically setup from the sample's metadata.

## Extending the ntupler with analysis-specific algorithms

We expose functions for generating the sequences used in `easyjet-ntupler` via the `hub.py` module, such that a custom executable can be defined that extends the basic job with analysis-specific operations. An annotated example for this can be found in [`HH4bAnalysis/bin/hh4b-ntupler`](./HH4bAnalysis/bin/hh4b-ntupler). See [`EasyjetHub/python/README.md`](./EasyjetHub/python/README.md) and [`EasyjetHub/python/hub.py`](./EasyjetHub/python/hub.py) directly for guidance.

## AthAnalysis in Docker

If you would rather work on a local computer, numbered `AthAnalysis` releases are available as Docker containers [in the Athena container registry][registry].
You will have to install [Docker](https://www.docker.com).

Preferably, do this in `$WORKDIR`.

```
docker pull gitlab-registry.cern.ch/atlas/athena/athanalysis:24.2.XXX
docker run -t -i -v $PWD:/workarea:delegated -v $HOME:$HOME:delegated atlas/athanalysis:24.2.XXX
```

where you should be careful to change XXX in the lines above to the
minor release version you'd like to set up. You can find a working
version in the [`.gitlab-ci.yml`](.gitlab-ci.yml) file.

This will start up an interactive terminal inside the container, which has read/write access to the following paths:

* the present working directory (`$PWD`) under the path `/workarea`;
* your home directory (`$HOME`) with the same full path name (e.g. `/Users/myname`).
The terminal itself begins in an empty directory, `/workdir`. *The `delegated` suffix for these mounted volumes helps optimise the read/write access for better responsiveness.* Within this terminal, you can follow the instructions to source the `/release_setup.sh` script, in place of `setupATLAS; asetup`.

If you encounter any issues, some relevant instructions are available at <https://atlassoftwaredocs.web.cern.ch/athena/dev-setup/>. The `AthAnalysis` containers do not require `cvmfs` access, but you may need to experiment with the command line arguments when launching the container.

[registry]: https://gitlab.cern.ch/atlas/athena/container_registry/8440

## Restore the setup

If you come back to this in a new shell session, you can recover the setup with:

```
cd $WORKDIR/build
setupATLAS
asetup --restore
source */setup.sh
```

*If you are working in a container, source the `/release_setup.sh` script, instead of the `setupATLAS; asetup --restore` commands.*

## Running on the grid

To run on the grid there is a script available and is used as follows:

```
easyjet-gridsubmit --data-list myDataList.txt
```

For more options you can do:

```
easyjet-gridsubmit -h
```

# Development

Contributions to this code are welcome in the form of merge requests. The fast-forward merge policy is used, such that the git history is always linear, and most importantly ensures that the continuous integration of the master branch cannot be broken due to two independent MRs that interact without their own CI breaking. Merge requests require successful pipelines and an approval by one of the project developers.

Limited code formatting rules are enforced via `cppcheck` and `flake8`. We aim to follow `PEP8` python style conventions, except certain cases where it is more natural to follow ATLAS-like camel-case variable names.

### Package structure

The following `CMake` packages are defined in this repository:
- `EasyjetHub`: Core framework code providing analysis-independent algorithms and job steering
- `HH4bAnalysis`: Analysis-specific code for HH->4b information
  - E.g. preselections, jet pairing algorithms and decay path tracing specific to the 4b decay mode are implemented here
  - For historical reasons, `yybb` code is also currently included in this package, but will be separated into its own package at a later date
- `BJetCalibrationTool` (submodule): Tool and algs for applying b-jet four-momentum correction
- `H5Writer`: Tool and algs for writing HDF5 output

### General organization of python modules

Please follow the existing module organisation as laid out in [`EasyjetHub/python/README.md`](./EasyjetHub/python/README.md).
In case of ambiguity we are happy to discuss where additional modules or folders can be inserted.

*Note: To distinguish modules defined in `easyjet` from those in in `atlas/athena`, modules producing `ComponentAccumulator` configurations are named in snake case and end in `config.py` e.g. `EasyjetHub.steering.main_sequence_config.py`, rather than in camel case e.g. `AthenaConfiguration.MainSequencesConfig.py`.*

### Pre-commit hooks

*Currently not entirely functional*

You can use pre-commit hooks, that check and autoformats some style and formatting hooks in `.pre-commit-config.yaml` when you execute `git commit`. All you have to do is install pre-commit with pip. The following will do it for you.

```
pip install -r requirements.txt --user
pre-commit install
pre-commit run
```
