Minimal Analysis Package example
================================

# Folder structure

- `bin/`: Executables
  - `example-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `example_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-example-base`: where all the common flags are set, configurations called by the executables (see below).
- `src/`: C++ code
  - `BaselineVarsExampleAlg`: Compute the baseline variables for the analysis.
  - `MjjExampleAlg`: Compute the invariant mass of two small-R jets (simple algorithm example).

# How to Run

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
- run the analysis on <span style="color: #F2385A">PHYS</span>: ```example-ntupler ttbar_PHYS_10evt.root --run-config ExampleAnalysis/RunConfig-example-base.yaml --out-file output_PHYS_example.root```
- run the analysis on <span style="color: #4BD9D9;">PHYSLITE</span>: ```example-ntupler ttbar_PHYSLITE_10evt.root --run-config ExampleAnalysis/RunConfig-example-base.yaml --out-file output_PHYSLITE_example.root```

# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Reconstructed objects:
    * photon kinematics (implemented as a vector): `ph_X_NOSYS`;
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_X_NOSYS` and `recojet_antikt4PFlow_X_NOSYS`;
    * example variables: `baseline_example_mjj_NOSYS`, `example_mjj_NOSYS` and `baseline_example_n_photons_NOSYS`.
