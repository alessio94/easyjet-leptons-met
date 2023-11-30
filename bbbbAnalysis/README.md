Analysis Package for Analysis Package for the $HH\rightarrow b\bar{b} b\bar{b}$ analyis
=========================

# Folder structure

- `bin/`: Executables
  - `bbbb-hists`: produces .h5 histograms;
  - `bbbb-ntupler`: produces ntuples.
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `config`:
    - `dihiggs`: define the jets associated to the HH;
    - `boosted` / `boosted_histograms`: for the boosted analysis ntuples and histograms;
    - `resolved`: for the resolved analysis ntuples.
  - `utils/option_validation.py`: set of checks to define the correct flags.
- `share/`: yaml files containing configurations used by the components
  - `histonly-config.yaml`: where all the common flags are set;
  - `RunConfig-X`: configurations called by the executables for either the 'Resolved' or 'SH4b' analyses (see below).
- `src/`: C++ code
  - `JetPairingAlg`: pair the jets together to the Higgs container (see explanations [here](https://gitlab.cern.ch/easyjet/easyjet/-/blob/main/bbbbAnalysis/docs/Algorithms.md?ref_type=heads#jetpairingalg));
  - `BaselineVarsBoostedAlg` / `BaselineVarsBoostedAlg`: Compute the baseline variables for the analysis (see explanations [here](https://gitlab.cern.ch/easyjet/easyjet/-/blob/main/bbbbAnalysis/docs/Algorithms.md?ref_type=heads#baselinevarsresolvedalg) and [there](https://gitlab.cern.ch/easyjet/easyjet/-/blob/main/bbbbAnalysis/docs/Algorithms.md?ref_type=heads#baselinevarsboostedalg));
  - `JetBoostHistograms` / `JetBoostHistogramsAlg`: Defines and runs the H5 histogramming of jet quantities for the $SH$ boosted analysis;
  - `MassPlaneBoostHistograms` / `MassPlaneBoostHistogramsAlg`: Defines and runs the H5 histogramming of mHH for different matching criteria for the $SH$ boosted analysis.

# How to Run

1.  Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations (only <span style="color: #F2385A">PHYS</span>).

2. Run the ntupler on those files:
    - Resolved: ```bbbb-ntupler ttbar_PHYS_10evt.root --run-config bbbbAnalysis/RunConfig-Resolved.yaml --out-file output_PHYS_bbbb_resolved_ntuple.root ```
    - Boosted: ```bbbb-ntupler ttbar_PHYS_10evt.root --run-config bbbbAnalysis/RunConfig-SH4b.yaml --out-file output_PHYS_bbbb_boosted_ntuple.root ```

# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt4_X`.
* Reconstructed objects:
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_NOSYS_X`.

And for the resolved analysis:
* For the different b-tagging efficiencies (YY) some di-jet variables: `resolved_DL1dv01_FixedCutBEff_YY_X`.

While for the boosted analyis:
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt10_X`.
* Reconstructed objects:
    * fat jet kinematics (implemented as a vector): `recojet_antikt10UFO_NOSYS_X`.

# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Arely Cortes Gonzalez, Dan Guest, Victor Hugo Ruelas Rivera, Teng Jian Khoo, Frederic Renner.  