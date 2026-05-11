# EasyJet Setup and Usage Guide

Complete guide for setting up and running physics analysis with the EasyJet framework.

## Prerequisites

- Access to CERN lxplus or an Alma9 computing environment
- ATLAS computing account with grid certificate
- Basic familiarity with ATLAS software and ROOT

## Initial Setup on lxplus

### 1. Environment Setup

```bash
# Setup ATLAS environment
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh

# Setup AthAnalysis release
asetup AthAnalysis,25.2.76
```

Add this to your `~/.bashrc` for convenience:

```bash
# ATLAS environment
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
alias setupATLAS='source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh'

# Quick setup alias for EasyJet
alias setup-easyjet='setupATLAS && asetup AthAnalysis,25.2.76 && cd ~/private/easyjet/easyjet/build && source x86_64-el9-gcc14-opt/setup.sh && cd ~/private/easyjet/easyjet/easyjet'
```

### 2. Clone the Repository

```bash
# Create working directory
mkdir -p ~/private/easyjet
cd ~/private/easyjet

# Clone the repository
setupATLAS
lsetup git
git clone --recursive https://gitlab.cern.ch/atlas-phys/exot/lpx/exot-2023-28/easyjet_leptonsmet.git easyjet
cd easyjet
```

For sparse checkout (specific packages only):

```bash
git clone --recursive --no-checkout https://gitlab.cern.ch/atlas-phys/exot/lpx/exot-2023-28/easyjet_leptonsmet.git easyjet
cd easyjet
git sparse-checkout init --cone
git sparse-checkout set EasyjetHub EasyjetTests LeptonsMETAnalysis
git checkout main
git submodule update --init --recursive
```

### 3. Build the Framework

```bash
# Create build directory
mkdir build
cd build

# Configure with cmake
cmake ../

# Build (use -j8 for 8 parallel jobs)
make -j8

# Source the build environment
source x86_64-el9-gcc14-opt/setup.sh

# Verify the build
which easyjet-ntupler
```

The build will take 5-10 minutes. If successful, you should see the executables in `build/x86_64-el9-gcc14-opt/bin/`.

## Directory Structure

After building, your directory structure will be:

```
~/private/easyjet/easyjet/
├── EasyjetHub/           # Core framework code
│   ├── share/            # Base configuration files
│   │   ├── base-config.yaml
│   │   ├── container-names-DAOD_PHYS.yaml
│   │   └── AnalysisMiniTree-config.yaml
│   ├── python/           # Python steering code
│   └── src/              # C++ framework algorithms
├── LeptonsMETAnalysis/   # Z' → leptons+MET analysis package
│   ├── share/            # Analysis configurations
│   │   ├── RunConfig-LeptonsMET.yaml
│   │   └── trigger.yaml
│   ├── datasets/         # Dataset lists for grid
│   └── scripts/          # Grid submission scripts
├── build/                # Build directory (created by you)
│   └── x86_64-el9-gcc14-opt/
│       ├── bin/          # Compiled executables
│       ├── lib/          # Libraries
│       └── setup.sh      # Environment setup script
└── README.md             # This file
```

## Running Your First Analysis

### Step 1: Get a Test File

Download a small test file:

```bash
cd ~/private/easyjet/easyjet
mkdir run
cd run

# Get ttbar DAOD_PHYS test file (10 events)
curl -L -s -o test_PHYS.root \
  https://gitlab.cern.ch/easyjet/hh4b-test-files/-/raw/p6266/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.DAOD_PHYS_10evts.e8514_s4162_r14622_p6266.pool.root
```

### Step 2: Run the Ntupler

```bash
# Run on 100 events
easyjet-ntupler test_PHYS.root \
  --run-config ../LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --out-file output.root \
  -e 100
```

Command options:
- `test_PHYS.root`: Input DAOD file
- `--run-config`: Path to YAML configuration
- `--out-file`: Output ROOT file name
- `-e 100`: Process 100 events (-1 for all)

### Step 3: Check the Output

```bash
# Quick check with ROOT
root -l output.root

# In ROOT prompt:
root [0] .ls                    // List objects in file
root [1] AnalysisMiniTree->Print()   // Show tree structure
root [2] AnalysisMiniTree->Scan("el_NOSYS_pt:mu_NOSYS_pt:met_NOSYS_met", "", "", 10)
root [3] .q
```

Expected output branches:
- `el_NOSYS_pt`, `el_NOSYS_eta`, `el_NOSYS_phi`: Electrons
- `mu_NOSYS_pt`, `mu_NOSYS_eta`, `mu_NOSYS_phi`: Muons
- `recojet_antikt4PFlow_NOSYS_pt`: Jets
- `met_NOSYS_met`, `met_NOSYS_phi`: Missing ET

## Configuration Files Explained

### Main Run Configuration

`LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml` configures:

```yaml
# Enable object types
do_electrons: true
do_muons: true
do_small_R_jets: true
do_met: true              # CRITICAL: Must be true for MET

# Electron selection
Electron:
  min_pT: 7.e+3           # 7 GeV threshold
  ID: "LooseBLayerLH"
  Iso: "NonIso"

# Muon selection
Muon:
  min_pT: 7.e+3           # 7 GeV threshold
  ID: "Loose"
  Iso: "PflowLoose_VarRad"

# Trigger configuration
Trigger:
  include: [trigger.yaml]
```

### Trigger Configuration

`LeptonsMETAnalysis/share/trigger.yaml` lists triggers for each data-taking year:

- Single lepton: e24, e26, e60, mu24, mu26, mu50
- MET triggers: xe70, xe90, xe110 (Run 2), xe65_cell (Run 3)
- Scale factors: `doSF: false` (loose leptons lack SF files)

## Data Formats

