
Analysis Package for the $VBS VVH(\rightarrow b\bar{b}) $ analyis
=========================

# Folder structure

- `bin/`: Executables
  - `vbshiggs-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `*_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-fullLep.yaml`: configurations called by the executables (see below);
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `vbshiggsFullLepSelectorAlg`: Find if the event pass the baseline VBS Fully Leptonic selection;
  - `vbshiggsSemiLepSelectorAlg`: Find if the event pass the baseline VBS Semi-Leptonic selection;
  - `vbshiggsFullHadSelectorAlg`: Find if the event pass the baseline VBS Fully Hadrnic selection;
  - `BaselineVarsvbshiggsAlg`: Compute the baseline variables for the analysis.

# How to Run (local)

```vbshiggs-ntupler /eos/user/i/izvelisc/VBS_Higgs/DAOD_PHYS/MC/LepChan/mc20a/mc20_13TeV.525362.MGPy8EG_ssWWhjj_llvvbbjj_EW8_LO_klp1.deriv.DAOD_PHYS.e8545_s3681_r13167_r13146_p6026/DAOD_PHYS.37982158._000001.pool.root.1 --run-config ../easyjet/vbshiggsAnalysis/share/RunConfig-fullLep.yaml --out-file output.root```

# How to Run (Grid)
To be added