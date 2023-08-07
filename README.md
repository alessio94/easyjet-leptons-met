# Easyjet framework

An AthAnalysis framework, focusing on physics analysis combining runs 2 and 3.

See [`CONTRIBUTING.md`](./CONTRIBUTING.md) for instructions on modifying this code.

For questions, join the SH4b analysis MM team [here](https://mattermost.web.cern.ch/signup_user_complete/?id=pra49cyqh3du9fymb7btog8exy&md=link&sbr=su).
- FW and NTUP users: [easyjet FW](https://mattermost.web.cern.ch/sh4b/channels/easyjet-fw).
- FW developers and notifications: [easyjet dev](https://mattermost.web.cern.ch/sh4b/channels/easyjet-dev).

Meetings will be held on demand, generally 1 pm Friday, Central European Time, see [the indico easyjet group](https://indico.cern.ch/category/17096/).

# Installation

*The instructions below with `setupATLAS` and `asetup` assume you are working on a CERN CentOS terminal, e.g. lxplus or a Singularity container on an institute cluster. Alternative instructions for using Docker images are given below.*

First, in a new working directory (we'll refer to this as `$WORKDIR` -- feel free to make an alias with `export WORKDIR=.`) clone the repository (you are also welcome to fork a copy in case you might want to develop on top and contribute to improving it!):

```
# Copy-paste this repo's URL, choosing your preferred authentication scheme, e.g. for lxplus or institute cluster
git clone --recursive ssh://git@gitlab.cern.ch:7999/easyjet/easyjet.git
```
Note the `--recursive` argument, which is needed to get the submodules in the package.

Now, compile the package

```
mkdir build
cd build
source ../easyjet/setup.sh
cmake ../easyjet/
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

To make some exploratory pileup and invariant mass plots, as well as getting a tree of variables, run with the example runconfig `easyjet/EasyjetHub/share/RunConfig.yaml`.:

```
easyjet-ntupler data.myinputfile.DAOD_PHYS.pool.root --run-config [path-to-runconfig] --evtMax 10 --out-file analysis-variables.root
```
>Your build also installs this one into the build area so you can also do --run-config ${EasyjetHub_DIR}/data/EasyjetHub/RunConfig.yaml
Feel free to increase the number of events, though beware of how many events may be in your file in case it takes a long time.
You should find a new ROOT files, `analysis-variables.root`.

To process Monte Carlo or PHYSLITE samples the command is exactly the same: configuration is automatically setup from the sample's metadata.

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
source ../hh4b-analysis/setup.sh
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

