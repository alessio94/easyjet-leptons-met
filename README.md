# HH4bAnalysis

$HH \to b\bar{b}b\bar{b}$ Analysis Framework initiated by Humboldt-Universität zu Berlin ERC Project.

# Installation

*The instructions below with `setupATLAS` and `asetup` assume you are working on a CERN CentOS terminal, e.g. lxplus or a Singularity container on an institute cluster. Alternative instructions for using Docker images are given below.*

First, in a new working directory (we'll refer to this as `$WORKDIR` -- feel free to make an alias with `export WORKDIR=.`) clone the repository (you are also welcome to fork a copy in case you might want to develop on top and contribute to improving it!):
```
# Copy-paste this repo's URL, choosing your preferred authentication scheme, e.g.
git clone ssh://git@gitlab.cern.ch:7999/viruelas/HH4bAnalysis.git
```
Now, compile the package
```
mkdir build run
cd build
setupATLAS
asetup AthAnalysis,22.2.55
cmake ../HH4bAnalysis/
make
source */setup.sh
```

## AthAnalysis in Docker

If you would rather work on a local computer, numbered `AthAnalysis` releases are available as Docker containers [on dockerhub](https://hub.docker.com/r/atlas/athanalysis/). Naturally, you will have to install [Docker](https://www.docker.com).

Once you have installed Docker, pull your desired image (here we use 22.2.55 throughout), then launch the container. Preferably, do this in `$WORKDIR`.
```
docker pull atlas/athanalysis:22.2.55
docker run -t -i -v $PWD:/workarea:delegated -v $HOME:$HOME:delegated atlas/athanalysis:22.2.55
```
This will start up an interactive terminal inside the container, which has read/write access to the following paths:
* the present working directory (`$PWD`) under the path `/workarea`;
* your home directory (`$HOME`) with the same full path name (e.g. `/Users/myname`).
The terminal itself begins in an empty directory, `/workdir`. *The `delegated` suffix for these mounted volumes helps optimise the read/write access for better responsiveness.* Within this terminal, you can follow the instructions to source the `/release_setup.sh` script, in place of `setupATLAS; asetup`.

If you encounter any issues, some relevant instructions are available at https://atlassoftwaredocs.web.cern.ch/athena/dev-setup/. The `AthAnalysis` containers do not require `cvmfs` access, but you may need to experiment with the command line arguments when launching the container.