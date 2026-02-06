# Grid Submission Guide for LeptonsMETAnalysis

**Date**: 2026-02-06
**Analysis**: Z' → Leptons + MET
**Package**: LeptonsMETAnalysis

---

## Prerequisites

### 1. Grid Credentials
You must have valid grid credentials before submitting:

```bash
# Check your VOMS proxy status
voms-proxy-info --all

# If expired or missing, create a new proxy (valid for 168 hours = 7 days)
voms-proxy-init -voms atlas -valid 168:00
```

**Expected output**:
```
subject   : /DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=<your name>/CN=<ID>/CN=<proxy>
issuer    : /DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=<your name>/CN=<ID>
...
timeleft  : 167:59:59
```

### 2. Panda Setup
The grid uses PanDA (Production and Distributed Analysis):

```bash
# Setup panda
lsetup panda

# Verify prun is available
which prun
```

### 3. ATLAS Environment
Ensure you're in your AthAnalysis environment:

```bash
# Should see AthAnalysis, 25.2.76
asetup --restore
echo $AtlasProject $AtlasVersion
```

---

## Data List Format

The grid submission script requires a text file listing the input datasets, one per line.

### Format Rules
- **One dataset per line**
- Lines starting with `#` or `*` are ignored (comments)
- Empty lines are ignored
- Format: `scope:dataset_name` or just `dataset_name`

### Example Data List (`data_Run3_p6026.txt`)
```
# Run 3 data, p6026 derivation
data22_13p6TeV:data22_13p6TeV.00428353.physics_Main.deriv.DAOD_PHYS.r14622_p6026
data22_13p6TeV:data22_13p6TeV.00428354.physics_Main.deriv.DAOD_PHYS.r14622_p6026
data23_13p6TeV:data23_13p6TeV.00450975.physics_Main.deriv.DAOD_PHYS.r15116_p6026
data24_13p6TeV:data24_13p6TeV.00473033.physics_Main.deriv.DAOD_PHYS.r15562_p6026
```

### Example MC List (`mc_Zprime_signal.txt`)
```
# Z' signal samples, mc23a
mc23_13p6TeV:mc23_13p6TeV.700001.Sh_2210_Zprime_ee_mZp1000.deriv.DAOD_PHYS.e8514_s4162_r14622_p6026
mc23_13p6TeV:mc23_13p6TeV.700002.Sh_2210_Zprime_mumu_mZp1000.deriv.DAOD_PHYS.e8514_s4162_r14622_p6026
mc23_13p6TeV:mc23_13p6TeV.700003.Sh_2210_Zprime_ee_mZp2000.deriv.DAOD_PHYS.e8514_s4162_r14622_p6026
mc23_13p6TeV:mc23_13p6TeV.700004.Sh_2210_Zprime_mumu_mZp2000.deriv.DAOD_PHYS.e8514_s4162_r14622_p6026
```

### How to Find Dataset Names

1. **Using Rucio** (recommended):
```bash
# Search for datasets
rucio list-dids "data23_13p6TeV:*physics_Main*DAOD_PHYS*p6026*" --filter type=CONTAINER

# Get detailed information about a specific dataset
rucio list-content data23_13p6TeV:data23_13p6TeV.00450975.physics_Main.deriv.DAOD_PHYS.r15116_p6026
```

2. **Using AMI** (ATLAS Metadata Interface):
   - Web interface: https://ami.in2p3.fr/
   - Search for derivations by run number, p-tag, etc.

3. **Ask colleagues** for recommended dataset lists for your analysis

---

## Grid Submission Command

### Basic Syntax

```bash
easyjet-gridsubmit \
  --data-list <file.txt> \       # For data (OR --mc-list for MC)
  --run-config <config.yaml> \   # Your run configuration
  --campaign <name> \            # Campaign identifier
  [options]
```

### Required Arguments

| Argument | Description | Example |
|----------|-------------|---------|
| `--data-list` | Text file with data dataset names | `data_Run3.txt` |
| `--mc-list` | Text file with MC dataset names | `mc_signal.txt` |
| `--run-config` | Path to run configuration | `LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml` |

