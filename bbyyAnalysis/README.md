Analysis Package for $(S/H)H\rightarrow b\bar{b} \gamma\gamma$ analyis
=========================

Analyses on GLANCE:

[$SH\rightarrow b\bar{b} \gamma\gamma$](https://atlas-glance.cern.ch/atlas/analysis/analyses/details.php?ref_code=ANA-HIGP-2024-34)

[$HH\rightarrow b\bar{b} \gamma\gamma$](https://atlas-glance.cern.ch/atlas/analysis/analyses/details.php?ref_code=ANA-HIGP-2024-35)

See here a link to the full SH analysis workflow: [[SH(bbyy) analysis workflow]](https://codimd.web.cern.ch/8fyJeyIKQguxXBUaOLa_Dg)

# Folder structure
- `bin/`: Executables
  - `bbyy-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `bbyy_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-bbyy-base`: where all the common flags are set, configurations called by the executables, no `CutList` defined here, all configs that define a `CutList` should include this (see below);
  -`RunConfig-bbyy`: includes `RunConfig-bbyy-base` adding a `CutList`;
  - `RunConfig-bbyy-bypass`: sets bypass flag to true, flags for the cuts are saved, but no cut is actually applied;
  - Skimming configs, events that do not pass the selection are not saved:
    - `RunConfig-bbyy-skimming-legacy`: config for ntuple skimming with legacy selection, CutList overwrites default one
    - `RunConfig-bbyy-skimming-loose`: config for ntuple skimming with loose selection, CutList overwrites default one
  - Resonant configs, for the $SH\rightarrow b\bar{b} \gamma\gamma$ analysis:
    - `RunConfig-Resonant-base`: Contains base parameters for the SH analysis, including PNN score computation and mass pairs.
    - `RunConfig-Resonant-Default`: Contains parameters for running the default SH analysis, includes the resolved and "1 b-jet" cases.
    - `RunConfig-Resonant-Boosted`: R&D config for studying the boosted regime of the SH mass grid. Includes GN2X score computation, large R jets.
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `bbyySelectorAlg`: Find if the event pass the bbyy selection defined in the config passed to the ntupler;
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
Giulia Di Gregorio, Sayuka Kita, Spyros Merianos, Lorenzo Santi, Abraham Tishelman-Charny, Francesco Curcio.

# Post processing
To run the post-processing, the following command needs to be executed:

`bbyyPostProcess.py --inFile name_easyJet_ntuple.root --xSectionsConfig bbyyAnalysis/XSectionData_Run2.yaml --outFile name_PostProcessed_ntuple.root`
or replace with `XSectionData_Run3` for Run3 samples.

The output file (`name_PostProcessed_ntuple.root`) contains the final weight (`weight`) which is evaluated considering the final sum of weights, cross-section, luminosity, MC weight and the different scale factors (`SF = weight_ftag_effSF_GN2v01_FixedCutBEff_77_NOSYS * weight_jvt_effSF_NOSYS * bbyy_Photon1_effSF_NOSYS * bbyy_Photon2_effSF_NOSYS`):
`weight = generatorWeight_%SYS * PileupWeight * Luminosity * AMIXsection * kFactor * FilterEff * SF / sumOfWeights`
