Analysis Package for the $HH\rightarrow b\bar{b} VV$ analyis
=========================

# Folder structure

- `bin/`: Executables
  - `bbVV-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `bbVV_config`
- `share/`: yaml files containing configurations used by the components
  - `bbVV-base`: where all the common flags are set;
  - `RunConfig-bbVV[-bypass]`: configurations called by the executables (see below);
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `HHbbVVSelectorAlg`: Find if the event pass the baseline bbVV selection;
  - `BaselineVarsbbVVAlg`: Compute the baseline variables for the analysis.

# How to Run

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
- run the analysis on <span style="color: #F2385A">PHYS</span>: ```bbVV-ntupler ttbar_PHYS_10evt.root --run-config bbVVAnalysis/RunConfig-bbVV-bypass.yaml --out-file output_PHYS_bbVV.root```
- run the analysis on <span style="color: #4BD9D9;">PHYSLITE</span>: ```bbVV-ntupler ttbar_PHYSLITE_10evt.root --run-config bbVVAnalysis/RunConfig-bbVV-bypass.yaml --out-file output_PHYSLITE_bbVV.root```
An alternative is to use the preselection step using the configuration `RunConfig-bbVV.yaml` instead of `RunConfig-bbVV-bypass.yaml`, however fewer events would be recorded.

# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt4_X`.
* Reconstructed objects:
    * electron / muon  kinematics (implemented as a vector): `el_NOSYS_X` / `mu_NOSYS_X`;
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_NOSYS_X`;
    * fat jet kinematics (implemented as a vector): `recojet_antikt10UFO_NOSYS_X`;
    * $E_T^{miss}$ : `met_NOSYS_X`.
* Standard set of `bbVV` variables, including:
    * cuts to pass: `bbVV_pass_SR_NOSYS`;
    * selected lepton: `bbVV_NOSYS_Selected_Lepton_X`.


# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Kira Abeling.