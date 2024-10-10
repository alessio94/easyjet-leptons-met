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

# How to Run (local)
- JJ channel
``` 
sample='/eos/atlas/unpledged/group-tokyo/users/tnobe/ForAntonio/DAODs/VBSVV4q/mc20_13TeV.521289.MGPy8_WZjj_had_LO.deriv.DAOD_PHYS.e8488_s3797_r13145_p6026/DAOD_PHYS.37964305._000007.pool.root.1'

VBSVV4q-ntupler $sample --run-config ../../easyjet/VBSVV4qAnalysis/share/RunConfig-JJ.yaml --out-file output.root -e 5000
```

# How to Run (Grid)
To be added
