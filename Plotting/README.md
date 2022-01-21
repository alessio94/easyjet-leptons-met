# Plotting jobs

To make some exploratory plots, run
```
PileupPlotterConfig.py --filesInput myinputfile.pool.root --threads=1 --evtMax 10
```
Feel free to increase the number of events, though beware of how many events may be in your file in case it takes a long time.
You should find a new ROOT file in your `$WORKDIR/run` dir, so have a look at the contents.


