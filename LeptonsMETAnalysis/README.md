# LeptonsMETAnalysis Package

Analysis package for Z' → leptons + MET searches and other BSM physics with leptons and missing transverse energy.

## Overview

This package provides the configuration for analyzing events with:
- Electrons and/or muons
- Jets (small-R, b-tagged)
- Missing transverse energy (MET)

Suitable for:
- Z' → ℓℓ + MET searches
- BSM searches with leptons + MET signatures
- Dark matter searches with leptons
- Other new physics signatures

## Quick Start

### Complete Setup and Run (lxplus)

```bash
# Setup ATLAS environment
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
asetup AthAnalysis,25.2.76

# Source the build environment
cd ~/private/easyjet/easyjet/build
source x86_64-el9-gcc14-opt/setup.sh

# Run the analysis
easyjet-ntupler \
  -c LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  -e 100 \
  -o output.root \
  input_DAOD_PHYS.root
```

### Solution for Future Use

**Always use a fresh shell for each build:**

```bash
# Option 1: Exit and reconnect
exit
ssh alpizzin@lxplus.cern.ch

# Option 2: Start a clean subshell
bash --noprofile --norc
```

Then run the setup commands:

```bash
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
asetup AthAnalysis,25.2.76
cd ~/private/easyjet/easyjet/build
source x86_64-el9-gcc14-opt/setup.sh
```

**Golden Rule: One shell = One build**

This prevents environment variable conflicts when switching between different ATLAS releases or builds.

## Folder Structure

- `share/`: YAML configuration files
  - `RunConfig-LeptonsMET.yaml`: Main run configuration (enables MET, leptons, jets)
  - `trigger.yaml`: Lepton and MET trigger lists per data-taking year
- `datasets/`: Dataset lists for grid submission
  - `mc_Zprime_*.txt`: Z' signal samples at various mass points
  - `data_Run3_*.txt`: Run 3 data samples
  - `llmet_background_samples_*.txt`: Background MC samples
- `scripts/`: Grid submission and helper scripts
  - `submit_grid.sh`: Automated grid submission script
- Documentation:
  - `PHYSLITE_NOTES.md`: DAOD_PHYS vs PHYSLITE format compatibility
  - `GRID_SUBMISSION_GUIDE.md`: Complete grid workflow documentation
  - `GRID_CERT_TROUBLESHOOTING.md`: Grid certificate diagnostics

## Key Configuration Features

### Enabled Objects
- Electrons (min pT: 7 GeV, LooseBLayerLH ID, NonIso)
- Muons (min pT: 7 GeV, Loose ID, PflowLoose_VarRad isolation)
- Small-R jets with b-tagging (20 GeV pT threshold)
- **MET reconstruction** (enabled via `do_met: true`)
- Overlap removal between objects

### Trigger Strategy
The trigger configuration combines:
- Single lepton triggers (e24, e26, e60, mu24, mu26, mu50)
- MET triggers (xe70, xe90, xe110 for Run 2; xe65_cell for Run 3)

This dual-trigger approach ensures high efficiency for:
- High-pT lepton events (single lepton triggers)
- Events with large MET but moderate lepton pT (MET triggers)

**Note**: Trigger scale factors are disabled (`doSF: false`) because loose lepton working points lack CP-provided scale factor files. For high-MET regions (>200 GeV), trigger efficiency approaches 100% and scale factors are approximately 1.0.

### B-tagging Configuration
- Nominal working point: GN2v01_FixedCutBEff_77 (77% efficiency)
- Additional working points: 70%, 85% efficiency
- B-tagging scores saved for all jets above threshold

### Data Format Requirements

**IMPORTANT: Use DAOD_PHYS format only**

- **DAOD_PHYS**: Fully compatible with this analysis
- **DAOD_PHYSLITE**: NOT compatible

PHYSLITE contains pre-calibrated objects that are incompatible with the EasyJet calibration workflow. The framework requires access to raw jet containers for proper MET reconstruction. See `PHYSLITE_NOTES.md` for detailed technical explanation.

