# Analysis Package for bbyy

This analysis package is currently used for investigating the potential use of the `DAOD_PHYS` (PHYS) or `DAOD_PHYSLITE` (PHYSLITE) data formats for Run 3 HH to yybb [HDBS workshop talk](https://indico.cern.ch/event/1132691/sessions/436749/attachments/2503571/4301659/PHYSLITE_HDBSWorkshop_Sep2022.pdf).

For any questions, comments, or feedback on the package feel free to contact:

Abraham Tishelman-Charny \
Email: `abraham.tishelman.charny@cern.ch` \
Mattermost: `@atishelm` \
Skype: `abe_t-c`

Participation and development on this package from: Giulia Di Gregorio, Sayuka Kita, Spyros Merianos, Lorenzo Santi

## Set up easyjet

Suggest to begin by creating a new working directory where you will keep `easyjet`, and your `build` and `run` directories:

```
mkdir easyjet
cd easyjet
```

Then clone `easyjet` following the instructions in the main README:

Via ssh protocol:
```
git clone --recursive ssh://git@gitlab.cern.ch:7999/easyjet/easyjet.git
```

Via https protocol:

```
git clone --recursive https://gitlab.cern.ch/easyjet/easyjet.git
```

Then create your build directory and build:

```
mkdir build \
cd build \
source ../easyjet/setup.sh \
cmake ../easyjet/ \
make \
source */setup.sh 
```

(Optional): Then create a run directory where you will run from, and navigate there:

```
cd ..
mkdir run
cd run
```

## Producing bbyy ntuples

bbyy ntuples are produced by running the `bbyy-ntupler` executable. Example commands for processing 100 events of a `DAOD_PHYS` or `DAOD_PHYSLITE` file can be found below:

PHYS:

```
bbyy-ntupler /eos/atlas/atlascerngroupdisk/phys-hdbs/diHiggs/Run3/yybb/mc20_13TeV.600021.PhPy8EG_PDF4LHC15_HHbbyy_cHHH01d0.deriv.DAOD_PHYS.e8222_s3681_r13167_p5631/DAOD_PHYS.33464338._000003.pool.root.1 \
  --run-config ../easyjet/bbyyAnalysis/share/RunConfig-PHYS-yybb.yaml --evtMax 100 --out-file yybb_PHYS_ntuple.root
```

PHYSLITE:

```
bbyy-ntupler /eos/atlas/atlascerngroupdisk/phys-hdbs/diHiggs/Run3/yybb/mc20_13TeV.600021.PhPy8EG_PDF4LHC15_HHbbyy_cHHH01d0.deriv.DAOD_PHYSLITE.e8222_s3681_r13167_p5631/DAOD_PHYSLITE.33440027._000003.pool.root.1 \
  --run-config ../easyjet/bbyyAnalysis/share/RunConfig-PHYSLITE-yybb.yaml --evtMax 100 \
  --out-file yybb_PHYSLITE_ntuple.root
```

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` containing:

* Standard event info, e.g. `runNumber`, `eventNumber` etc.
* Truth information on the Higgs
* All photon kinematics: `ph_NOSYS_pt`, `ph_NOSYS_eta`, `ph_NOSYS_phi`
* Reco jet kinematics: `recojet_antikt4PFlow_NOSYS_X`
* Truth jet information: `truthjet_...`
* Standard set of `yybb` variables, including:
  * `N_LOOSE_PHOTONS`
  * `TWO_TIGHTID_PHOTONS`
  * `myy`
  * `Leading/Subleading_Photon/Jet_<kinematic>`

Note that running with the `bbyy-ntupler` executable will add the standard `bbyy` variables above, defined in `bbyyAnalysis/src/BaselineVarsyybbAlg.cxx` and `bbyyAnalysis/python/yybb_config.py`. 

One can change the outputs saved in the TTree by adjusting the base bbyy configuration file `bbyyAnalysis/share/yybb-base-config.yaml`.

## Development

Currently, the development branch for `yybb` additions is here:

https://gitlab.cern.ch/atishelm/easyjet/-/tree/yybb_dev?ref_type=heads

This is to allow for review among `yybb` analyzers before updating `yybb` developments, before making merge requests to [the main easyjet repository](https://gitlab.cern.ch/easyjet/easyjet).

If you would like to update the `yybb` analysis package, start from [yybb dev](https://gitlab.cern.ch/atishelm/easyjet/-/tree/yybb_dev?ref_type=heads), commit your changes locally, and push to a new branch in [Forked repo](https://gitlab.cern.ch/atishelm/easyjet) with a meaningful name e.g. `yybb_cutflow`, `yybb_systematics`, etc, and assign `@atishelm` as a reviewer. 

## Package structure

The main file which defines the `yybb` part of the easyjet sequence is `easyjet/bbyyAnalysis/python/yybb_config.py`. It is here that the [`PhotonSelectorAlg`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/master/EasyjetHub/src/PhotonSelectorAlg.cxx) and [`JetSelectorAlg`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/master/EasyjetHub/src/JetSelectorAlg.cxx) are called from `EasyjetHub` (because they are not `yybb` specific packages). These are run with `yybb` parameters and the output collections are saved.

These are then input to [`BaselineVarsyybbAlg`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/master/bbyyAnalysis/src/BaselineVarsyybbAlg.cxx), where these input collections are used to compute final variables saved in the TTree.