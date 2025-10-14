Analysis Package for the $HH\rightarrow b\bar{b} \tau\tau$ analysis
=========================

The bbττ analysis documentation page is available at [https://atlas-hh-bbtautau.docs.cern.ch/](https://atlas-hh-bbtautau.docs.cern.ch/)

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
  - `BaselineVarsbbttAlg`: Compute the baseline variables for the analysis;
  - `BaselineVarsbBoostedbttAlg`: Compute variables for the boosted analysis;
  - `AntiTauDecoratorAlg`: Get Anti-tau and do its trigger matching; 
  - `TriggerDecoratorAlg`: Get trigger matching information;
  - `TriggerSFAlg`: Get trigger SFs;
  - `TriggerUtils`: Define trigger pass.

# Channels

The bbττ analysis is structured into multiple channels. Each channel targets a specific final state and serves as either a signal region (SR) or a control region (CR).

- `HadHad` channel: Targets bbτ<sub>had</sub>τ<sub>had</sub> + MET final state (1/2 b-tag regions)
- `LepHad` channel: Targets bbτ<sub>lep</sub>τ<sub>had</sub> + MET (bbℓτ<sub>had</sub> + MET) final sate (1/2 b-tag regions)
- `ZCR`: Targets eebb or μμbb final states (opposite-charge leptons, 2-tag region)
- `TopEMuCR`: Targets eμbb final state (opposite-charge leptons, 2-tag region)
- `AntiIsoLepHad`: Same as LepHad channel but requires the e/μ to fail the defined isolation working point.

> **⚠️ Important:**  
> All channels can be run either individually or together as described below, **except for `AntiIsoLepHad`, which must always be run alone!**

Additional config options:
- `do_antiID_regions`: For each region containing a τ<sub>had</sub>, an additional anti-τ<sub>had</sub> region (where one τ<sub>had</sub> is replaced by an anti-τ<sub>had</sub>) is created
- `do_1B_regions: true`: In the signal regions, 1‑tag categories are also included

## Triggers

- `bbttAnalysis/share/trigger_taus.yaml`: List of triggers used in the `HadHad` channel
- `bbttAnalysis/share/trigger_leptons.yaml`: List of triggers used in the `LepHad` channel (same for `ZCR` and `TopEMuCR`)
- `bbttAnalysis/share/trigger_boosted.yaml`: List of triggers used in the boosted analysis (under development)
- `bbttAnalysis/share/trigger.yaml`: List of all triggers used in the analysis
- `bbttAnalysis/share/trigger.yaml`: Configuration for trigger scale factors

## GRLs
- `bbttAnalysis/share/bbtt_grl.yaml`: List of XML files for good run lists, divided by year
- `bbttAnalysis/share/bbtt_grl_years.yaml`: directories hosting GRLs and pile-up reweighting files.
- `bbttAnalysis/lumicalc_bbtt.yaml`: List of lumicalc output files, divided by year
- `bbttAnalysis/prw_bbtt.yaml`: List of pile-up reweighting files, divided by year, includes campaign specific ones

# Analysis config files

- `RunConfig-bbtt-base.yaml`: Baseline bbττ config (imported into othere configs)
- `RunConfig-bbtt-hadhad.yaml`: Run `HadHad` channel **without** systematics
- `RunConfig-bbtt-syst-hadhad.yaml`: Run `HadHad` channel **with** systematics`

Similarly, configs (to run single or multiple channels with or without systematics) are available for:

- `all-channels`: `HadHad`, `LepHad`, `ZCR`, `TopEMuCR`, `Boosted`
- `lep`: `LepHad`
- `cr`: `ZCR`, `TopEMuCR`
- `antiiso`: `AntiIsoLepHad` (must always be run alone, no systematics needed)
- `boosted`: `Boosted`, `HadHad` (under development)

> **⚠️ Important:**
> The `bypass` mode (indicated in the config name) allows all events to be saved in the final output (no preselection will be applied). This is convenient for development, efficiency studies, debugging, and similar tasks. However, it is **not feasible** to use `bypass` mode for full nTuple production. **Interpreting results obtained in this mode requires expert knowledge of the analysis.**

# How to Run

> **⚠️ Important:**
> **Refer to the general EasyJet README for instructions on cloning, compiling, and setting up the code based on your specific use case**

> **⚠️ Important:**
> **The bbττ analysis can currently only be run on PHYS derivations; it does not support PHYSLITE**

1. Get the files to make the test: have a look at the general [README section](https://gitlab.cern.ch/easyjet/easyjet#running-on-files) for updated informations.

2. Run the ntupler on those files:
```
bbtt-ntupler ttbar_PHYS_10evt.root --run-config ../easyjet/bbttAnalysis/share/RunConfig-bbtt-hadhad.yaml --channels HadHad --out-file output_PHYS_bbtt.root
```

3. Grid submission
```
lsetup rucio 
voms-proxy-init -voms atlas 
lsetup panda
easyjet-gridsubmit --exec bbtt-ntupler --run-config ../easyjet/bbttAnalysis/share/RunConfig-bbtt-hadhad.yaml --channels HadHad --noTag --framework easyjet --campaign FancyCampaignName --mc-list ../easyjet/bbttAnalysis/datasets/PHYS/prod/mc23/mc23_13p6TeV.HH_bbtt.prod.txt
```

# nTuple production (instructions based on v7)

> **⚠️ Important:**
> Don't do this alone, coorinate with the analysis team and analysis contacts

> **⚠️ Important:**
> Note that the `--channels` argument must be provided in the run command even if it is already defined in the config file

> **⚠️ Important:**  
> Setting `--nGBperJob 3` results in a very large number of output files with few events per file, which significantly slows down HHARD. This option should only be used when running with systematics.  
> **TODO: Update the instructions to support merging files directly on the grid when this option is used.**


1. Clone the code with the right tag
```
setupATLAS
lsetup git
git lfs install
git clone --recursive -b 0.35.0 ssh://git@gitlab.cern.ch:7999/easyjet/easyjet.git
cd easyjet
git fetch --tags
git checkout 0.35.0
```
2. Compile the code
```
cd ../
mkdir build run
cd build
source ../easyjet/setup.sh
cmake ../easyjet/
make
source */setup.sh
cd ../
```
3. For submitting on the grid
```
cd run/
lsetup panda
lsetup rucio 
voms-proxy-init -voms atlas
```
4. Submit from the run directory
```
# For HadHad (MC20 with systematics, similar for MC23)
for f in ../easyjet/bbttAnalysis/datasets/PHYS/prod/mc20/mc*.txt; do easyjet-gridsubmit --exec bbtt-ntupler --run-config ../easyjet/bbttAnalysis/share/RunConfig-bbtt-syst-hadhad.yaml --channels HadHad --noTag --framework easyjet --excluded-site AGLT2,NIKHEF,SWT2_CPB,BNL,CERN-T0,BNL_OPP --nGBperJob 3 --campaign EJ_0_35_0_v7 --mc-list $f; done

# For LepHad + ZCR + TopEMuCR (MC20 with systematics, similar for MC23)
for f in #../easyjet/bbttAnalysis/datasets/PHYS/prod/mc20/mc*.txt; do easyjet-gridsubmit --exec bbtt-ntupler --run-config ../easyjet/bbttAnalysis/share/RunConfig-bbtt-syst-lep.yaml --channels LepHad ZCR TopEMuCR --noTag --framework easyjet --excluded-site AGLT2,NIKHEF,SWT2_CPB,BNL,CERN-T0,BNL_OPP --nGBperJob 3 --campaign EJ_0_35_0_v7 --mc-list $f; done

# !!! Important !!! Cleanup your run directory with rm -f ./* before launching the data submission

# For HadHad (Data Run 2, similar for Run 3)
for f in ../easyjet/bbttAnalysis/datasets/PHYS/prod/mc20/data*.txt; do easyjet-gridsubmit --exec bbtt-ntupler --run-config ../easyjet/bbttAnalysis/share/RunConfig-bbtt-syst-hadhad.yaml --channels HadHad --noTag --framework easyjet --excluded-site AGLT2,NIKHEF,SWT2_CPB,BNL,CERN-T0,BNL_OPP --nGBperJob 3 --campaign EJ_0_35_0_v7 --data-list $f; done

# For LepHad + ZCR + TopEMuCR (Data Run 2, similar for Run 3)
for f in ../easyjet/bbttAnalysis/datasets/PHYS/prod/mc20/data*.txt; do easyjet-gridsubmit --exec bbtt-ntupler --run-config ../easyjet/bbttAnalysis/share/RunConfig-bbtt-syst-lep.yaml --channels LepHad ZCR TopEMuCR --noTag --framework easyjet --excluded-site AGLT2,NIKHEF,SWT2_CPB,BNL,CERN-T0,BNL_OPP --nGBperJob 3 --campaign EJ_0_35_0_v7 --data-list $f; done

# AntiIsoLepHad (MC20, similar for MC23, no systematics)
for f in ../easyjet/bbttAnalysis/datasets/PHYS/prod/mc20/mc*.txt; do easyjet-gridsubmit --exec bbtt-ntupler --run-config ../easyjet/bbttAnalysis/share/RunConfig-bbtt-antiiso.yaml --channels AntiIsoLepHad --noTag --framework easyjet --excluded-site AGLT2,NIKHEF,SWT2_CPB,BNL,CERN-T0,BNL_OPP --nGBperJob 3 --campaign EJ_0_35_0_v7 --mc-list $f; done
```

# Input files

The list of PHYS files used in the bbττ analysis is maintained in `bbttAnalysis/datasets/PHYS/prod` for nominal samples and in `bbttAnalysis/datasets/PHYS/prod_sys` for alternative samples

# Output

If these run properly, your outputs files should contain a TTree `AnalysisMiniTree` with the following content (X denotes a set of variables associated to the object, usually pT, Eta ...):
* Some information saved for every analyses displayed in the main [README section](https://gitlab.cern.ch/easyjet/easyjet#have-a-look-at-the-output);
* Truth information:
    * jets (implemented as a vector): `truthjet_antikt4_X`.
* Reconstructed objects:
    * electron / muon / tau kinematics (implemented as a vector): `el_NOSYS_X` / `mu_NOSYS_X` / `tau_NOSYS_X`;
    * jet kinematics (implemented as a vector): `recojet_antikt4PFlow_NOSYS_X`;
    * $E_T^{miss}$ : `met_NOSYS_X`.
* Standard set of `bbtt` variables only for nominal jobs, including:
    * cuts to pass: `bbtt_pass_X_NOSYS`.
    * Missing Mass Calculator (MMC) outputs: `bbtt_mmc_X`;
    * selected lepton: `bbtt_Lepton_X_NOSYS`;
    * Leading and sub-leading taus (kinematics + efficiency SF): `bbtt_Leading_Tau_X` and `bbtt_Sublead_Tau_X`;
    * Leading and sub-leading b-jets: `bbtt_Leading_Bjet_X` and `bbtt_Sublead_Bjet_X`;
    * Reconstructed Higgs candidates: `bbtt_H_bb_X` and `bbtt_H_vis_tautau_X`;
    * HH variables: `bbtt_HH_X` and `bbtt_HH_vis_X`.
* The whole output branch list can be found [here](https://gitlab.cern.ch/atlas-physics/HDBS/DiHiggs/bbtautau/HHARD/-/tree/master/docs/ntuples?ref_type=heads) in the [HHARD](https://gitlab.cern.ch/atlas-physics/HDBS/DiHiggs/bbtautau/HHARD) postprocessing framework 

# Main developers

The main developments have been performed by (non extensive list, feel free to add your name):
Yimin Che, Jordy Degens, Minori Fujimoto, Thomas Strebler, Salah-Eddine Dahbi, Philipp Rincke, Greg Myers, Leonardo Splendori, Brian Moser, Petar Bokan