## Supported Data Taking Periods

The trigger configuration includes:

**Run 3 (2022-2024):**
- 2022: Early Run 3 commissioning
- 2023: Full Run 3 data taking
- 2024: Run 3 continued data taking
- MET triggers: xe65_cell (Run 3 specific)

**Run 2 (2015-2018):**
- Legacy triggers included for backward compatibility
- MET triggers: xe70, xe90, xe110

## Output Format

The output ROOT file contains an `AnalysisMiniTree` TTree with the following branches:

### Event-level Information
- Event weights and normalization factors
- Trigger decisions for all configured triggers
- Pileup reweighting information
- Event metadata (run number, event number, etc.)

### Reconstructed Physics Objects

**Electrons:**
- Branches: `el_NOSYS_pt`, `el_NOSYS_eta`, `el_NOSYS_phi`, `el_NOSYS_e`, `el_NOSYS_charge`
- Selection: LooseBLayerLH ID, NonIso, pT > 7 GeV
- Additional: isolation variables, identification scores

**Muons:**
- Branches: `mu_NOSYS_pt`, `mu_NOSYS_eta`, `mu_NOSYS_phi`, `mu_NOSYS_e`, `mu_NOSYS_charge`
- Selection: Loose ID, PflowLoose_VarRad isolation, pT > 7 GeV
- Additional: isolation variables, quality flags

**Jets:**
- Branches: `recojet_antikt4PFlow_NOSYS_pt`, `recojet_antikt4PFlow_NOSYS_eta`, etc.
- Algorithm: Anti-kt R=0.4 PFlow jets
- B-tagging: GN2v01 discriminant scores for all working points
- JVT (Jet Vertex Tagger) information

**Missing Transverse Energy (MET):**
- Branches: `met_NOSYS_met`, `met_NOSYS_phi`
- Components: `met_NOSYS_sumet` (scalar sum ET)
- Reconstruction: Rebuilt from calibrated objects

### Truth Information (MC only)
- Truth-level electrons, muons, jets
- Truth MET (from invisible particles)
- Hard-scatter process information
- Generator weights

### Systematic Variations
- When `do_CP_systematics: true`, each branch has systematic variations
- Naming: `object_SYSNAME_variable` (e.g., `el_EG_SCALE_UP_pt`)
- Available systematics: lepton energy scale/resolution, jet energy scale/resolution, b-tagging, MET

## Configuration Customization

Modify `share/RunConfig-LeptonsMET.yaml` for your specific needs:

### 1. Lepton Selection

```yaml
Electron:
  min_pT: 10.e+3     # Change threshold (MeV)
  ID: "TightLH"      # Options: LooseBLayerLH, MediumLH, TightLH
  Iso: "Tight_VarRad" # Options: NonIso, Loose_VarRad, Tight_VarRad

Muon:
  min_pT: 10.e+3     # Change threshold (MeV)
  ID: "Medium"       # Options: Loose, Medium, Tight
  Iso: "Tight_VarRad" # Options: NonIso, PflowLoose_VarRad, PflowTight_VarRad
```

### 2. Jet and B-tagging

```yaml
Small_R_jet:
  min_pT: 25.e+3    # Jet pT threshold (MeV)
  btag_wp: "FixedCutBEff_70"  # B-tagging working point
  btag_extra_wps: ["FixedCutBEff_85"]  # Additional WPs
```

### 3. Trigger Configuration

Edit `share/trigger.yaml` to add/remove triggers for specific years.

### 4. Systematic Uncertainties

```yaml
do_CP_systematics: true   # Enable systematic variations
```

**Warning**: Enabling systematics significantly increases output file size (factor of 5-10).

## Grid Submission

For large-scale production on the grid, see `GRID_SUBMISSION_GUIDE.md` for complete instructions.

Quick example:

```bash
# Setup grid credentials
voms-proxy-init -voms atlas -valid 168:00
lsetup panda

# Submit test job
cd ~/private/easyjet/easyjet/easyjet
easyjet-gridsubmit \
  --mc-list LeptonsMETAnalysis/datasets/mc_Zprime_1000GeV_test.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign ZprimeLepMET_test \
  --nFiles 2 \
  --noTag

# Monitor jobs
pbook
```

