# EasyJet Package Comparison: Trigger & MET Configuration

**Purpose**: Compare LeptonsMETAnalysis with multileptonAnalysis and bbllAnalysis to identify configuration differences that may affect DAOD_PHYS/PHYSLITE compatibility.

**Date**: 2026-02-06

---

## Summary of Key Differences

| Feature | LeptonsMETAnalysis | multileptonAnalysis | bbllAnalysis |
|---------|-------------------|---------------------|--------------|
| **Trigger SFs** | `doSF: false` | `doSF: true` | `doSF: true` |
| **Electron ID** | LooseBLayerLH | LooseBLayerLH | MediumLH |
| **Electron Iso** | NonIso | Loose_VarRad | Loose_VarRad |
| **Muon ID** | Loose | Loose | Medium |
| **Muon Iso** | PflowLoose_VarRad | Loose_VarRad | PflowLoose_VarRad |
| **MET Triggers** | ✅ Yes (xe70, xe90, etc.) | ❌ No | ❌ No |
| **MET Enabled** | `do_met: true` | `do_met: true` | `do_met: true` |

---

## 1. Trigger Scale Factor Configuration

### LeptonsMETAnalysis (`LeptonsMETAnalysis/share/trigger.yaml`)
```yaml
scale_factor:
  doSF: false  # Disabled - loose leptons don't have SF files
```

**Why disabled**: LooseBLayerLH + NonIso electrons and Loose + PflowLoose_VarRad muons don't have trigger SF files from ATLAS CP groups.

### multileptonAnalysis (`multileptonAnalysis/share/trigger.yaml:227-276`)
```yaml
scale_factor:
  doSF: true
  chains:
    '2015':
    - 'HLT_e24_lhmedium_L1EM20VH || HLT_e60_lhmedium || HLT_e120_lhloose'
    - 'HLT_mu20_iloose_L1MU15 || HLT_mu50'
    ...
  Electron:
    ID_Run2: "MediumLH"
    Iso_Run2: "FCLoose"
    ID_Run3: "MediumLH"
    Iso_Run3: "FCLoose"
  Muon:
    ID: "Medium"
```

**Why enabled**: MediumLH + FCLoose electrons and Medium muons are standard working points with available SF files.

### bbllAnalysis (`bbllAnalysis/share/trigger.yaml:93-164`)
```yaml
scale_factor:
  doSF: true
  chains:
    '2015':
    - 'HLT_e24_lhmedium_L1EM20VH || HLT_e60_lhmedium || HLT_e120_lhloose'
    - 'HLT_mu20_iloose_L1MU15 || HLT_mu40'
    ...
  # Note: Specified in RunConfig-bbll.yaml:69-76
  Electron:
    ID_Run2: "TightLH"
    Iso_Run2: "FCTight"
    ID_Run3: "Tight"
    Iso_Run3: "Tight_VarRad"
  Muon:
    ID: "Medium"
```

**Why enabled**: TightLH + FCTight electrons and Medium muons are standard working points with available SF files.

---

## 2. MET Trigger Strategy

### LeptonsMETAnalysis
**Includes MET triggers** for both Run 2 and Run 3:

**Run 2 MET Triggers**:
- 2015: `HLT_xe70_mht`
- 2016: `HLT_xe90_mht_L1XE50`, `HLT_xe110_mht_L1XE50`
- 2017: `HLT_xe90_pufit_L1XE50`, `HLT_xe110_pufit_L1XE55`
- 2018: `HLT_xe110_pufit_xe70_L1XE50`, `HLT_xe120_pufit_L1XE50`

**Run 3 MET Triggers**:
- 2022-2024: `HLT_xe75_cell_xe65_tcpufit_xe90_trkmht_L1XE50`
- 2022-2024: `HLT_xe65_cell_xe100_mhtpufit_pf_L1XE50`
- 2022-2024: `HLT_xe80_cell_xe115_tcpufit_L1XE50`

**Strategy**: Logical OR of lepton + MET triggers for maximum efficiency in high-MET events.

### multileptonAnalysis & bbllAnalysis
**No MET triggers** - rely exclusively on single lepton, di-lepton, and lepton+tau triggers.

