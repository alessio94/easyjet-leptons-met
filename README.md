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
setupATLAS
asetup AthAnalysis,22.2.108
cmake ../hh4b-analysis/
make
source */setup.sh
```

*If you are working in a container (described [below](#athanalysis-in-docker)), source the `/release_setup.sh` script, instead of the `setupATLAS; asetup` commands.*


## Updating

You may occasionally have to resync the submodules after an
update. You can run

```
git submodule update --init --recursive
```

When changing branches with `git checkout` or `git switch`, also be sure to use the `--recurse-submodule` option to keep the submodules current.

# Running on files

To make some exploratory pileup and invariant mass plots, as well as getting a tree of variables, run with the example runconfig hh4b-analysis/HH4bAnalysis/share/RunConfig.yaml.:

```
VariableDumperConfig.py --runConfig [path-to-runconfig] --filesInput data.myinputfile.DAOD_PHYS.pool.root --evtMax 10
```
>Your build also installs this one into the build area so you can also do --runConfig ${HH4bAnalysis_DIR}/data/HH4bAnalysis/RunConfig.yaml
Feel free to increase the number of events, though beware of how many events may be in your file in case it takes a long time.
You should find a new ROOT files, `analysis-variables.root`.

To process Monte Carlo samples the configuration is automatically setup from the sample's metadata:

```
VariableDumperConfig.py --runConfig [path-to-runconfig] --filesInput mc.myinputfile.pool.root --evtMax 10
```

To process DAOD_PHYSLITE the configuration is automatic as well, the command looks the same:

```
VariableDumperConfig.py --runConfig [path-to-runconfig] --filesInput mc.myinputfile.DAOD_PHYSLITE.pool.root --evtMax 10
```

## AthAnalysis in Docker

If you would rather work on a local computer, numbered `AthAnalysis` releases are available as Docker containers [in the Athena container registry][registry].
You will have to install [Docker](https://www.docker.com).

Preferably, do this in `$WORKDIR`.

```
docker pull gitlab-registry.cern.ch/atlas/athena/athanalysis:22.2.108
docker run -t -i -v $PWD:/workarea:delegated -v $HOME:$HOME:delegated atlas/athanalysis:22.2.100
```

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
grid_submit.py --data-list myDataList.txt
```

For more options you can do:

```
grid_submit.py -h
```

## Development

You can use pre-commit hooks, that check and autoformats some style and formatting hooks in `.pre-commit-config.yaml` when you execute `git commit`. All you have to do is install pre-commit with pip. The following will do it for you.

```
pip install -r requirements.txt --user
pre-commit install
pre-commit run
```
