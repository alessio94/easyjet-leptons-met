Analysis Package for Analysis Package for the $HH\rightarrow b\bar{b} b\bar{b}$ analyis
=========================

# Folder structure

- `bin/`: Executables
  - `bbbb-ntupler`: produces ntuples;
  - `bbbb-hists`: produces .h5 histograms (discarded).
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `config`:
    - `dihiggs`: main controller;
    - `boosted`: for the boosted analysis ntuples;
    - `resolved`: for the resolved analysis ntuples;
    - `boosted_histograms`: for the boosted histograms (discarded).
  - `utils/option_validation.py`: set of checks to define the correct flags.
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-bbbbb-base.yaml`: base configurations;
  - `RunConfig-HH4b-X.yaml`: configurations called by the executables for either the HH4b 'Resolved' or 'Boosted' analysis or 'All' for their combination;
  - `RunConfig-SH4b*.yaml`: configurations called by the executables for the SH4b analysis;
  - `triggers*.yaml`: list of triggers to use;
  - `histonly-config.yaml`: where all the common flags are set (discarded).
- `src/`: C++ code
  - `bbbbSelectorAlg`: pre-select events to reduce the file size;
  - `SmallRJetTriggerSFAlg`: assign trigger scale-factor
  - `TriggerDecoratorAlg`: trigger bucket for resolved analysis
  - `BaselineVarsBoostedAlg` / `BaselineVarsResolvedAlg`: compute the baseline variables for the analysis, is not used now as we prefer vertor-like outputs;
  - `JetPairingAlg`: pair the jets together to the Higgs container (see explanations [here](https://gitlab.cern.ch/easyjet/easyjet/-/blob/main/bbbbAnalysis/docs/Algorithms.md?ref_type=heads#jetpairingalg)), is not used now as we prefer vertor-like outputs;
  - `JetBoostHistograms` / `JetBoostHistogramsAlg`: Defines and runs the H5 histogramming of jet quantities for the $SH$ boosted analysis (discarded);
  - `MassPlaneBoostHistograms` / `MassPlaneBoostHistogramsAlg`: Defines and runs the H5 histogramming of mHH for different matching criteria for the $SH$ boosted analysis (discarded).
- `datasets/`: input DAOD samples for grid submission
- `scripts/`: grid submission scripts

# How to Run

1.  Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations (only <span style="color: #F2385A">PHYS</span>).

2. Run the ntupler on those files (current strategy is to produce combined ntuples for both resolved and boosted analysis):
    - Combined: ```bbbb-ntupler --run-config bbbbAnalysis/RunConfig-HH4b-All.yaml -o output_PHYS_bbbb_resolved_ntuple.root  ttbar_PHYS_10evt.root```

3. Input PHYS samples can be found in the "datasets" directory.

4. Choose your run-config file.

5. Running on the Grid (* Please test your setup before running bulk grid submission)
- This is hadled by the easyjet-gridsubmit script. To run a test job on the grid please use the following command:
"easyjet-gridsubmit --mc-list input_sample.txt --exec bbbb-ntupler --run-config bbbbAnalysis/RunConfig-All.yaml --campaign TestXXX --nGBperJob 2 --noTag"

- It is recommended to run from a central tag for large scale productions. If you have some local updates, you will be invited to commit them and a tag will be created to be pushed on your fork. If you want to avoid this, you can use the --noTag options.

- Grid submission scripts are now available in the "scripts" directory.

# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt4_X`, `truthjet_antikt10_X`.
* Reconstructed objects:
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_X___NOSYS`, `recojet_antikt10UFO_X___NOSYS`.

# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Arely Cortes Gonzalez, Dan Guest, Victor Hugo Ruelas Rivera, Teng Jian Khoo, Frederic Renner.
