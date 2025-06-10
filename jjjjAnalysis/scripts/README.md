How to use the script:
=========================
# Setup easyjet
```
cd build/
source ../easyjet/setup.sh
source x86_64*/setup.sh
cd ../run
```
# Setup Panda
```
voms-proxy-init -voms atlas
lsetup panda
```
# Submit grid jobs
```
cp ../easyjet/jjjjAnalysis/scripts/runOnGrid.sh ./
. runOnGrid.sh
```
