Analysis Package for the ZQG analyis
=========================

# Folder structure

- `bin/`: Executables
  - `ZQG-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `ZQG_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-ZQG`: configurations called by the executables (see below);
  - `RunConfig-ZQG-bypass`: configurations called by the executables (see below). Runs the code in "pass-through" mode aka no-skimming is applied;		
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `ZQGSelectorAlg`: Find if the event pass the baseline ZQG selection;
  - `BaselineVarsZQGAlg`: Compute the baseline variables for the analysis.

# How to Run

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
- run the analysis on <span style="color: #F2385A">PHYS</span>: ```ZQG-ntupler ttbar_PHYS_10evt.root --run-config ZQGAnalysis/RunConfig-ZQG.yaml --out-file output_PHYS_ZQG.root```
- run the analysis on <span style="color: #4BD9D9;">PHYSLITE</span>: ```ZQG-ntupler ttbar_PHYSLITE_10evt.root --run-config ZQGAnalysis/RunConfig-ZQG.yaml --out-file output_PHYSLITE_ZQG.root```

3. Input PHYS and PHYLITE samples can be found in the "datasets" directory

4. Choose your run-config file (e.g. ZQGAnalysis/RunConfig-ZQG.yaml or ZQGAnalysis/RunConfig-ZQG-bypass.yaml for skimmed and unskimmed production, respectively)

5. Running on the Grid (* Please test your setup before running bulk grid submission)
- This is hadled by the easyjet-gridsubmit script. To run a test job on the grid please use the following command:
"easyjet-gridsubmit --mc-list input_sample.txt --exec ZQG-ntupler --run-config ZQGAnalysis/RunConfig-ZQG.yaml --campaign TestXXX  --nGBperJob 5 --noTag"

- It is recommended to run from a central tag for large scale productions. If you have some local updates, you will be invited to commit them and a tag will be created to be pushed on your fork. If you want to avoid this, you can use the --noTag options.

- Grid submission scripts are now available in the "scripts" directory. Separate scripts are used for skimmed and unskimmed ntuple production (e.g. RunOnGrid_Run2_skim.sh and  RunOnGrid_Run2_unskimmed.sh, respectively). 


# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);

# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Yi Yu, Vithyaban Anjelo Narendran
