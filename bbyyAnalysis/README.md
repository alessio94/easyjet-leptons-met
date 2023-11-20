# Analysis Package for bbyy

This analysis package is currently used for investigating the potential use of the `DAOD_PHYS` (PHYS) or `DAOD_PHYSLITE` (PHYSLITE) data formats for Run-3 HH to bbyy [HDBS workshop talk](https://indico.cern.ch/event/1132691/sessions/436749/attachments/2503571/4301659/PHYSLITE_HDBSWorkshop_Sep2022.pdf).

For any questions, comments, or feedback on the package feel free to contact:

Abraham Tishelman-Charny \
Email: `abraham.tishelman.charny@cern.ch` \
Mattermost: `@atishelm` \
Skype: `abe_t-c`

Participation and development on this package from: Giulia Di Gregorio, Sayuka Kita, Spyros Merianos, Lorenzo Santi

## Producing bbyy ntuples

bbyy ntuples are produced by running the `bbyy-ntupler` executable. Example commands for processing 100 events of a `DAOD_PHYS` or `DAOD_PHYSLITE` file can be found below:

PHYS:

```
bbyy-ntupler /eos/atlas/atlascerngroupdisk/phys-hdbs/diHiggs/Run3/yybb/mc20_13TeV.600021.PhPy8EG_PDF4LHC15_HHbbyy_cHHH01d0.deriv.DAOD_PHYS.e8222_s3681_r13167_p5855/DAOD_PHYS.35180056._000001.pool.root.1 \
  --run-config ../easyjet/bbyyAnalysis/share/RunConfig-PHYS-bbyy.yaml --evtMax 100 --out-file bbyy_PHYS_ntuple.root
```

PHYSLITE:

```
bbyy-ntupler /eos/atlas/atlascerngroupdisk/phys-hdbs/diHiggs/Run3/yybb/mc20_13TeV.600021.PhPy8EG_PDF4LHC15_HHbbyy_cHHH01d0.deriv.DAOD_PHYSLITE.e8222_s3681_r13167_p5855/DAOD_PHYSLITE.35180056._000001.pool.root.1 \
  --run-config ../easyjet/bbyyAnalysis/share/RunConfig-PHYSLITE-bbyy.yaml --evtMax 100 \
  --out-file bbyy_PHYSLITE_ntuple.root
```

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` containing:

* Standard event info, e.g. `runNumber`, `eventNumber` etc.
* Truth information on the Higgs: `truth_H1_X` and `truth_H2_X`
* All photon kinematics: `ph_NOSYS_X`
* Reco jet kinematics: `recojet_antikt4PFlow_NOSYS_X`
* Truth jet information: `truthjet_antikt4_X`
* Standard set of `bbyy` variables, including:
  * reco photons: `NOSYS_Leading_Photon_X` and `NOSYS_Subleading_Photon_X` + the related variables `NOSYS_myy` etc.
  * reco jets: `NOSYS_Jet_X_b1` and `NOSYS_Jet_X_b2` + the related variables `NOSYS_mbb` etc.
  * HH variables: `NOSYS_mbbyy` etc.
  * gloabl variables: `NOSYS_nJets`, `NOSYS_nPhotons`etc.
  * cuts to pass: `NOSYS_PASS_TRIGGER`, `NOSYS_TWO_TIGHTID_ISO_PHOTONS`

Note that running with the `bbyy-ntupler` executable will add the standard `bbyy` variables above, defined in `bbyyAnalysis/src/BaselineVarsbbyyAlg.cxx` and `bbyyAnalysis/python/bbyy_config.py`. 

One can change the outputs saved in the TTree by adjusting the base bbyy configuration file `bbyyAnalysis/share/bbyy-base-config.yaml`, which also defines the identification and isolation WP of the different objects.

## Package structure

The main file which defines the `bbyy` part of the easyjet sequence is `easyjet/bbyyAnalysis/python/bbyy_config.py`. It is here that the [`PhotonSelectorAlg`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/main/EasyjetHub/src/PhotonSelectorAlg.cxx) and [`JetSelectorAlg`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/main/EasyjetHub/src/JetSelectorAlg.cxx) are called from `EasyjetHub` (because they are not `bbyy` specific packages). These are run with `bbyy` parameters and the output collections are saved.

These are then input to [`BaselineVarsbbyyAlg`](https://gitlab.cern.ch/easyjet/easyjet/-/blob/main/bbyyAnalysis/src/BaselineVarsbbyyAlg.cxx), where these input collections are used to compute final variables saved in the TTree.