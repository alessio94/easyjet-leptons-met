Analysis Package for bbtt
=========================


# Folder structure


- `bin/`: Executables
  - `bbtt-ntupler`
- `python/`: Python modules
- `share/`: Run configuration yaml files
- `src/`: C++ code
- `datasets/`: Text files holding dataset lists for grid submission



# Naming conventions

TODO


# How to Run

run the analysis with selection on PHYSLITE with selection: 
```
bbtt-ntupler <PHYSLITE_INPUTFILE.root> --run-config easyjet/bbttAnalysis/share/RunConfig-PHYSLITE-bbtt.yaml --evtMax 1000 --out-file outputfile.root
```

run the analysis with selection on PHYSLITE without selection: 
```
bbtt-ntupler <PHYSLITE_INPUTFILE.root> --run-config easyjet/bbttAnalysis/share/RunConfig-PHYSLITE-bbtt-bypass.yaml --evtMax 1000 --out-file outputfile.root
```


run the analysis with selection on PHYS with selection: 
```
bbtt-ntupler <PHYSLITE_INPUTFILE.root> --run-config easyjet/bbttAnalysis/share/RunConfig-PHYS-bbtt.yaml --evtMax 1000 --out-file outputfile.root
```


run the analysis with selection on PHYS without selection: 

```
bbtt-ntupler <PHYSLITE_INPUTFILE.root> --run-config easyjet/bbttAnalysis/share/RunConfig-PHYS-bbtt-bypass.yaml --evtMax 1000 --out-file outputfile.root
```


If everything works you should have a rootfile named outputfile.root. In the file is the following:
- event specific variables like runNumber/eventNumber/lumiBlock...
- Event passing a certain trigger
- a vector for the 4 momentum and other quantities of the selected particles (muons/electrons/taus/jets) named
  - el_pt/el_eta/el_phi/el_charge
  - mu_pt/mu_eta/mu_phi/mu_charge
  - tau_pt/tau_eta/tau_phi/tau_charge/tau_nProng/tau_isIDTau/tau_isAntiTau
  - recojet_antikt4PFlow_pt/recojet_antikt4PFlow_eta/recojet_antikt4PFlow_phi/recojet_antikt4PFlow_m/recojet_antikt4PFlow_NNJvtPass/recojet_antikt4PFlow_ftag_select_DL1dv01_FixedCutBEff_77/
  - MET
- truth level information
- output of the Missing Mass Calculator (MMC)
- info after selection via variables starting with bbtt_
  - leading tau
  - subleading tau
  - selected lepton
- dihiggs variables (HH_)
