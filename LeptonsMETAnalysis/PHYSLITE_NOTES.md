# PHYSLITE Incompatibility Notes

## Issue Summary

PHYSLITE format fails with "incompatible jet variable config" error when running the EasyJet framework.

## Root Cause

The fundamental issue is that **PHYSLITE and DAOD_PHYS have different container naming conventions**:

### DAOD_PHYS (Works ✓)
- Contains **raw/uncalibrated** objects that need processing
- Jet container: `AntiKt4EMPFlowJets`
- MET container: `AntiKt4EMPFlow`
- Electron container: `Electrons`
- Muon container: `Muons`

### DAOD_PHYSLITE (Fails ✗)
- Contains **pre-calibrated/analyzed** objects
- Jet container: `AnalysisJets` (already calibrated)
- MET container: `AnalysisMET` (already calibrated)
- Electron container: `AnalysisElectrons` (already calibrated)
- Muon container: `AnalysisMuons` (already calibrated)

## Why It Fails

The EasyJet framework expects to:
1. Read raw containers from DAOD_PHYS
2. Apply calibrations and selections
3. Produce "Analysis*" output containers

However, PHYSLITE **already provides pre-calibrated "Analysis*" containers**, which causes:
- Missing expected raw container variables
- Incompatible jet variable configurations (raw vs calibrated)
- Framework attempting to re-calibrate already-calibrated objects

## Container Name Mapping

The framework uses different container name configs:

**File: `EasyjetHub/share/container-names-DAOD_PHYS.yaml`**
```yaml
input:
  reco4PFlowJet: AntiKt4EMPFlowJets
  met: AntiKt4EMPFlow
  muons: Muons
  electrons: Electrons
```

**File: `EasyjetHub/share/container-names-DAOD_PHYSLITE.yaml`**
```yaml
input:
  reco4PFlowJet: AnalysisJets
  met: AnalysisMET
  muons: AnalysisMuons
  electrons: AnalysisElectrons
```

## Solutions

### Short-term: Use DAOD_PHYS ✓ (Recommended)
- **Status**: Working
- Request DAOD_PHYS format samples from production
- This is the standard format for EasyJet framework
- Full variable access and customization

### Long-term: PHYSLITE Support (Future Work)
To support PHYSLITE, the framework would need:
1. Bypass calibration steps for pre-calibrated objects
2. Adjust variable accessor to handle different naming schemes
3. Handle reduced variable set in PHYSLITE
4. Special configuration flag: `use_physlite: true`

Example potential solution:
```yaml
# Hypothetical PHYSLITE support
disable_calib: true  # Skip calibration for pre-calibrated objects
use_physlite_containers: true
```

## Current Workaround

For Run 3 Z'+MET analysis:
- ✓ Use **DAOD_PHYS** format (requested pre-summer 2025)
- ✓ Works with current framework
- ✓ Full jet variable access
- ✓ MET properly configured

## References

- Base config: `EasyjetHub/share/base-config.yaml`
- PHYS containers: `EasyjetHub/share/container-names-DAOD_PHYS.yaml`
- PHYSLITE containers: `EasyjetHub/share/container-names-DAOD_PHYSLITE.yaml`
- MET is enabled in: `LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml`
