Analysis Package for the $HH\rightarrow$ multilepton analyis
=========================

# Folder structure

- `bin/`: Executables
  - `hhml-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `hhml_config`
- `share/`: yaml files containing configurations used by the components
  - `hhml-base`: where all the common flags are set;
  - `RunConfig-hhml`: configurations called by the executables (see below);
  - `RunConfig-hhml-bypass`: configurations called by the executables (see below). Runs the code in "pass-through" mode aka no-skimming is applied;		
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `MultipeltonSelectorAlg`: Find if the event pass the baseline hhml selection;
  - `BaselineVarsMultipeltonAlg`: Compute the baseline variables for the analysis.

# How to Run

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
- run the analysis on <span style="color: #F2385A">PHYS</span>: ```hhml-ntupler ttbar_PHYS_10evt.root --run-config multileptonAnalysis/RunConfig-multilepton.yaml --out-file output_PHYS_hhml.root```
- run the analysis on <span style="color: #4BD9D9;">PHYSLITE</span>: ```hhml-ntupler ttbar_PHYSLITE_10evt.root --run-config multileptonAnalysis/RunConfig-multilepton.yaml --out-file output_PHYSLITE_hhml.root```

3. Input PHYS and PHYLITE samples can be found in the "datasets" directory

4. Choose your run-config file (e.g. multileptonAnalysis/RunConfig-multilepton.yaml or multileptonAnalysis/RunConfig-multilepton-bypass.yaml for skimmed and unskimmed production, respectively)

5. Running on the Grid (* Please test your setup before running bulk grid submission)
- This is hadled by the easyjet-gridsubmit script. To run a test job on the grid please use the following command:
`easyjet-gridsubmit --mc-list input_sample.txt --exec hhml-ntupler --run-config multileptonAnalysis/RunConfig-multilepton.yaml --campaign TestXXX --noTag`

- It is recommended to run from a central tag for large scale productions. If you have some local updates, you will be invited to commit them and a tag will be created to be pushed on your fork. If you want to avoid this, you can use the --noTag options.

- Grid submission scripts are now available in the "scripts" directory for NonResonant and Resonant production. Separate scripts are used for skimmed and unskimmed ntuple production (e.g. RunOnGrid_XHH_prod_Run2_skim.sh and  RunOnGrid_XHH_prod_Run2_unskimmed.sh, respectively). 


# Output (Under development)

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
* Missing Mass Calculator (MMC) outputs: `mmc_X`
* Standard set of `hhml` variables, including:
    * TBA
    * ...


# Postprocessing (Under development)
You will need to execute the following command to tun the post-processing:
hhmlPostProcess.py --inFile easyjet_ntuple.root --xSectionsConfig ../easyjet/multileptonAnalysis/share/XSectionData.yaml --outFile output_postprocessed_ntuple.root --mergeMyFiles --mergeToOutput

The output file includes the final weight ("weight") which is evaluated considering the final sum of weights, cross-section, luminosity, MC weight and the different scale factors (`SF = TBA...`):
`weight = eventWeight * PileupWeight * Luminosity * AMIXsection * kFactor * FilterEff * SF / sumOfWeights`

The --mergeMyFiles and --mergeToOutput options make sure that the output file will have the same branches as the input with the addition of the final weight. In this way, the input ntuple is not modified.

# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Yulei Zhang, Cen Mo, Shudong Wang, Rea Thornberry.
