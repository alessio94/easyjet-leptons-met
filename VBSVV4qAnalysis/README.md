Analysis Package for the $VBS VV(\rightarrow 4q) $ analysis
=========================

# Folder structure

- `bin/`: Executables
  - `VBSVV4q-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `*_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-common.yaml` : Common configuration between  3 channels 
  - `RunConfig-JJ.yaml`: Full leptonic specific configurations called by the executables (see below);
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `JJSelectorAlg`: Find if the event pass the baseline selection;
  - `BaselineVarsJJAlg`: Compute the baseline variables for the boosted-boosted analysis.

# How to generate Ntuples
After following [the main setup instructions](https://gitlab.cern.ch/easyjet/easyjet/-/blob/main/README.md?ref_type=heads#installation), your folder structure should look like this:
```
.
├── build
├── easyjet
│   ├── CITATION.cff
│   ├── CMakeLists.txt
│   ├── CONTRIBUTING.md
│   ├── EasyjetHub
│   ├── EasyjetTests
│   ├── KinematicFitTool
│   ├── LICENSE
│   ├── NOTICE
│   ├── README.md
│   ├── requirements.txt
│   ├── setup.cfg
│   ├── setup.sh
│   ├── TruthWeightTools
│   ├── VBFTagger
│   └── VBSVV4qAnalysis
└── run
```
You can generate Ntuples locally or by submitting a Grid job from the `run` folder.

## Local Run
Prepare the DAOD files you want to use. For example, you can download them with Rucio:
```
lsetup rucio
voms-proxy-init -voms atlas -valid 192:00
$ rucio list-dids mc20_13TeV:mc20_13TeV.5684*8.MGPy8EG_A14N*DAOD_LLJ1.e8488_s3797_r13145_p6967
+-------------------------------------------------------------------------------------------------------------------------+--------------+
| SCOPE:NAME                                                                                                              | [DID TYPE]   |
|-------------------------------------------------------------------------------------------------------------------------+--------------|
| mc20_13TeV:mc20_13TeV.568408.MGPy8EG_A14N30NLO_VBF_radion_WW_qqqq_kl35L3_m2000.deriv.DAOD_LLJ1.e8488_s3797_r13145_p6967 | CONTAINER    |
| mc20_13TeV:mc20_13TeV.568448.MGPy8EG_A14NNPDF30NLO_vbfHVT_Agv1_VzWW_qqqq_m2000.deriv.DAOD_LLJ1.e8488_s3797_r13145_p6967 | CONTAINER    |
| mc20_13TeV:mc20_13TeV.568468.MGPy8EG_A14NNPDF30NLO_VBF_RS_G_WW_qqqq_kt1_m2000.deriv.DAOD_LLJ1.e8488_s3797_r13145_p6967  | CONTAINER    |
+-------------------------------------------------------------------------------------------------------------------------+--------------+

$ rucio download --nrandom 1 mc20_13TeV:mc20_13TeV.568408.MGPy8EG_A14N30NLO_VBF_radion_WW_qqqq_kl35L3_m2000.deriv.DAOD_LLJ1.e8488_s3797_r13145_p6967
```
Once you have a DAOD file, run the ntupler:
``` 
sample='mc20_13TeV.568408.MGPy8EG_A14N30NLO_VBF_radion_WW_qqqq_kl35L3_m2000.deriv.DAOD_LLJ1.e8488_s3797_r13145_p6967/DAOD_LLJ1.46304716._000002.pool.root.1'

VBSVV4q-ntupler $sample --run-config ../../easyjet/VBSVV4qAnalysis/share/RunConfig-JJ.yaml --out-file output.root -e 5000
```

## Grid Run
From the `run` folder, copy the submission script:
```
cp ../easyjet/VBSVV4qAnalysis/runOnGrid.sh .
```
Open `runOnGrid.sh` and update the list of samples (`mc_list`) as needed.
```
lsetup panda
voms-proxy-init -voms atlas -valid 192:00
source runOnGrid.sh <campaignName>
```