**Note**: Use either `--data-list` OR `--mc-list`, not both in same command.

### Important Optional Arguments

| Argument | Description | Default | Recommended |
|----------|-------------|---------|-------------|
| `--campaign` | Campaign name for output datasets | `EJ_<timestamp>` | `ZprimeLepMET_v01` |
| `--exec` | Executable name | `easyjet-ntupler` | `easyjet-ntupler` |
| `--nGBperJob` | GB per grid job | Auto | `5` (smaller for testing) |
| `--nFilesPerJob` | Files per job | Auto | `1-2` (for testing) |
| `--nFiles` | Total files to process | All | `5` (for testing) |
| `--memory` | Memory per core (MB) | 2000 | `4000` (if jobs fail) |
| `--noTag` | Skip git tag creation | False | Use for testing |
| `--noSubmit` | Create tarball only, don't submit | False | Use for dry-run |
| `--mergeOutput` | Merge output files | False | `--mergeOutput` |
| `--dest-se` | Destination storage element | None | Your local SE |

---

## Step-by-Step Submission Process

### Step 1: Test Locally First

Before submitting to the grid, **always** test your configuration locally:

```bash
cd ~/private/easyjet/easyjet/easyjet

# Test with a local DAOD_PHYS file
easyjet-ntupler \
  -c LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  -e 100 \
  -o test_output.root \
  ~/private/DAOD_PHYS.45529054._000001.pool.root.1
```

**Verify**:
- ✅ Job completes without errors
- ✅ Output file contains expected branches (electrons, muons, jets, MET)
- ✅ Events pass trigger filtering

### Step 2: Create Your Data List

Create a text file with the datasets you want to process:

```bash
# Example: Create data list for Run 3
cat > LeptonsMETAnalysis/datasets/data_Run3_test.txt << 'EOF'
# Run 3 data test - 2 periods
data23_13p6TeV:data23_13p6TeV.00450975.physics_Main.deriv.DAOD_PHYS.r15116_p6026
data23_13p6TeV:data23_13p6TeV.00450976.physics_Main.deriv.DAOD_PHYS.r15116_p6026
EOF
```

### Step 3: Setup Grid Environment

```bash
# Check grid credentials (must be valid)
voms-proxy-info | grep timeleft

# If expired, renew
voms-proxy-init -voms atlas -valid 168:00

# Setup panda
lsetup panda
```

### Step 4: Dry-Run Submission (Recommended)

Test the submission command without actually submitting jobs:

```bash
cd ~/private/easyjet/easyjet/easyjet

easyjet-gridsubmit \
  --data-list LeptonsMETAnalysis/datasets/data_Run3_test.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign ZprimeLepMET_test \
  --noTag \
  --noSubmit
```

**What this does**:
- ✅ Creates the tarball (`code.tar.gz`)
- ✅ Validates dataset names
- ✅ Shows the `prun` commands that would be executed
- ❌ Does NOT submit jobs to the grid

**Expected output**:
```
prun --inDS data23_13p6TeV:data23_13p6TeV.00450975.physics_Main.deriv.DAOD_PHYS.r15116_p6026 \
     --outDS user.<your-nickname>.ZprimeLepMET_test.data23_13p6TeV.00450975.p6026 \
     --exec "easyjet-ntupler %IN -l --run-config config.yaml --out-file output-tree.root" \
     ...
```

### Step 5: Submit Test Jobs

Submit a small test with limited files:

```bash
easyjet-gridsubmit \
  --data-list LeptonsMETAnalysis/datasets/data_Run3_test.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign ZprimeLepMET_test_v01 \
  --nFiles 2 \
  --nFilesPerJob 1 \
  --nGBperJob 5 \
  --noTag
```

**What this does**:
- Processes only 2 files total
- 1 file per job (creates 2 jobs)
- Each job requests 5 GB
- Skips git tag creation (testing mode)

### Step 6: Monitor Jobs

```bash
# List your jobs
pbook

# Inside pbook, check job status:
pbook> show()

# Check specific job details (replace 123 with your job ID)
pbook> show(123)

# Kill a job if needed
pbook> kill(123)

# Retry failed jobs
pbook> retry(123)

# Exit pbook
pbook> exit
```

