How to use the script:
=========================
# Setup easyjet
```
cd build/
source ../easyjet/setup.sh
source */setup.sh
cd ../run
```
# Setup Panda
```
voms-proxy-init -voms atlas
lsetup panda
```
# Submit grid jobs
```
cp ../easyjet/VBSVV4qAnalysis/scripts/BoostedBoosted/runOnGrid_DAOD_LLJ1.sh ./
. runOnGrid_DAOD_LLJ1.sh
```