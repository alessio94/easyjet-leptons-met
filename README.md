# Easyjet framework

An AthAnalysis framework, focusing on physics analysis combining runs 2 and 3.

See [`CONTRIBUTING.md`](./CONTRIBUTING.md) for instructions on modifying this code.

For questions, [join the EasyJet MM team](https://mattermost.web.cern.ch/signup_user_complete/?id=zqgsdao6sffjum15jm11y65gkh&md=link&sbr=su).
- FW and NTUP users: [easyjet FW](https://mattermost.web.cern.ch/easyjet/channels/easyjet-fw).
- FW developers and notifications: [easyjet dev](https://mattermost.web.cern.ch/easyjet/channels/easyjet-dev).
- Major HH decay channels have a corresponding mattermost channel, e.g. bbbb, bbtautau, etc.

Meetings will be biweekly at 5 pm on Tuesday, Central European Time, see [the indico easyjet group](https://indico.cern.ch/category/17096/) or [the dihiggs subgroup indico](https://indico.cern.ch/category/10816/), and announced on the e-group [atlas-phys-hdbs-dihiggs-hhframework@cern.ch](https://e-groups.cern.ch/e-groups/Egroup.do?egroupName=atlas-phys-hdbs-dihiggs-hhframework)

# Installation

*The instructions below with `setupATLAS` and `asetup` assume you are working on a CERN CentOS terminal, e.g. lxplus or a Singularity container on an institute cluster. Alternative instructions for using Docker images are given below.*

First, in a new working directory (we'll refer to this as `$WORKDIR` -- feel free to make an alias with `export WORKDIR=.`) clone the repository :
```
# Copy-paste this repo's URL, choosing your preferred authentication scheme, e.g. for lxplus or institute cluster
git clone --recursive ssh://git@gitlab.cern.ch:7999/easyjet/easyjet.git
```
Note the `--recursive` argument, which is needed to get the submodules in the package.

### Installation as developer

In case you want to make some developments to the software itself, we recommend you to:
1. Fork the branch using [this link](https://gitlab.cern.ch/easyjet/easyjet/-/forks/new). Please remember to put your username in the Project URL.
2. Clone the repository and set it as upstream:
``` git clone --recursive --origin upstream ssh://git@gitlab.cern.ch:7999/easyjet/easyjet.git ```
3. Go to the directory and set your forked branch as origin and fetch it:
``` 
cd easyjet
git remote add origin ssh://git@gitlab.cern.ch:7999/$(git config user.name)/easyjet.git
git fetch origin
```

For each development do:

4. Update the main branch (if you haven't done it since a while):
```
git checkout main
git pull --rebase upstream main
```
5. Now you can create a new branch:
``` git checkout -b YourNewFavoriteBranch ```
    - Alternatively you can checkout an existing branch on your fork by doing
    ``` git checkout YourExisitingBranch ```
    (don't use origin/YourExisitingBranch).
6. Once your changes are done you can create a Merge Request (make sure you merge onto the upstream main)
7. In case the MR highlights some conflicts and requests you to rebase your developments, please follow:
```
git pull --rebase upstream main
```
Then fix the potential conflicts and push to the current branch:
```
git push -f origin $(git symbolic-ref --short HEAD)
```
Please favour `rebase` over `merge` as it can badly affect the commit history. By default `git pull`, will use merge, unless you provide the explicit `--rebase` option.

And repeat 4 - 7 as many times as necessary up until the analysis is published.

### How to compile

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

To make some exploratory pileup and invariant mass plots, as well as getting a tree of variables, run with the example run config `easyjet/EasyjetHub/share/RunConfig.yaml`.:

```
easyjet-ntupler data.myinputfile.DAOD_PHYS.pool.root --run-config [path-to-runconfig] --evtMax 10 --out-file analysis-variables.root
```
>Your build also installs this one into the build area so you can also do --run-config ${EasyjetHub_DIR}/data/EasyjetHub/RunConfig.yaml
Feel free to increase the number of events, though beware of how many events may be in your file in case it takes a long time.
You should find a new ROOT files, `analysis-variables.root`.

To process Monte Carlo or PHYSLITE samples the command is exactly the same: configuration is automatically setup from the sample's metadata.

### Getting oriented with the configuration

For an overview of the core package structure and basic instructions for building an analysis custom executable, see [EasyjetHub/README.md](./EasyjetHub/README.md)

For an overview of the configuration syntax and how to extend this, see [EasyjetHub/steering/README.md](./EasyjetHub/steering/README.md).

For an explanation of the TTree output configuration, see [EasyjetHub/output/ttree/README.md](./EasyjetHub/output/ttree/README.md).

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