**Strategy**: Pure lepton-trigger approach suitable for analyses without high-MET requirements.

---

## 3. Lepton Working Points

### LeptonsMETAnalysis (`LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml:35-46`)
```yaml
Electron:
  min_pT: 7.e+3
  ID: "LooseBLayerLH"
  Iso: "NonIso"

Muon:
  min_pT: 7.e+3
  ID: "Loose"
  Iso: "PflowLoose_VarRad"
```

**Why loose**: Lower thresholds (7 GeV) for Z' → leptons + MET analysis with soft leptons.

### multileptonAnalysis (`multileptonAnalysis/share/RunConfig-multilepton.yaml:50-96`)
```yaml
Electron:
  min_pT: 10.0e+3
  ID: "LooseBLayerLH"
  Iso: "Loose_VarRad"
  extra_wps:
    - ["TightDNN", "Tight_VarRad"]
    - ["MediumDNN", "Tight_VarRad"]
    # ... many additional working points

Muon:
  min_pT: 10.0e+3
  ID: "Loose"
  Iso: "Loose_VarRad"
  extra_wps:
    - ["Tight", "PflowLoose_VarRad"]
    - ["Medium", "PflowLoose_VarRad"]
    # ... many additional working points
```

**Note**: Baseline uses Loose ID but with Loose_VarRad isolation (not NonIso). Also computes many extra working points for systematic studies.

### bbllAnalysis (`bbllAnalysis/share/RunConfig-bbll.yaml:47-54`)
```yaml
Electron:
  ID: "MediumLH"
  Iso: "Loose_VarRad"

Muon:
  ID: "Medium"
  Iso: "PflowLoose_VarRad"
```

**Why tighter**: Di-lepton analysis with cleaner selection, higher purity requirements.

---

## 4. Container and Output Configuration

All three packages use the **same container names** for jets and MET:

```yaml
reco_outputs:
  small_R_jets: 'container_names.output.reco4PFlowJet'
  met: 'container_names.output.met'
```

This means all three packages are accessing:
- **Jets**: `AntiKt4EMPFlowJets` (DAOD_PHYS) → calibrated to `AntiKt4EMPFlowCustomVtxJets`
- **MET**: Reconstructed from PFlow jets

**Conclusion**: The container setup is identical, so PHYSLITE incompatibility is **NOT** caused by container naming differences.

---

## 5. B-tagging Configuration

| Package | Nominal WP | Extra WPs |
|---------|-----------|-----------|
| LeptonsMETAnalysis | GN2v01_FixedCutBEff_77 | 70, 85 |
| multileptonAnalysis | GN2v01_FixedCutBEff_85 | Continuous |
| bbllAnalysis | GN2v01_FixedCutBEff_85 | Continuous |

**Different but irrelevant**: B-tagging WP shouldn't affect PHYSLITE compatibility.

---

## 6. Analysis-Specific Features

### multileptonAnalysis
- **Purpose**: HH→4l, multi-lepton final states
- **Features**:
  - Tau reconstruction (`do_taus: true`)
  - Parent decoration for leptons
  - Track decoration for electrons
  - IFF (Isolation-Fix) decoration
  - Extra working points for systematics

### bbllAnalysis
- **Purpose**: HH→bbll, di-lepton final states
- **Features**:
  - Truth decay mode decoration: bbtt, bbWW, bbZZ
  - Resonant PNN scoring
  - Neutrino weighting tool
  - Cutflow with specific selections:
    - EXACTLY_TWO_LEPTONS
    - TWO_OPPOSITE_CHARGE_LEPTONS
    - EXACTLY_TWO_B_JETS

### LeptonsMETAnalysis
- **Purpose**: Z' → leptons + MET, BSM searches
- **Features**:
  - MET reconstruction with all components
  - MET triggers for high-MET events
  - Loose lepton selections for soft leptons
  - Lower pT thresholds (7 GeV vs 10 GeV)

---

## 7. Implications for PHYSLITE Testing

### Hypothesis 1: Package-Specific Issue
**If only LeptonsMETAnalysis fails** with PHYSLITE:
- **Likely culprit**: MET trigger configuration or loose working points
- **Test**: Disable MET triggers and re-test

