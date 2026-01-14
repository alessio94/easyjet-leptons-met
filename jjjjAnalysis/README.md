Analysis Package for the 4-jets xs measurements and Quantum Gravity searches.
=======================================

# Folder structure

- `bin/`: Executables
  - `jjjj-ntupler`
- `pyhon/`: Main python code to configure the components (objects, selections as well as the variables to save).
  - `jjjj_config.py`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-jjjj-common.yaml`: Common configuration.
  - `RunConfig-jjjj-bypass.yaml`: Running with bypass mode used for CI testing.
  - `RunConfig-jjjj-syst.yaml`: enable systematic uncertainties.
  - `trigger.yaml`: list of triggers to use per year.
- `src/`: C++ code
  - `BaselineVarsjjjjAlg`: Compute the baseline variables.
  - `jjjjSelectorAlg`: Baseline selection
- `scripts`: scripts for submitting grid jobs
# How to Run (locally)
```
cd build
source ../easyjet/setup.sh
source x86_64*/setup.sh
cd ../run
jjjj-ntupler $PATH_TO_DAOD_FILE --run-config jjjjAnalysis/RunConfig-jjjj-comman.yaml --out-file output.root -e 500
```

# How to submit grid jobs
See [`scripts/README.md`](./scripts/README.md) for instructions.