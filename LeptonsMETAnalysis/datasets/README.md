# Dataset Lists for LeptonsMETAnalysis

This directory contains text files with input dataset names for grid submission.

## File Naming Convention

```
<type>_<campaign>_<tag>.txt
```

**Examples**:
- `data_Run3_p6026.txt` - Run 3 data with p6026 derivation
- `mc_Zprime_signal_p6026.txt` - Z' signal MC samples
- `mc_backgrounds_p6026.txt` - Background MC samples

## Format

One dataset name per line:
```
scope:dataset.name
```

Lines starting with `#` or `*` are ignored (use for comments).

## How to Find Dataset Names

### Using Rucio (Command Line)
```bash
# Search for Run 3 data with specific p-tag
rucio list-dids "data23_13p6TeV:*physics_Main*DAOD_PHYS*p6026*" --filter type=CONTAINER

# Search for MC samples
rucio list-dids "mc23_13p6TeV:*Zprime*DAOD_PHYS*p6026*" --filter type=CONTAINER
```

### Using AMI (Web Interface)
1. Go to https://ami.in2p3.fr/
2. Click "Dataset Browser"
3. Search by:
   - Project: `data23_13p6TeV` or `mc23_13p6TeV`
   - Data type: `DAOD_PHYS`
   - P-tag: `p6026`

### Ask Colleagues
Contact your analysis group for recommended dataset lists.

## Example Files

See `data_Run3_test.txt` and `mc_Zprime_test.txt` for template examples.

## Notes

- **DAOD Format**: Use `DAOD_PHYS` only (not PHYSLITE) for LeptonsMETAnalysis
- **P-tag**: Use consistent p-tag across all datasets (e.g., p6026)
- **Campaign**: Keep separate lists for different data-taking periods or MC campaigns