**Job states**:
- `running`: Jobs are processing
- `finished`: All jobs completed successfully
- `failed`: Some jobs failed
- `cancelled`: Jobs were killed

### Step 7: Download Outputs

Once jobs finish, download the output files:

```bash
# Download all outputs from a job
rucio download "user.<nickname>.ZprimeLepMET_test_v01.*"

# Download specific dataset
rucio download user.<nickname>.ZprimeLepMET_test_v01.data23_13p6TeV.00450975.p6026
```

---

## Full Production Submission

Once tests succeed, submit the full production:

### Example: Run 3 Data Production

```bash
easyjet-gridsubmit \
  --data-list LeptonsMETAnalysis/datasets/data_Run3_full.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign ZprimeLepMET_v01 \
  --nGBperJob 10 \
  --memory 4000 \
  --mergeOutput \
  --dest-se DESY-ZN_LOCALGROUPDISK
```

**Notes**:
- `--campaign ZprimeLepMET_v01`: Official campaign name (will create git tag)
- `--nGBperJob 10`: Process ~10 GB per job (auto-splits datasets)
- `--memory 4000`: Request 4000 MB per core (increase if jobs run out of memory)
- `--mergeOutput`: Merge output files per dataset automatically
- `--dest-se`: Copy outputs to your group storage element

### Example: MC Signal Production

```bash
easyjet-gridsubmit \
  --mc-list LeptonsMETAnalysis/datasets/mc_Zprime_signal.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign ZprimeLepMET_v01 \
  --nGBperJob 10 \
  --memory 4000 \
  --mergeOutput
```

---

## Creating a Grid Submission Script

For reproducibility, create a shell script:

**`LeptonsMETAnalysis/scripts/submit_Run3_data.sh`**:
```bash
#!/bin/bash

# Grid submission script for Z' → Leptons + MET analysis
# Run 3 data, p6026 derivation

runConfig="LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml"
executable="easyjet-ntupler"
campaignName="ZprimeLepMET_v01"

# Data lists
data_dir="LeptonsMETAnalysis/datasets"
data_list="${data_dir}/data_Run3_p6026.txt"

# Check prerequisites
if ! voms-proxy-info --exists --valid 24:00 2>/dev/null; then
    echo "ERROR: No valid grid proxy found. Run: voms-proxy-init -voms atlas -valid 168:00"
    exit 1
fi

if ! which prun &>/dev/null; then
    echo "ERROR: prun not found. Run: lsetup panda"
    exit 1
fi

# Submit data
echo "Submitting Run 3 data..."
easyjet-gridsubmit \
    --data-list ${data_list} \
    --run-config ${runConfig} \
    --exec ${executable} \
    --campaign ${campaignName} \
    --nGBperJob 10 \
    --memory 4000 \
    --mergeOutput \
    --dest-se DESY-ZN_LOCALGROUPDISK

echo "Done! Monitor jobs with: pbook"
```

**Make it executable**:
```bash
chmod +x LeptonsMETAnalysis/scripts/submit_Run3_data.sh
```

**Run it**:
```bash
cd ~/private/easyjet/easyjet/easyjet
./LeptonsMETAnalysis/scripts/submit_Run3_data.sh
```

---

## Troubleshooting

### Problem: "Failed to find easyjet-ntupler"

**Solution**: Make sure you've sourced the build environment:
```bash
cd ~/private/easyjet/easyjet/build
source x86_64-el9-gcc14-opt/setup.sh
which easyjet-ntupler  # Should return a path
```

### Problem: "You have not set up panda"

**Solution**: Setup panda before submitting:
```bash
lsetup panda
```

### Problem: "Failed to create tarball"

**Solution**: This usually means you're not in the right directory or the build isn't complete. Make sure:
```bash
cd ~/private/easyjet/easyjet/easyjet  # Must be in source directory
ls -la config.yaml  # Should exist temporarily during submission
```

### Problem: Jobs fail with "Memory exceeded"