## Troubleshooting

### Issue: PHYSLITE Compatibility
**Symptom**: Missing jet variables or MET reconstruction failures
**Solution**: Use DAOD_PHYS format samples instead of PHYSLITE

PHYSLITE samples have pre-calibrated objects that bypass the EasyJet calibration workflow, causing incompatibilities. See `PHYSLITE_NOTES.md` for detailed technical explanation and requirements.

### Issue: MET Not in Output Tree
**Symptom**: Missing `met_NOSYS_*` branches in output
**Cause**: Configuration has `do_met: false`
**Solution**: Use `LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml` which has `do_met: true`

### Issue: Grid Certificate Problems
**Symptom**: `voms-proxy-init` fails with authentication errors
**Solution**: See `GRID_CERT_TROUBLESHOOTING.md` for complete diagnostics

Common fixes:
```bash
# Check certificate permissions
chmod 644 ~/.globus/usercert.pem
chmod 400 ~/.globus/userkey.pem

# Verify certificate validity
openssl x509 -in ~/.globus/usercert.pem -noout -dates
```

### Issue: Build Environment Conflicts
**Symptom**: Command not found, library errors, segmentation faults
**Cause**: Multiple ATLAS releases or builds sourced in same shell
**Solution**: Always use a fresh shell for each build (see "Solution for Future Use" section above)

## Dataset Information

### Z' Signal Samples (mc23, p6697)

DSIDs: 546587-546616
- Mass points: 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000 GeV
- Channels: ee and mumu
- Process: Z' → ℓℓ + neutrinos (off-shell)
- Simulation: MadGraph+Pythia8, EvtGen

Example dataset:
```
mc23_13p6TeV:mc23_13p6TeV.546597.MGPy8EG_lv_offsh_mZp_1000_ee.deriv.DAOD_PHYS.e8588_a934_r16083_p6697
```

Dataset lists available in `datasets/mc_Zprime_*.txt`

### Background Samples

Standard Model backgrounds for validation:
- Top quark: ttbar, single top
- Vector bosons: W+jets, Z+jets, diboson
- Multijet QCD (data-driven estimation recommended)

Lists available in `datasets/llmet_background_samples_PHYS_*.txt`

## Performance Considerations

### Output File Size
- Default configuration: ~1-2 KB per event
- With systematics enabled: ~5-10 KB per event
- 1M events: expect 1-2 GB (no systematics) or 5-10 GB (with systematics)

### Processing Speed
- Local processing: ~10-50 events/second (depends on configuration)
- Grid jobs: automatic splitting by file size (use `--nGBperJob 10`)

### Recommendations
- Test locally with 100-1000 events before grid submission
- Use `--nFiles 2-5` for grid tests before full production
- Enable systematics only for final production (not for validation)

## References and Documentation

**EasyJet Framework:**
- Main repository: https://gitlab.cern.ch/easyjet/easyjet
- Base configuration: `EasyjetHub/share/base-config.yaml`
- Container definitions: `EasyjetHub/share/container-names-DAOD_PHYS.yaml`

**ATLAS Recommendations:**
- Trigger: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TriggerRecommendations
- MET: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MissingEtRecommendations
- B-tagging: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTaggingRecommendations

**Analysis Documentation:**
- Package comparison: `PACKAGE_COMPARISON.md`
- Grid submission guide: `GRID_SUBMISSION_GUIDE.md`
- Grid certificate help: `GRID_CERT_TROUBLESHOOTING.md`

## Contact and Support

For questions about this analysis package:
- EXOT-2023-28 analysis team
- EasyJet framework: https://mattermost.web.cern.ch/easyjet/channels/easyjet-fw

For grid and technical issues:
- Grid support: atlas-adc-grid-support@cern.ch
- Framework issues: https://gitlab.cern.ch/easyjet/easyjet/-/issues
