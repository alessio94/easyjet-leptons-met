Analysis Package for the $HH\rightarrow b\bar{b} \tau\tau$ analyis
=========================

# Folder structure

- `bin/`: Executables
  - `ttHH-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `ttHH_config`
- `share/`: yaml files containing configurations used by the components
  - `ttHH-base-config`: where all the common flags are set;
  - `RunConfig-X-ttHH`: configurations called by the executables (see below);
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `JetPairingAlgttHH`: Pair jets from HH decay;
  - `BaselineVarsttHHAlg`: Compute the baseline variables for the analysis.

# How to Run

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
- run the analysis on <span style="color: #F2385A">PHYS</span>: ```ttHH-ntupler ttbar_PHYS_10evt.root --run-config ttHHAnalysis/RunConfig-PHYS-ttHH.yaml --out-file output_PHYS_ttHH.root```
- run the analysis on <span style="color: #4BD9D9;">PHYSLITE</span>: ```ttHH-ntupler ttbar_PHYSLITE_10evt.root --run-config ttHHAnalysis/RunConfig-PHYSLITE-ttHH.yaml --out-file output_PHYSLITE_ttHH.root```

# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt4_X`.
* Reconstructed objects:
    * electron / muon  kinematics (implemented as a vector): `el_NOSYS_X` / `mu_NOSYS_X`;
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_NOSYS_X`;
    * Higgses kinematics: `H1_X` /`H2_X`;
    * global quantities `n_leptons`, `njets`, `nBjets`, `HT`.
* Standard set of `ttHH` variables, including:
    * selected jet kinematics (Y from 1 to 4): `Jet_X_BY`;
    * their delta R (M,N,O,P from 1 to 4): `Jets_DeltaRMN`, `Jets_DeltaRMNO` and `Jets_DeltaRMNOP`;
    * the jet pairing outputs: `Jets_DeltaR[Max,Min,Mean]` and `Jets_DeltaREta[Max,Min,Mean]`.

# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Giulia Di Gregorio, Luis Falda Coelho.