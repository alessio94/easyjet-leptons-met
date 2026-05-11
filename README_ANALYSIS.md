# Z' → Leptons + MET Analysis using EasyJet

ATLAS EXOT-2023-28 analysis searching for Z' bosons decaying to leptons and missing transverse energy.

## Quick Start

### Prerequisites

- CERN lxplus or Alma9 terminal access
- ATLAS software access (AthAnalysis 25.2.76)
- Valid grid certificate for production jobs

### Setup and Build

```bash
# On lxplus
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh

# Setup AthAnalysis
asetup AthAnalysis,25.2.76

# Clone this repository
git clone https://gitlab.cern.ch/atlas-phys/exot/lpx/exot-2023-28/easyjet_leptonsmet.git
cd easyjet_leptonsmet

# Build
mkdir build
cd build
cmake ../
make -j8
source x86_64-el9-gcc14-opt/setup.sh
```

### Running Locally

Test your configuration on a small number of events:

```bash
cd ../
mkdir run
cd run

# Download a test file (ttbar DAOD_PHYS)
curl -L -s -o test_PHYS.root https://gitlab.cern.ch/easyjet/hh4b-test-files/-/raw/p6266/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.DAOD_PHYS_10evts.e8514_s4162_r14622_p6266.pool.root

# Run the ntupler
easyjet-ntupler test_PHYS.root \
  --run-config ../LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --out-file output.root \
  -e 100
```

Check the output contains electrons, muons, jets, and MET:

```bash
root -l output.root
root [0] AnalysisMiniTree->Print()
root [1] AnalysisMiniTree->Scan("el_NOSYS_pt:mu_NOSYS_pt:met_NOSYS_met", "", "", 10)
```

## Analysis Configuration

### LeptonsMETAnalysis Package

Located in `LeptonsMETAnalysis/`, this package contains:

- **share/RunConfig-LeptonsMET.yaml**: Main analysis configuration
  - Electrons: 7 GeV pT threshold, LooseBLayerLH ID, NonIso
  - Muons: 7 GeV pT threshold, Loose ID, PflowLoose_VarRad isolation
  - Jets: 20 GeV pT threshold, PFlow reconstruction
  - MET: Fully enabled with all components

- **share/trigger.yaml**: Trigger lists for Run 2 and Run 3
  - Single lepton triggers (e24, e26, e60, mu24, mu26, mu50)
  - MET triggers (xe70, xe90, xe110, xe65_cell)
  - Note: Trigger scale factors disabled (`doSF: false`) due to loose lepton working points

### Important Notes

1. **Use DAOD_PHYS format only**. PHYSLITE is incompatible with this analysis due to jet variable requirements.

2. **Trigger scale factors**: Currently disabled because loose lepton working points lack CP-provided scale factor files. For high-MET regions (>200 GeV), trigger efficiency approaches 100% and scale factors are approximately 1.0, making this acceptable for initial production.

3. **p-tag**: Use p6697 for Run 3 MC samples.

## Grid Submission

Complete documentation is available in `LeptonsMETAnalysis/GRID_SUBMISSION_GUIDE.md`. Quick summary:

### Prerequisites

```bash
# Valid grid proxy (7 days)
voms-proxy-init -voms atlas -valid 168:00

# Setup panda
lsetup panda
```

### Submit Test Jobs

```bash
# Test with 2 files
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

# Web interface
https://bigpanda.cern.ch/
```

### Download Outputs

```bash
rucio download "user.<your-nickname>.ZprimeLepMET_test.*"
```

## Signal Samples

Z' → leptons + neutrino off-shell samples (mc23, p6697):

- DSIDs: 546587-546616
- Mass points: 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000 GeV
- Channels: ee and mumu
- Simulation tags available: a910, a911, a934 (use a934 for latest)

Example dataset:
```
mc23_13p6TeV:mc23_13p6TeV.546597.MGPy8EG_lv_offsh_mZp_1000_ee.deriv.DAOD_PHYS.e8588_a934_r16083_p6697
```

Full dataset lists are in `LeptonsMETAnalysis/datasets/`.

## Documentation

- `LeptonsMETAnalysis/README.md`: Package overview and physics motivation
- `LeptonsMETAnalysis/GRID_SUBMISSION_GUIDE.md`: Complete grid workflow (400+ lines)
- `LeptonsMETAnalysis/GRID_CERT_TROUBLESHOOTING.md`: Certificate issues and solutions
- `LeptonsMETAnalysis/PACKAGE_COMPARISON.md`: Technical comparison with other packages
- `LeptonsMETAnalysis/PHYSLITE_NOTES.md`: Why PHYSLITE doesn't work and the solution

## Troubleshooting

### "easyjet-ntupler: command not found"

Source the build environment:
```bash
cd build
source x86_64-el9-gcc14-opt/setup.sh
```

### "PHYSLITE incompatible" error

Use DAOD_PHYS format. See `LeptonsMETAnalysis/PHYSLITE_NOTES.md` for details.

### Grid certificate issues

Run the diagnostics in `LeptonsMETAnalysis/GRID_CERT_TROUBLESHOOTING.md`.

### No MET in output

Verify you're using `LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml` which has `do_met: true`.

## Production Workflow

1. **Test locally** with 100 events on a test file
2. **Submit small grid test** (2-5 files) with `--noTag`
3. **Monitor test jobs** until finished
4. **Download and validate** test outputs
5. **Submit full production** with proper campaign name
6. **Document** output datasets and processing version

For production, coordinate with the analysis team and use versioned campaign names (e.g., `ZprimeLepMET_v01`).

## Contact

- Analysis team: EXOT-2023-28 mailing list
- EasyJet framework support: [easyjet-fw Mattermost](https://mattermost.web.cern.ch/easyjet/channels/easyjet-fw)
- Grid issues: atlas-adc-grid-support@cern.ch

## References

- [EasyJet framework documentation](https://gitlab.cern.ch/easyjet/easyjet)
- [ATLAS Run 3 analysis documentation](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/Run3Analysis)
- [Trigger recommendations](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TriggerRecommendations)
- [MET recommendations](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MissingEtRecommendations)