### DAOD_PHYS (Recommended)

Use DAOD_PHYS for all analyses with this framework:

```bash
# Correct format
easyjet-ntupler input_DAOD_PHYS.root --run-config ...
```

Contains raw containers that EasyJet calibrates:
- Jets: `AntiKt4EMPFlowJets`
- MET: Reconstructed from jets

### DAOD_PHYSLITE (Not Supported)

PHYSLITE has pre-calibrated objects incompatible with EasyJet's calibration workflow. See `LeptonsMETAnalysis/PHYSLITE_NOTES.md` for details.

## Working with Grid Jobs

For processing large datasets, use grid submission. See `LeptonsMETAnalysis/GRID_SUBMISSION_GUIDE.md` for complete instructions.

Quick summary:

### Setup Grid Tools

```bash
# Valid grid certificate
voms-proxy-init -voms atlas -valid 168:00

# Setup panda
lsetup panda

# Setup rucio (data management)
lsetup rucio
```

### Submit Test Job

```bash
cd ~/private/easyjet/easyjet/easyjet

easyjet-gridsubmit \
  --mc-list LeptonsMETAnalysis/datasets/mc_Zprime_1000GeV_test.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign ZprimeLepMET_test \
  --nFiles 2 \
  --noTag
```

### Monitor Jobs

```bash
# Command line
pbook
pbook> show()

# Web: https://bigpanda.cern.ch/
```

## Common Workflows

### Adding a New Analysis Variable

1. Edit `LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml`
2. Add variable to the appropriate section (Electron, Muon, etc.)
3. Rebuild: `cd build && make -j8 && source */setup.sh`
4. Test locally before grid submission

### Changing Lepton Selection

Edit `RunConfig-LeptonsMET.yaml`:

```yaml
Electron:
  min_pT: 10.e+3      # Change threshold
  ID: "TightLH"       # Change ID working point
  Iso: "Tight_VarRad" # Change isolation
```

Rebuild and test. Note: Changing to tighter working points may enable trigger scale factors.

### Running on Data vs MC

The framework automatically detects data/MC from file metadata. No configuration changes needed.

Data files typically named:
```
data23_13p6TeV.00450975.physics_Main.deriv.DAOD_PHYS...
```

MC files typically named:
```
mc23_13p6TeV.601229.PhPy8EG_ttbar.deriv.DAOD_PHYS...
```

## Troubleshooting

### "easyjet-ntupler: command not found"

**Solution**: Source the build environment:
```bash
cd ~/private/easyjet/easyjet/build
source x86_64-el9-gcc14-opt/setup.sh
```

### "Could not find config file"

**Solution**: Check you're in the correct directory:
```bash
cd ~/private/easyjet/easyjet/easyjet  # Note: extra /easyjet at end
ls LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml  # Should exist
```

### "MET not in output tree"

**Solution**: Verify `do_met: true` in your run config:
```bash
grep "do_met" LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml
```

### Build Errors

**Common causes**:
1. Wrong AthAnalysis version: Use `asetup AthAnalysis,25.2.76`
2. Missing environment: Run `setupATLAS` first
3. Stale build: `cd build && rm -rf * && cmake ../ && make -j8`

### Grid Certificate Issues

See `LeptonsMETAnalysis/GRID_CERT_TROUBLESHOOTING.md` for complete diagnostics.

Quick check:
```bash
voms-proxy-info | grep timeleft
# Should show hours remaining
```

## Performance Tips

### Local Processing

- Use `-j8` or `-j16` for parallel builds
- Process 100-1000 events locally for testing
- Save full processing for grid

### Grid Submission

- Test with 2-5 files before full production
- Use `--nGBperJob 10` for automatic splitting
- Monitor first few jobs before submitting thousands

### Output File Size

Default configuration produces ~1-2 KB per event. For 1M events, expect ~1-2 GB output.

To reduce:
- Disable systematic variations: `do_CP_systematics: false`
- Limit saved variables in `ttree_output` section
- Use event skimming/preselection

## Next Steps

1. **Test locally**: Run on 100-1000 events
2. **Validate output**: Check all expected branches exist
3. **Small grid test**: 2-5 files with `--noTag`
4. **Full production**: After validating test outputs
5. **Analysis**: ROOT, Python, or framework of choice

## Additional Documentation

Analysis-specific guides in `LeptonsMETAnalysis/`:
- `README.md`: Package overview
- `GRID_SUBMISSION_GUIDE.md`: Complete grid workflow (400+ lines)
- `GRID_CERT_TROUBLESHOOTING.md`: Certificate diagnostics
- `PHYSLITE_NOTES.md`: DAOD format compatibility

Framework documentation:
- `EasyjetHub/README.md`: Core framework details
- Base configs: `EasyjetHub/share/*.yaml`

## Getting Help

1. **EasyJet Mattermost**: https://mattermost.web.cern.ch/easyjet/channels/easyjet-fw
2. **Analysis team**: EXOT-2023-28 mailing list
3. **Grid support**: atlas-adc-grid-support@cern.ch
4. **Framework issues**: https://gitlab.cern.ch/easyjet/easyjet/-/issues

## Quick Reference

```bash
# Complete setup from scratch
setupATLAS
asetup AthAnalysis,25.2.76
cd ~/private/easyjet/easyjet/build
source x86_64-el9-gcc14-opt/setup.sh
cd ~/private/easyjet/easyjet/easyjet

# Run local test
easyjet-ntupler test.root \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --out-file output.root \
  -e 100

# Grid setup
voms-proxy-init -voms atlas -valid 168:00
lsetup panda
lsetup rucio

# Submit grid job
easyjet-gridsubmit \
  --mc-list datasets/mylist.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign MyCampaign_v01
```
