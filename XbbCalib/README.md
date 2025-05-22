# Run easyjet for Xbb Calibration. (WIP)

To run a test job

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

python3 ../easyjet/XbbCalib/scripts/XbbCalib-submitGrid.py -c ../easyjet/XbbCalib/share/RunConfig-ZbbyCalib-syst.yaml --tag <XbbCalib-date-version> -s "all" --nGBPerJob 2 --memory 4000 
```
