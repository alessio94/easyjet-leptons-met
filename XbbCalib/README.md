# Run easyjet for Xbb Calibration. (WIP)

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