### Hypothesis 2: DAOD-Specific Issue
**If all three packages fail** with the same DAOD_PHYS files:
- **Likely culprit**: p-tag incompatibility (p6026) or derivation format
- **Solution**: Request newer p-tag or different derivation

### Hypothesis 3: Framework-Wide Issue
**If all packages fail** but work with other DAOD_PHYS files:
- **Likely culprit**: Specific dataset issue
- **Solution**: Report to EasyJet developers, use different test files

---

## 8. Recommended Testing Strategy

### On lxplus with AthAnalysis 25.2.76:

1. **Install data files** for test packages:
```bash
cd ~/private/easyjet/easyjet/build
make -j8 multileptonAnalysisDataInstall -k
make -j8 bbllAnalysisDataInstall -k
source x86_64-el9-gcc14-opt/setup.sh
```

2. **Test multileptonAnalysis** on the same DAOD file:
```bash
cd ~/private/easyjet/easyjet/easyjet
easyjet-ntupler \
  -c multileptonAnalysis/share/RunConfig-multilepton.yaml \
  -e 100 \
  -o test_multilep.root \
  ~/private/DAOD_PHYS.45529054._000001.pool.root.1
```

**Expected outcome**:
- If **succeeds**: Issue is specific to LeptonsMETAnalysis config
- If **fails**: Issue is with DAOD file or EasyJet framework

3. **Test bbllAnalysis** on the same DAOD file:
```bash
easyjet-ntupler \
  -c bbllAnalysis/share/RunConfig-bbll.yaml \
  -e 100 \
  -o test_bbll.root \
  ~/private/DAOD_PHYS.45529054._000001.pool.root.1
```

**Expected outcome**:
- If **both succeed**: LeptonsMETAnalysis has unique issue (likely MET triggers)
- If **both fail**: DAOD file or framework issue

4. **Test with MET triggers disabled** (if needed):
```yaml
# Create LeptonsMETAnalysis/share/RunConfig-LeptonsMET-noMET.yaml
# Remove MET triggers from trigger.yaml, keep only lepton triggers
```

---

## 9. Key Findings for Software & Computing Week Presentation

1. **Trigger SF Constraint**: Loose working points don't have SF files
   - **Solution**: Use `doSF: false` (documented in trigger.yaml)
   - **Alternative**: Switch to Medium/Tight WPs (not recommended for soft leptons)

2. **High-MET Strategy Bypasses SF Issue**:
   - MET > 200-250 GeV: trigger efficiency ≈ 100%, SF ≈ 1.0
   - Physics-based solution, no framework changes needed
   - Validated by colleague (Eirik) and supervisor (David)

3. **PHYSLITE Incompatibility**:
   - Documented in `PHYSLITE_NOTES.md`
   - Use DAOD_PHYS format instead
   - Testing other packages will determine if issue is config-specific

4. **Production-Ready Configuration**:
   - Electron: 7 GeV, LooseBLayerLH, NonIso
   - Muon: 7 GeV, Loose, PflowLoose_VarRad
   - Jets: 20 GeV, PFlow, GN2v01 b-tagging
   - MET: Fully enabled with all components
   - Triggers: Lepton + MET logical OR

---

## 10. Next Steps

1. ✅ **Completed**: Configuration comparison and documentation
2. **On lxplus**: Test multileptonAnalysis and bbllAnalysis with DAOD_PHYS files
3. **If tests pass**: Identify LeptonsMETAnalysis-specific issue (likely MET triggers)
4. **If tests fail**: Report DAOD/framework issue to EasyJet developers
5. **Proceed to production**: Configuration validated, ready for v1 production without trigger SFs

---

## References

- **EasyJet base config**: `EasyjetHub/share/base-config.yaml`
- **Container names**: `EasyjetHub/share/container-names-DAOD_PHYS.yaml`
- **LeptonsMETAnalysis**: `LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml`
- **multileptonAnalysis**: `multileptonAnalysis/share/RunConfig-multilepton.yaml`
- **bbllAnalysis**: `bbllAnalysis/share/RunConfig-bbll.yaml`
- **PHYSLITE notes**: `LeptonsMETAnalysis/PHYSLITE_NOTES.md`
