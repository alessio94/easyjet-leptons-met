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

## Folder Structure

- `share/`: YAML configuration files
  - `RunConfig-LeptonsMET.yaml`: Main run configuration (enables MET, leptons, jets)
  - `trigger.yaml`: Lepton and MET trigger lists per data-taking year
  - `PHYSLITE_NOTES.md`: Documentation on DAOD_PHYS vs PHYSLITE formats

## Key Configuration Features

### Enabled Objects
✓ Electrons (min pT: 7 GeV)
✓ Muons (min pT: 7 GeV)
✓ Small-R jets with b-tagging
✓ **MET reconstruction** (enabled via `do_met: true`)
✓ Overlap removal

### Trigger Strategy
The trigger configuration combines:
- Single lepton triggers (e24, e26, e60, mu24, mu26, mu50)
- MET triggers (xe70, xe90, xe110, xe65_cell, etc.)

This ensures high efficiency for both:
- High-pT lepton events
- Events with large MET but moderate lepton pT

### B-tagging
- Nominal WP: GN2v01_FixedCutBEff_77
- Extra WPs: 70%, 85% efficiency

## How to Run

### 1. Setup Environment
```bash
cd /path/to/easyjet-leptons-met
source setup.sh  # or your setup script
```

### 2. Run on DAOD_PHYS (Recommended)
```bash
# Example command structure:
your-ntupler input_file.root \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --out-file output_leptonsMET.root
```

### 3. Data Format Requirements
⚠️ **Use DAOD_PHYS format only**
- ✓ DAOD_PHYS: Works correctly
- ✗ DAOD_PHYSLITE: Not compatible (see PHYSLITE_NOTES.md)

PHYSLITE has pre-calibrated objects which are incompatible with the EasyJet calibration workflow.

## Run 3 Data Taking Periods

The trigger configuration supports:
- **2022**: Early Run 3
- **2023**: Run 3
- **2024**: Run 3

Legacy Run 2 triggers (2015-2018) are also included for compatibility.

## Output

The output ROOT file contains an `AnalysisMiniTree` TTree with:

**Event-level:**
- Event weights and metadata
- Trigger decisions
- Pileup information

**Reconstructed objects:**
- Electrons: `el_NOSYS_X` (pT, eta, phi, E, charge, etc.)
- Muons: `mu_NOSYS_X` (pT, eta, phi, E, charge, etc.)
- Jets: `recojet_antikt4PFlow_NOSYS_X` (pT, eta, phi, E, b-tagging scores)
- MET: `met_NOSYS_X` (MET, MET_phi, and components)

**Truth information (MC only):**
- Truth electrons, muons, jets
- Truth MET
- Generator-level information

## Configuration Customization

To modify the configuration for your specific analysis:

1. **Adjust lepton pT thresholds**: Edit `Electron.min_pT` and `Muon.min_pT` in `RunConfig-LeptonsMET.yaml`

2. **Change b-tagging working points**: Modify `Small_R_jet.btag_wp` and `btag_extra_wps`

3. **Add/remove triggers**: Edit `share/trigger.yaml`

4. **Enable systematics**: Set `do_CP_systematics: true` in RunConfig (warning: increases output size)

## Known Issues and Solutions

### Issue: PHYSLITE compatibility
**Status**: Not supported
**Solution**: Request DAOD_PHYS format samples

See `PHYSLITE_NOTES.md` for detailed explanation.

### Issue: MET not in output
**Cause**: `do_met: false` in config
**Solution**: Use `RunConfig-LeptonsMET.yaml` which has `do_met: true`

## References

- EasyJet framework: https://gitlab.cern.ch/easyjet/easyjet
- Base configuration: `EasyjetHub/share/base-config.yaml`
- Container names: `EasyjetHub/share/container-names-DAOD_PHYS.yaml`

## Contact

For questions specific to this analysis package, contact the Z'+MET analysis team.
