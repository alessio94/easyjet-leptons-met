# HH4bAnalysis

HH to 4b Analysis Framework initiated by Humboldt-Universität zu Berlin ERC Project.

# Installation

*The instructions below with `setupATLAS` and `asetup` assume you are working on a CERN CentOS terminal, e.g. lxplus or a Singularity container on an institute cluster. Alternative instructions for using Docker images are given below.*

First, in a new working directory (we'll refer to this as `$WORKDIR` -- feel free to make an alias with `export WORKDIR=.`) clone the repository (you are also welcome to fork a copy in case you might want to develop on top and contribute to improving it!):
```
# Copy-paste this repo's URL, choosing your preferred authentication scheme, e.g. for lxplus or institute cluster
git clone ssh://git@gitlab.cern.ch:7999/hub_zn_hep/hh4b-analysis.git
```
Now, compile the package
```
mkdir build
cd build
setupATLAS
asetup AthAnalysis,22.2.55
cmake ../hh4b-analysis/
make
source */setup.sh
```

## AthAnalysis in Docker

If you would rather work on a local computer, numbered `AthAnalysis` releases are available as Docker containers [on dockerhub](https://hub.docker.com/r/atlas/athanalysis/). Naturally, you will have to install [Docker](https://www.docker.com).

 Preferably, do this in `$WORKDIR`.
```
docker pull atlas/athanalysis:22.2.55
docker run -t -i -v $PWD:/workarea:delegated -v $HOME:$HOME:delegated atlas/athanalysis:22.2.55
```
This will start up an interactive terminal inside the container, which has read/write access to the following paths:
* the present working directory (`$PWD`) under the path `/workarea`;
* your home directory (`$HOME`) with the same full path name (e.g. `/Users/myname`).
The terminal itself begins in an empty directory, `/workdir`. *The `delegated` suffix for these mounted volumes helps optimise the read/write access for better responsiveness.* Within this terminal, you can follow the instructions to source the `/release_setup.sh` script, in place of `setupATLAS; asetup`.

If you encounter any issues, some relevant instructions are available at https://atlassoftwaredocs.web.cern.ch/athena/dev-setup/. The `AthAnalysis` containers do not require `cvmfs` access, but you may need to experiment with the command line arguments when launching the container.

## Restore the setup

If you come back to this in a new shell session, you can recover the setup with:
```
cd $WORKDIR/build
setupATLAS
asetup AthAnalysis,22.2.55
source */setup.sh
```
*If you are working in a container, source the `/release_setup.sh` script, instead of the `setupATLAS; asetup` commands.*