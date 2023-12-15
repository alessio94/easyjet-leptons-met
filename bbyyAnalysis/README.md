Analysis Package for $HH\rightarrow b\bar{b} \gamma\gamma$ analyis
=========================

# Folder structure
- `bin/`: Executables
  - `bbyy-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `bbyy_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-bbyy`: where all the common flags are set, configurations called by the executables (see below);
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `SelectionFlagsbbyyAlg`: Find if the event pass the baseline bbyy selection;
  - `BaselineVarsbbyyAlg`: Compute the baseline variables for the analysis.


# How to Run

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
- run the analysis on <span style="color: #F2385A">PHYS</span>: ```bbyy-ntupler ttbar_PHYS_10evt.root --run-config bbyyAnalysis/RunConfig-bbyy.yaml --out-file output_PHYS_bbyy.root```
- run the analysis on <span style="color: #4BD9D9;">PHYSLITE</span>: ```bbyy-ntupler ttbar_PHYSLITE_10evt.root --run-config bbyyAnalysis/RunConfig-bbyy.yaml --out-file output_PHYSLITE_bbyy.root```

# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt4_X`.
* Reconstructed objects:
    * photon kinematics (implemented as a vector): `ph_NOSYS_X`;
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_NOSYS_X`;
    * global variables: `NOSYS_nJets`, `NOSYS_nPhotons`...
* Standard set of `bbyy` variables, including:
    * cuts to pass: `NOSYS_PASS_TRIGGER`, `NOSYS_TWO_TIGHTID_ISO_PHOTONS`...
    * leading and sub-leading photons: `NOSYS_Leading_Photon_X` and `NOSYS_Subleading_Photon_X` + the related variables `NOSYS_myy`...
    * leading and sub-leading b-tagged jets: `NOSYS_Jet_X_b1` and `NOSYS_Jet_X_b2` + the related variables `NOSYS_mbb`...
    * HH variables: `NOSYS_mbbyy`...

Note that running with the `bbyy-ntupler` executable will add the standard `bbyy` variables above, defined in `bbyyAnalysis/src/BaselineVarsbbyyAlg.cxx` and `bbyyAnalysis/python/bbyy_config.py`. 

One can change the outputs saved in the TTree by adjusting the base bbyy configuration file `bbyyAnalysis/share/RunConfig-bbyy.yaml`, which also defines the identification and isolation WP of the different objects.

# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Giulia Di Gregorio, Sayuka Kita, Spyros Merianos, Lorenzo Santi, Abraham Tishelman-Charny.
