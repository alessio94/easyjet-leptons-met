# Run easyjet for Xbb Calibration. (WIP)

Analysis Package for the $Xbb$ Calibration
=========================

# Folder structure

- `bin/`: Executables
  - `xbbcalib-ntupler`
- `datasets/`
    - `ttbar`: Lists of data and mc samples to process
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `ttCalib_config`
  - `ZbbjCalib_config`
  - `ZbbyCalib_config`
  - `ZllCalib_config`
- `scripts`
  - `grid`: Scripts for submitting grid jobs.
  - `ttCalibPostProcess`: script for computing sumofweights
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-ttCalib`: configurations called by the executables (see below);	
  - `trigger`: list of the triggers to use per year.
  - `XSectionsData`: XSections for computing weights.
- `src/`: C++ code
  - `ttCalibSelectorAlg`: Find if the event pass the baseline selection for $t \bar{t}$ calibration;
  - `BaselineVarsttCalibAlg`: Compute the baseline variables for the $t \bar{t}$ calibration.


# Zbby calibration (WIP)

To run the Xbb calibration ntupler a sparse-checkout of `EasyjetHub`, `EasyjetTests`, `XbbCalib`is needed. After builiding easyjet, a test job can be run. First download a file locally, the example uses the Zbb+y signal sample.

```
lsetup rucio
rucio get --nrandom 1 mc20_13TeV.700353.Sh_2211_pTZ100_Zbbgamma.deriv.DAOD_PHYS.e8312_s3681_r13145_p6490
```
 
Run test 

```
xbbcalib-ntupler run/mc20_13TeV.700353.Sh_2211_pTZ100_Zbbgamma.deriv.DAOD_PHYS.e8312_s3681_r13145_p6490/DAOD_PHYS.<sample downloaded>.pool.root.1 --run-config ../easyjet/XbbCalib/share/RunConfig-ZbbyCalib.yaml --out-file output.root -e 10
```

Then to submit to the grid (Zbb+y specific example when running the production including systematics)

```
lsetup rucio
lsetup panda
```

The systematic production is split into two set, `syst_Set1.yaml` (MUON, MET, JER systematics NOT included) and `syst_Set2.yaml` (MET, JER systematics included). To sumit to the grid from the run directory:

```
python3 ../easyjet/XbbCalib/scripts/XbbCalib-submitGrid.py -c ../easyjet/XbbCalib/share/RunConfig-ZbbyCalib-syst_Set1.yaml --tag <NameAndDateOfProdcution>_syst1  -s "all" --nGBPerJob 2 --memory 4000 
python3 ../easyjet/XbbCalib/scripts/XbbCalib-submitGrid.py -c ../easyjet/XbbCalib/share/RunConfig-ZbbyCalib-syst_Set2.yaml --tag <NameAndDateOfProdcution>_syst2  -s "all" --nGBPerJob 2 --memory 4000 
```

To sumbit data:

```
easyjet-gridsubmit --data-list /direct/usatlas+u/ivelisce/VBSHiggs/easyjet/XbbCalib/datasets/data/data_Run2_p6490.txt --run-config ../easyjet/XbbCalib/share/RunConfig-ZbbyCalib-syst_Set2.yaml --exec xbbcalib-ntupler --campaign <NameAndDateOfProdcution> --mergeOutput --noEmail --nGBperJob 2 --memory 4000 --noTag
```

Nominal production can be run as:

```
python3 ../easyjet/XbbCalib/scripts/XbbCalib-submitGrid.py -c ../easyjet/XbbCalib/share/RunConfig-ZbbyCalib.yaml --tag <NameAndDateOfProdcution> -s "all" --nGBPerJob 2 --memory 4000 
```


# ttbar calibration

## Installation

```
setupATLAS
lsetup git
git lfs install
git clone --recursive --no-checkout --origin upstream ssh://git@gitlab.cern.ch:7999/easyjet/easyjet.git
cd easyjet
git sparse-checkout init --cone
git sparse-checkout set EasyjetHub EasyjetTests EasyjetPlus XbbCalib
git checkout upstream/main
git submodule update --init --recursive
```

## Running Locally

From the run directory

```xbb-ntupler ttbar_PHYS_10evt.root --run-config XbbCalib/RunConfig-ttCalib.yaml --out-file calibration-vars.root```

## Running on Grid

Configure ```RunOnGrid_ttCalib.sh``` to match the corresponding datasets in ```datasets/ttbar```, then run 
```source ../easyjet/XbbCalib/scripts/grid/RunOnGrid_ttCalib.sh```

Once your grid jobs have ran succesfully and you retrived the outputs, you will need to hadd the outputs before reweighting. Theres is a problem with hadding files which contain empty trees; For this, there is a custom mergeRootFiles.C macro. <https://gitlab.cern.ch/dallen/gn2x_calibration/-/blob/master/mergeRootFiles.C?ref_type=heads>

## Reweighting MC

Using the ttCalibPostProcess.py script will add a 'weight' branch to your analyis tree.

```../easyjet/XbbCalib/scripts/ttCalibPostProcess.py --xSectionsConfig ../easyjet/XbbCalib/share/XSectionsData.yaml --mergeMyFiles --mergeToOutput --inFile calibration-vars.root --ouFile reweighted.root```



