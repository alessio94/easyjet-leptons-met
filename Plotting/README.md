# Plotting jobs

To make some exploratory pileup and invariant mass plots, run
```
PileupPlotterConfig.py --filesInput myinputfile.pool.root --threads=1 --nTrkMin=2 --evtMax 10
VariablePlotterConfig.py --filesInput myinputfile.pool.root --threads=1 --evtMax 10
```
Feel free to increase the number of events, though beware of how many events may be in your file in case it takes a long time.
You should find two new ROOT files, so have a look at the contents.


