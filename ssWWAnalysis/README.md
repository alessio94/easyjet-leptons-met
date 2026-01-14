Analysis Package for the VBS ssWW polarisation analyis
=========================

# Folder structure

- `bin/`: Executables
  - `ssWW-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `ssWW_config`
- `share/`: yaml files containing configurations used by the components
  - `ssWW-base`: where all the common flags are set;
  - `RunConfig-ssWW`: configurations called by the executables (see below);
  - `RunConfig-ssWW-bypass`: configurations called by the executables (see below). Runs the code in "pass-through" mode aka no-skimming is applied;		
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `ssWWSelectorAlg`: Find if the event pass the baseline ssWW selection;
  - `BaselineVarsssWWAlg`: Compute the baseline variables for the analysis.

# How to Run

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
- run the analysis on <span style="color: #F2385A">PHYS</span>: ```ssWW-ntupler ttbar_PHYS_10evt.root --run-config ssWWAnalysis/RunConfig-ssWW.yaml --out-file output_PHYS_ssWW.root```
- run the analysis on <span style="color: #4BD9D9;">PHYSLITE</span>: ```ssWW-ntupler ttbar_PHYSLITE_10evt.root --run-config ssWWAnalysis/RunConfig-ssWW.yaml --out-file output_PHYSLITE_ssWW.root```

3. Input PHYS and PHYLITE samples can be found in the "datasets" directory

4. Choose your run-config file (e.g. ssWWAnalysis/RunConfig-ssWW.yaml or ssWWAnalysis/RunConfig-ssWW-bypass.yaml for skimmed and unskimmed production, respectively)

5. Running on the Grid (* Please test your setup before running bulk grid submission)
- This is hadled by the easyjet-gridsubmit script. To run a test job on the grid please use the following command:
"easyjet-gridsubmit --mc-list input_sample.txt --exec ssWW-ntupler --run-config ssWWAnalysis/RunConfig-ssWW.yaml --campaign TestXXX  --nGBperJob 5 --noTag"

- It is recommended to run from a central tag for large scale productions. If you have some local updates, you will be invited to commit them and a tag will be created to be pushed on your fork. If you want to avoid this, you can use the --noTag options.

- Grid submission scripts are now available in the "scripts" directory. Separate scripts are used for skimmed and unskimmed ntuple production (e.g. RunOnGrid_Run2_skim.sh and  RunOnGrid_Run2_unskimmed.sh, respectively). 


# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt4_X`.
* Reconstructed objects:
    * electron / muon  kinematics (implemented as a vector): `el_NOSYS_X` / `mu_NOSYS_X`;
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_NOSYS_X`;
    * $E_T^{miss}$ : `met_NOSYS_X`;
    * global quantities `NOSYS_nJets`, `NOSYS_nElectrons` ...
    * some (duplicated ?) di-b variables: `NOSYS_mbb`, `NOSYS_pTbb`.
* Standard set of `ssWW` variables, including:
      * cuts to pass:  `NOSYS_PASS_TRIGGER`,  `NOSYS_TWO_MEDIUM_ISO_LEPTONS` ...
      * di-lepton mass / delta R / phi / eta : `ssWW_MXX` / `ssWW_dRXX` / `ssWW_EtaXX` / `ssWW_PhiXX`;
      * leading and sub-leading electron and muon (kinematics): `ssWW_Leading_Electron_X` / `ssWW_Sublead_Electron_X` and `ssWW_Leading_Muon_X` / `ssWW_Sublead_Muon_X`;
      * leading and sub-leading jet and b-tagged jet (kinematics): `ssWW_Leading_Jet_X` / `ssWW_Sublead_Jet_X` and `ssWW_Jet_X_b1` / `ssWW_Jet_X_b2`;
      * some di-jets kinematics: `ssWW_dRbb`, `ssWW_Etabb` and `ssWW_Phibb`.

# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Yi Yu, Thomas Strebler
