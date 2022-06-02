# Plotting jobs

To make some exploratory pileup and invariant mass plots, as well as getting a tree of variables, run:
```
PileupPlotterConfig.py --filesInput data.myinputfile.DAOD_PHYS.pool.root --threads=1 --nTrkMin=2 --evtMax 10
VariablePlotterConfig.py --filesInput data.myinputfile.DAOD_PHYS.pool.root --threads=1 --evtMax 10
VariableDumperConfig.py --filesInput data.myinputfile.DAOD_PHYS.pool.root --threads=1 --evtMax 10
```
Feel free to increase the number of events, though beware of how many events may be in your file in case it takes a long time.
You should find tree new ROOT files, `pileup-hists.root`, `variable-hists.root` and `analysis-variables.root`, respectively.

To process Monte Carlo samples the `--mc` flag is needed:

```
VariableDumperConfig.py --filesInput mc.myinputfile.pool.root --threads=1 --evtMax 10 --mc
```

To process DAOD_PHYSLITE the `--daod-physlite` flag is needed:

```
VariableDumperConfig.py --filesInput mc.myinputfile.DAOD_PHYSLITE.pool.root --threads=1 --evtMax 10 --mc --daod-physlite
```


