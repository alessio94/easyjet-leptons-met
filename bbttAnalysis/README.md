Analysis Package for the $HH\rightarrow b\bar{b} \tau\tau$ analyis
=========================

# Folder structure

- `bin/`: Executables
  - `bbtt-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `bbtt_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-bbtt[-bypass]`: where all the common flags are set, configurations called by the executables (see below).
- `src/`: C++ code
  - `HHbbttSelectorAlg`: Find if the event pass the baseline bbtt selection;
  - `MMCDecoratorAlg`: Compute the tau tau MMC mass;
  - `MMCSelectorAlg`: Filter events based on MMC;
  - `BaselineVarsbbttAlg`: Compute the baseline variables for the analysis.

# How to Run

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
- run the analysis on <span style="color: #F2385A">PHYS</span>: ```bbtt-ntupler ttbar_PHYS_10evt.root --run-config bbttAnalysis/RunConfig-bbtt-bypass.yaml --out-file output_PHYS_bbtt.root```
- run the analysis on <span style="color: #4BD9D9;">PHYSLITE</span>: ```bbtt-ntupler ttbar_PHYSLITE_10evt.root --run-config bbttAnalysis/RunConfig-bbtt-bypass.yaml --out-file output_PHYSLITE_bbtt.root```
An alternative is to use the preselection step using the configuration `RunConfig-bbtt.yaml` instead of `RunConfig-bbtt-bypass.yaml`, however fewer events would be recorded.
- possible channels are `lephad`, `hadhad` and `hadhad1b` (`hadhad` one b-tag region)

# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt4_X`.
* Reconstructed objects:
    * electron / muon / tau kinematics (implemented as a vector): `el_NOSYS_X` / `mu_NOSYS_X` / `tau_NOSYS_X`;
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_NOSYS_X`;
    * $E_T^{miss}$ : `met_NOSYS_X`.
* Standard set of `bbtt` variables, including:
    * cuts to pass: `bbtt_pass_X_NOSYS`.
    * Missing Mass Calculator (MMC) outputs: `bbtt_mmc_X`;
    * selected lepton: `bbtt_Lepton_X_NOSYS`;
    * Leading and sub-leading taus (kinematics + efficiency SF): `bbtt_Leading_Tau_X` and `bbtt_Sublead_Tau_X`;
    * Leading and sub-leading b-jets: `bbtt_Leading_Bjet_X` and `bbtt_Sublead_Bjet_X`;
    * Reconstructed Higgs candidates: `bbtt_H_bb_X` and `bbtt_H_vis_tautau_X`;
    * HH variables: `bbtt_HH_X` and `bbtt_HH_vis_X`.

# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Yimin Che, Jordy Degens, Minori Fujimoto, Thomas Strebler.
