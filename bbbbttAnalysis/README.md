Analysis Package for the $HHH\rightarrow b\bar{b}b\bar{b} \tau\tau$ analysis
=========================

# Folder structure

- `bin/`: Executables
  - `bbbbtt-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `bbbbtt_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-bbbbtt[-bypass]`: where all the common flags are set, configurations called by the executables (see below).
- `src/`: C++ code
  - `HHHbbbbttSelectorAlg`: Find if the event pass the baseline bbbbtt selection [**STILL HAS TO BE ADAPTED!**];
  - `MMCDecoratorAlg`: Compute the tau tau MMC mass;
  - `MMCSelectorAlg`: Filter events based on MMC;
  - `BaselineVarsbbbbttAlg`: Compute the baseline variables for the analysis.

# How to Run

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
- run the analysis on <span style="color: #F2385A">PHYS</span>: ```bbbbtt-ntupler ttbar_PHYS_10evt.root --run-config bbbbttAnalysis/RunConfig-bbbbtt-bypass.yaml --out-file output_PHYS_bbbbtt.root```
- run the analysis on <span style="color: #4BD9D9;">PHYSLITE</span>: ```bbbbtt-ntupler ttbar_PHYSLITE_10evt.root --run-config bbbbttAnalysis/RunConfig-bbbbtt-bypass.yaml --out-file output_PHYSLITE_bbbbtt.root```
An alternative is to use the preselection step using the configuration `RunConfig-bbbbtt.yaml` instead of `RunConfig-bbbbtt-bypass.yaml`, however fewer events would be recorded.
- possible channels are `lephad`, `hadhad`, `lephad1b` (`lephad` one b-tag region), `hadhad1b` (`hadhad` one b-tag region), `ZCR` (Z+HF control region) and `TopEMuCR` (top control region, similar to ZCR but with different flavour leptons)

# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt4_X`.
* Reconstructed objects:
    * electron / muon / tau kinematics (implemented as a vector): `el_NOSYS_X` / `mu_NOSYS_X` / `tau_NOSYS_X`;
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_NOSYS_X`;
    * $E_T^{miss}$ : `met_NOSYS_X`.
* Standard set of `bbbbtt` variables, including: [**STILL HAS TO BE IMPLEMENTED!**]
    * cuts to pass: `bbbbtt_pass_X_NOSYS`.
    * Missing Mass Calculator (MMC) outputs: `bbbbtt_mmc_X`;
    * selected lepton: `bbbbtt_Lepton_X_NOSYS`;
    * Leading and sub-leading taus (kinematics + efficiency SF): `bbbbtt_Leading_Tau_X` and `bbbbtt_Sublead_Tau_X`;
    * Leading and sub-leading b-jets: `bbbbtt_Leading_Bjet_X` and `bbbbtt_Sublead_Bjet_X`;
    * Reconstructed Higgs candidates: `bbbbtt_H_bb_X` and `bbbbtt_H_vis_tautau_X`;
    * HH variables: `bbbbtt_HH_X` and `bbbbtt_HH_vis_X`.

# Main developers

This code is an adaptation from the bbtt package. The main developments in this HHH package have been performed by (non extensive list, feel free to add your name):
Marcus Rodrigues, Jordy Degens
