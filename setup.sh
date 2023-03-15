# setup ATLAS
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase

echo "=== running setupATLAS ==="
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh -q

echo "=== running asetup ==="
# fancy pants check to get version number from ci image
SCRIPT_PATH=${BASH_SOURCE[0]:-${0}}
CI=${SCRIPT_PATH%/*}/.gitlab-ci.yml
DOCKER_ANALYSIS_BASE_VERSION=$(
    sed -rn '/athanalysis/ s/.*:([^-]*).*/\1/ p' $CI | head -n1)
unset CI

# make sure our pants weren't _too_ fancy
if [[ ! $DOCKER_ANALYSIS_BASE_VERSION =~ [0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "couldn't parse CI file for base image, setting up latest"
fi
asetup AthAnalysis,${DOCKER_ANALYSIS_BASE_VERSION-"master,latest"}
unset DOCKER_ANALYSIS_BASE_VERSION
