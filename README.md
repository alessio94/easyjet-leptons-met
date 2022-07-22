# HH4bAnalysis

HH to 4b Analysis Framework initiated by Humboldt-Universität zu Berlin ERC Project.

# Installation

*The instructions below with `setupATLAS` and `asetup` assume you are working on a CERN CentOS terminal, e.g. lxplus or a Singularity container on an institute cluster. Alternative instructions for using Docker images are given below.*

First, in a new working directory (we'll refer to this as `$WORKDIR` -- feel free to make an alias with `export WORKDIR=.`) clone the repository (you are also welcome to fork a copy in case you might want to develop on top and contribute to improving it!):

```
# Copy-paste this repo's URL, choosing your preferred authentication scheme, e.g. for lxplus or institute cluster
git clone ssh://git@gitlab.cern.ch:7999/hub-zn-hep/hh4b-analysis.git
```

Now, compile the package

```
mkdir build
cd build
setupATLAS
asetup AthAnalysis,22.2.80
cmake ../hh4b-analysis/
make
source */setup.sh
```

# Running on files

To make some exploratory pileup and invariant mass plots, as well as getting a tree of variables, run:

```
VariableDumperConfig.py --filesInput data.myinputfile.DAOD_PHYS.pool.root --evtMax 10
```

Feel free to increase the number of events, though beware of how many events may be in your file in case it takes a long time.
You should find a new ROOT files, `analysis-variables.root`.

To process Monte Carlo samples the `--mc` flag is needed:

```
VariableDumperConfig.py --filesInput mc.myinputfile.pool.root --evtMax 10 --mc 
```

To process DAOD_PHYSLITE the `--daod-physlite` flag is needed:

```
VariableDumperConfig.py --filesInput mc.myinputfile.DAOD_PHYSLITE.pool.root --evtMax 10 --mc --daod-physlite 
```

## AthAnalysis in Docker

If you would rather work on a local computer, numbered `AthAnalysis` releases are available as Docker containers [on dockerhub](https://hub.docker.com/r/atlas/athanalysis/). Naturally, you will have to install [Docker](https://www.docker.com).

 Preferably, do this in `$WORKDIR`.

```
docker pull atlas/athanalysis:22.2.80
docker run -t -i -v $PWD:/workarea:delegated -v $HOME:$HOME:delegated atlas/athanalysis:22.2.80
```

This will start up an interactive terminal inside the container, which has read/write access to the following paths:

* the present working directory (`$PWD`) under the path `/workarea`;
* your home directory (`$HOME`) with the same full path name (e.g. `/Users/myname`).
The terminal itself begins in an empty directory, `/workdir`. *The `delegated` suffix for these mounted volumes helps optimise the read/write access for better responsiveness.* Within this terminal, you can follow the instructions to source the `/release_setup.sh` script, in place of `setupATLAS; asetup`.

If you encounter any issues, some relevant instructions are available at <https://atlassoftwaredocs.web.cern.ch/athena/dev-setup/>. The `AthAnalysis` containers do not require `cvmfs` access, but you may need to experiment with the command line arguments when launching the container.

## Restore the setup

If you come back to this in a new shell session, you can recover the setup with:

```
cd $WORKDIR/build
setupATLAS
asetup --restore
source */setup.sh
```

*If you are working in a container, source the `/release_setup.sh` script, instead of the `setupATLAS; asetup` commands.*

## Development

You can use pre-commit hooks, that check if you linted and formatted the files when you execute `git commit`. It prevents you from pushing something that will fail a pipeline in the CI on gitlab. All you have to do is install pre-commit with pip. The following will do it for you (currently only pre-commits is in the requirements.txt, but let's have it like that as it might grow).

```
pip install -r requirements.txt --user
pre-commit install
pre-commit run 
```