**Solution**: Increase memory per job:
```bash
--memory 6000  # Request 6 GB per core
```

### Problem: Jobs stuck in "pending" state

**Cause**: Grid site is busy or dataset is not available at any active site.

**Solution**:
```bash
# Check dataset availability
rucio list-dataset-replicas <dataset-name>

# Exclude problematic sites
easyjet-gridsubmit ... --excluded-site ANALY_SITE_NAME
```

### Problem: "Dataset not found"

**Solution**: Verify dataset name with rucio:
```bash
rucio list-dids "scope:partial_name*"
```

### Problem: Output datasets not visible

**Wait**: Grid jobs take time. Check after ~30 minutes.

**Monitor**:
```bash
# Use pbook to see job progress
pbook
pbook> show()
```

---

## Best Practices

### 1. **Always Test Locally First**
- Run on at least 100 events locally
- Verify output before grid submission

### 2. **Start Small**
- Use `--nFiles 5` for first test
- Use `--noTag` during testing
- Monitor 1-2 test jobs before full submission

### 3. **Use Meaningful Campaign Names**
```bash
# Good: Descriptive, versioned
ZprimeLepMET_v01
ZprimeLepMET_Run3_data_v02

# Bad: Generic, unclear
test123
my_analysis
```

### 4. **Check Grid Quotas**
```bash
# Check your user quota
rucio list-account-usage <your-nickname>
```

### 5. **Merge Outputs**
- Use `--mergeOutput` for production
- Easier to download and process

### 6. **Document Your Submission**
- Save the exact command used
- Note the campaign name and datasets
- Keep track of job IDs

---

## Output Dataset Naming

The script automatically creates output dataset names:

**Format**:
```
user.<nickname>.<campaign>.<dsid>.<p-tag>
```

**Example**:
```
Input:  data23_13p6TeV:data23_13p6TeV.00450975.physics_Main.deriv.DAOD_PHYS.r15116_p6026
Output: user.johndoe.ZprimeLepMET_v01.data23_13p6TeV.00450975.p6026
```

**For MC**:
```
Input:  mc23_13p6TeV:mc23_13p6TeV.700001.Sh_2210_Zprime_ee_mZp1000.deriv.DAOD_PHYS.e8514_s4162_r14622_p6026
Output: user.johndoe.ZprimeLepMET_v01.700001.p6026
```

---

## Next Steps After Grid Submission

1. **Monitor jobs** until all finish (can take hours to days)
2. **Download outputs** using rucio
3. **Merge outputs** if needed (or use `--mergeOutput`)
4. **Validate outputs**: Check event counts, branch structure
5. **Run post-processing** (if applicable)
6. **Analyze ntuples**: Histograms, fits, signal extraction

---

## Quick Reference Card

```bash
# 1. Check prerequisites
voms-proxy-info | grep timeleft    # Should show > 24 hours
lsetup panda                        # Setup panda
which easyjet-ntupler               # Should return path

# 2. Test locally (always!)
easyjet-ntupler -c LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  -e 100 -o test.root input.root

# 3. Dry-run grid submission
easyjet-gridsubmit --data-list mydata.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign TestCampaign --noTag --noSubmit

# 4. Submit test jobs (2 files)
easyjet-gridsubmit --data-list mydata.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign ZprimeLepMET_test --nFiles 2 --noTag

# 5. Monitor jobs
pbook
pbook> show()

# 6. Download outputs
rucio download "user.<nickname>.ZprimeLepMET_test.*"

# 7. Full production
easyjet-gridsubmit --data-list full_data.txt \
  --run-config LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml \
  --campaign ZprimeLepMET_v01 --nGBperJob 10 --mergeOutput
```

---

## Summary

You now have everything needed to submit your LeptonsMETAnalysis to the grid:

- ✅ Understanding of grid submission workflow
- ✅ Data list format and examples
- ✅ Complete command-line options
- ✅ Step-by-step testing procedure
- ✅ Production submission examples
- ✅ Troubleshooting guide

**Remember**: Test locally → Dry-run → Small test → Monitor → Full production

Good luck with your grid submission! 🚀
