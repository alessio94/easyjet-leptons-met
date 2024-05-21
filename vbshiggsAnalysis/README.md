Analysis Package for the $VBS VVH(\rightarrow b\bar{b}) $ analyis
=========================

# Folder structure

- `bin/`: Executables
  - `vbshiggs-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `*_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-common.yaml` : Common configuration between  3 channels 
  - `RunConfig-fullLep.yaml`: Full leptonic specific configurations called by the executables (see below);
  - `RunConfig-semiLep.yaml`: Semi leptonic specific  configurations called by the executables (see below);
  - `RunConfig-fullHad.yaml`: Full hadronic specific configurations called by the executables (see below);
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `FullLepSelectorAlg`: Find if the event pass the baseline VBS Fully Leptonic selection;
  - `SemiLepSelectorAlg`: Find if the event pass the baseline VBS Semi-Leptonic selection;
  - `FullHadSelectorAlg`: Find if the event pass the baseline VBS Fully Hadrnic selection;
  - `BaselineVarsFullLepAlg`: Compute the baseline variables for the full leptonic analysis.
  - `BaselineVarsSemiLepAlg`: Compute the baseline variables for the Semi leptonic analysis.
  - `BaselineVarsFullHadAlg`: Compute the baseline variables for the full hadronic analysis.

# How to Run (local)
- Full Lepton channel
``` 
vbshiggs-ntupler /eos/atlas/atlascerngroupdisk/phys-hdbs/dbl/VBS_Higgs/MC/DAOD_PHYS/LepChan/mc20a/mc20_13TeV.525362.MGPy8EG_ssWWhjj_llvvbbjj_EW8_LO_klp1.deriv.DAOD_PHYS.e8545_s3681_r13167_r13146_p6026/DAOD_PHYS.37982158._000001.pool.root.1 --run-config ../easyjet/vbshiggsAnalysis/share/RunConfig-fullLep.yaml --out-file output.root
```

- Semi-leptonic channel
``` 
vbshiggs-ntupler /eos/atlas/atlascerngroupdisk/phys-hdbs/dbl/VBS_Higgs/MC/DAOD_PHYS/LepChan/mc20a/mc20_13TeV.525362.MGPy8EG_ssWWhjj_llvvbbjj_EW8_LO_klp1.deriv.DAOD_PHYS.e8545_s3681_r13167_r13146_p6026/DAOD_PHYS.37982158._000001.pool.root.1 --run-config ../easyjet/vbshiggsAnalysis/share/RunConfig-SemiLep.yaml --out-file output.root
```

- Full Hadronic channel
``` 
vbshiggs-ntupler /eos/atlas/atlascerngroupdisk/phys-hdbs/dbl/VBS_Higgs/MC/DAOD_PHYS/LepChan/LepChan/mc20a/mc20_13TeV.525362.MGPy8EG_ssWWhjj_llvvbbjj_EW8_LO_klp1.deriv.DAOD_PHYS.e8545_s3681_r13167_r13146_p6026/DAOD_PHYS.37982158._000001.pool.root.1 --run-config ../easyjet/vbshiggsAnalysis/share/RunConfig-SemiLep.yaml --out-file output.root
```

# How to Run (Grid)
To be added
