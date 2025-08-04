# Monojet Sample Production with EasyJet

This package facilitates the monojet sample production using the [easyjet](https://gitlab.cern.ch/easyjet/easyjet) framework. The structure is adapted from the [monobcAnalysis](https://gitlab.cern.ch/atlas-phys/exot/jdm/ANA-EXOT-2023-25/monobcAnalysis/-/tree/master/monobcAnalysis?ref_type=heads).

## Setup on lxplus

### EasyJet Installation
First, in a new working directory (`$WORKDIR`) setup the ATLAS environment and `git`:

```
setupATLAS
lsetup git
git lfs install #IMPORTANT: needed to pull LFS files; only needs to be setup once
```

Clone the `easyjet` repository with a sparse checkout:

```
git clone --recursive --no-checkout --origin upstream ssh://git@gitlab.cern.ch:7999/easyjet/easyjet.git
cd easyjet
git sparse-checkout init --cone
git sparse-checkout set EasyjetHub EasyjetTests YourFavouriteAnalysis
git submodule update --init --recursive
```

> Notes:
> - The `--recursive` argument, needed to get the submodules in the package
> - The `--no-checkout` argument, to define the list of packages you want to checkout and avoiding the download of all analysis packages
> - Just replace `YourFavouriteAnalysis` with the name of the analysis package you want to use. You can specify several of them.
> - You can omit the `--no-checkout` option and the subsequent lines if you want to checkout the full repository.

### Monojet Installation
Inside easyjet, git clone this package into a folder entitled monojetAnalysis. As an example:
```
git clone https://gitlab.cern.ch/atlas-phys/exot/jdm/ana-exot-2023-19/ntuples-production.git monojetAnalysis
```


## How to compile

In your `$WORKDIR` you can do

```
mkdir build run
cd build
source ../easyjet/setup.sh
cmake ../easyjet/
make
source */setup.sh
cd ..
cd run
# Launch your favorite easyjet command
```


After the installing and compiling, your folders should look like this:

`$WORKDIR/`
- `easyjet/`:
    - `EasyjetXX/`:       Easyjet code
    - `monojetAnalysis/`: Our ntuple production repository
    - `XXAnalysis/`:      Other analysis if needed
- `build/`:  Where you compile
- `run/`:    Where you launch commands

## Updating

You may occasionally have to resync the submodules after an
update. You can run

```
git submodule update --init --recursive
```


## When you come back
```
cd $WORKDIR/build
source ../easyjet/setup.sh
source */setup.sh
```

## Running

### Running local jobs
Configuration files for each region are located in the share/ directory. To execute the code, use the command below:
```
monojet-ntupler input.root --run-config monojetAnalysis/configfile.yaml --out-file output.root
```

As an example you can do

```
monojet-ntupler /eos/atlas/atlascerngroupdisk/phys-exotics/jdm/ANA-EXOT-2023-19_allhadMET/easyjet-useful/DAOD_PHYS.40055556._000001.pool.root.1 --run-config monojetAnalysis/RunConfig-Monojet.yaml --out-file output.root
```

### Running on the grid

For grid submission, define or locate input lists in datasets/PHYS and update configurations as needed in scripts/grid/RunOnGrid_MC20e_skim.sh. To submit a grid job, execute:
```
cd run
scp ../easyjet/monojetAnalysis/scripts/grid/RunOnGrid_MC20e_skim.sh .
source RunOnGrid_MC20e_skim.sh
```

Remember that in order to submit you need to do:
```
lsetup rucio panda
voms-proxy-init -voms atlas
```
