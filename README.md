# Easyjet framework

An AthAnalysis framework, focusing on physics analysis combining runs 2 and 3.

See [`CONTRIBUTING.md`](./CONTRIBUTING.md) for instructions on modifying this code.

For questions, [join the EasyJet MM team](https://mattermost.web.cern.ch/signup_user_complete/?id=zqgsdao6sffjum15jm11y65gkh&md=link&sbr=su).
- FW and NTUP users: [easyjet FW](https://mattermost.web.cern.ch/easyjet/channels/easyjet-fw).
- FW developers, notifications and bug reports: [easyjet dev](https://mattermost.web.cern.ch/easyjet/channels/easyjet-dev).
- Major HH decay channels have a corresponding mattermost channel, e.g. bbbb, bbtautau, etc.

Meetings will be biweekly at 5 pm on Tuesday, Central European Time, see [the indico easyjet group](https://indico.cern.ch/category/17096/) or [the dihiggs subgroup indico](https://indico.cern.ch/category/10816/), and announced on the e-group [atlas-phys-hdbs-dihiggs-hhframework@cern.ch](https://e-groups.cern.ch/e-groups/Egroup.do?egroupName=atlas-phys-hdbs-dihiggs-hhframework)

# Installation

*The instructions below with `setupATLAS` and `asetup` assume you are working on a CERN CentOS terminal, e.g. lxplus or a Singularity container on an institute cluster. Alternative instructions for using Docker images are given below.*

First, in a new working directory (we'll refer to this as `$WORKDIR` -- feel free to make an alias with `export WORKDIR=.`) clone the repository :
```
setupATLAS
lsetup git
# Copy-paste this repo's URL, choosing your preferred authentication scheme, e.g. for lxplus or institute cluster
git clone --recursive --origin upstream ssh://git@gitlab.cern.ch:7999/easyjet/easyjet.git
```
Note the `--recursive` argument, which is needed to get the submodules in the package.

### Installation as developer

If not familiar with git already, pleas have a look at the [ATLAS git tutorial](https://atlassoftwaredocs.web.cern.ch/gittutorial/)

In case you want to make some developments to the software itself, we recommend you to:
1. Fork the branch using [this link](https://gitlab.cern.ch/easyjet/easyjet/-/forks/new). Please remember to put your username in the Project URL.
2. Clone the repository and set it as upstream:

``` 
setupATLAS
lsetup git
git clone --recursive --origin upstream ssh://git@gitlab.cern.ch:7999/easyjet/easyjet.git
```

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
```
git checkout -b YourNewFavoriteBranch
```
 - Alternatively you can checkout an existing branch on your fork by doing
```
git checkout YourExisitingBranch
```
(don't use origin/YourExisitingBranch).

5bis. Optional: in case you need to install Athena to test some changes in the Athena code

```
git atlas init-workdir ssh://git@gitlab.cern.ch:7999/$(git config user.name)/athena.git
cd athena
git atlas addpkg YourFavoritePackage1 YourFavoritePackage2
rm -rf Projects
```

You can then proceed with the standard compilation.

6. Once your local changes you should commit them and push them on your fork

```
git add UpdatedFile1
git add UpdatedFile2
...
git commit -m "Your description"
git push origin YourNewFavoriteBranch
```

This can be repeated several times if needed

7. When your development is complet, you can create a Merge Request (make sure you merge onto the upstream main). Please favour small and contained MRs, instead of big ones introducing several features.

8. In case the MR highlights some conflicts and requests you to rebase your developments, please follow:
```
git pull --rebase upstream main
```
Then fix the potential conflicts and push to the current branch:
```
git push -f origin $(git symbolic-ref --short HEAD)
```
Please favour `rebase` over `merge` as it can badly affect the commit history. By default `git pull`, will use merge, unless you provide the explicit `--rebase` option.

And repeat 4 - 8 as many times as necessary up until the analysis is published.

### How to compile and run

```
mkdir build
cd build
source ../easyjet/setup.sh
cmake ../easyjet/
make
source */setup.sh
cd ..
mkdir run
cd run
# Launch your favorite easyjet command
```

*If you are working in a container (described [below](#athanalysis-in-docker)), source the `/release_setup.sh` script, instead of the `setupATLAS; asetup` commands.*. The setup.sh will provide the current working AthAnalysis version


## Updating

You may occasionally have to resync the submodules after an
update. You can run

```
git submodule update --init --recursive
```

When changing branches with `git checkout` or `git switch`, also be sure to use the `--recurse-submodule` option to keep the submodules current.

# Running on files

The first step to run on some test files is:
### Getting test MC files
Most of test can be done, as for the pipelines, with $t\bar{t}$ events:
- <span style="color: #F2385A">PHYS</span>: ``` curl -s -o ttbar_PHYS_10evt.root https://gitlab.cern.ch/easyjet/hh4b-test-files/-/raw/p6026/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.DAOD_PHYS_10evts.e8514_s4162_r14622_p6026.pool.root```
- <span style="color: #4BD9D9;">PHYSLITE</span>: ``` curl -s -o ttbar_PHYSLITE_10evt.root https://gitlab.cern.ch/easyjet/hh4b-test-files/-/raw/p6026/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.DAOD_PHYSLITE_10evts.e6337_s3681_r13144_p6026.pool.root```

The next step is:
### Running some local jobs
To make some exploratory pileup and invariant mass plots, as well as getting a tree of variables, run with the example run config `easyjet/EasyjetHub/share/RunConfig.yaml`:

```
easyjet-ntupler ttbar_PHYS_10evt.root --run-config EasyjetHub/RunConfig.yaml --out-file analysis-variables.root
```
>Feel free to increase the number of events, though beware of how many events may be in your file in case it takes a long time.
You should find a new ROOT files, `analysis-variables.root`.

* To run analysis specific code run the executable of the analysis you want to run. For example to run the bbtautau analysis run `bbtt-ntupler <file> ...`. For more information on each individual analyis read the README in the analysis subfolder. 

To process Monte Carlo or PHYSLITE samples the command is exactly the same: configuration is automatically setup from the sample's metadata.

Then you can:
### Have a look at the output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Standard event info, e.g. `runNumber`, `eventNumber` ...
* Event passing a certain trigger: `trigPassed_X`;
* Truth information:
    * Higgs: `truth_H1_X` and `truth_H1_X`;
    * their decay products (implemented as a vector): `truth_children_fromH1_X` and `truth_children_fromH2_X`;

Other analysis specific variables are also described in each of the analysis READMEs.

Now it's time for:
### Getting oriented with the configuration

For an overview of the core package structure and basic instructions for building an analysis custom executable, see [EasyjetHub/README.md](./EasyjetHub/README.md)

For an overview of the configuration syntax and how to extend this, see [EasyjetHub/python/steering/README.md](./EasyjetHub/python/steering/README.md).

For an explanation of the TTree output configuration, see [EasyjetHub/python/output/ttree/README.md](./EasyjetHub/python/output/ttree/README.md).

## AthAnalysis in Docker

If you would rather work on a local computer, numbered `AthAnalysis` releases are available as Docker containers [in the Athena container registry][registry].
You will have to install [Docker](https://www.docker.com).

Preferably, do this in `$WORKDIR`.

```
docker pull gitlab-registry.cern.ch/atlas/athena/athanalysis:25.2.XXX
docker run -t -i -v $PWD:/workarea:delegated -v $HOME:$HOME:delegated atlas/athanalysis:25.2.XXX
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
source ../easyjet/setup.sh
source */setup.sh
```

*If you are working in a container, source the `/release_setup.sh` script, instead of the `setupATLAS; asetup --restore` commands.*

## Running on the grid

To run on the grid, you should either use a central tag

```
cd easyjet
git checkout [YourFavoriteTag]
cd ../run
```

or make sure your fork is setup, as a tag will be pushed on the fork

```
cd easyjet
git remote add origin ssh://git@gitlab.cern.ch:7999/$(git config user.name)/easyjet.git
cd ../run
```


To run on the grid there is a script available and is used as follows:

```
easyjet-gridsubmit --data-list myDataList.txt
```

For more options you can do:

```
easyjet-gridsubmit -h
```

Remember that in order to submit you need to do:

```
lsetup panda
```

Note: if you want to retrieve CutFlows saved as TEfficiency, you will need to set `mergeOutput` to `False` in [easyjet-gridsubmit](https://gitlab.cern.ch/easyjet/easyjet/-/blame/main/EasyjetHub/bin/easyjet-gridsubmit#L76), as the merge step doesn't support TEfficiency merging.

# Editing with VSCode support

You may already be using VSCode to edit easyjet -- if you aren't, this is a remarkably convenient tool with which to get helpful hints in your coding.
First, off you can get started from the [atlassoftwaredocs](https://atlassoftwaredocs.web.cern.ch/guides/vscode/) instructions.
Below are specific tips to get this set up more effectively, or specifically for easyjet.

- Unlike a sparse checkout of the `athena` repository, you do not need to add the `.vscode` directory -- it is present already.
- The `clangd` extension has been found to be more effective than the Microsoft `IntelliSense` extension. To use this, you need to:
  1. Ensure that `clang-tools-extra` is installed. If you are running on a local machine or container, do `sudo dnf install clang-tools-extra`. If you're on a cluster, see if your sysadmins are nice enough to do this for you.
  2. Install the `clangd` extension
  3. Disable the `C/C++ IntelliSense` extension
  4. Reload your session
- Take care that you *must* compile in a `build/` dir that is on the same level as the `easyjet` source dir (i.e. you navigate there from inside `easyjet` with the path `../build`). This is assumed by the `.vscode` configuration.
- The `.vscode` setup seems to work best when opening the `easyjet` top-level folder directly in VSCode. If you want to edit files in your run directory etc, you can simply `Add Folder to Workspace` in the options menu, which creates an `Untitled` workspace whose configuration you can then save (`Save Workspace As...`). 

Recommended extensions:
- `Python`, `Pylance`, `autopep8` -- for python coding support
- `clangd` -- for C++ coding support
- `GitLens` -- for git integration, diffs, viewing commits etc
- `CMake` -- maybe. It is possible to set up CMake for compilation and debugging but may not be trivial.
- Some markdown extension -- for writing READMEs like this
