
Analysis Package for the $(S/H)H\rightarrow \gamma\gamma$ + multilepton analysis
=========================

# Folder structure

- `bin/`: Executables
  - `yyml-ntupler`
- `python/`: Main python code to configure the components (objects, selections as well as the variables to save)
  - `yyml_config`
- `share/`: yaml files containing configurations used by the components
  - `RunConfig-yyml-base`: where all the common flags are set;
  - `RunConfig-yyml`: configurations called by the executables (see below);
  - `RunConfig-yyml-bypass`: configurations called by the executables (see below). Runs the code in "pass-through" mode aka no-skimming is applied;
  - `trigger`: list of the triggers to use per year.
- `src/`: C++ code
  - `yymlSelectorAlg`: Find if the event pass the baseline yyml selection;
  - `BaselineVarsyymlAlg`: Compute the baseline variables for the analysis.
