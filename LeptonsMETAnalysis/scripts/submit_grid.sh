#!/bin/bash

################################################################################
# Grid Submission Script for LeptonsMETAnalysis
# Z' → Leptons + MET Analysis
#
# Usage:
#   ./submit_grid.sh [test|data|mc|all]
#
# Options:
#   test - Submit small test with 2 files (no git tag)
#   data - Submit full data production
#   mc   - Submit full MC production
#   all  - Submit both data and MC production
#
# Before running:
#   1. Edit the dataset lists in LeptonsMETAnalysis/datasets/
#   2. Check grid credentials: voms-proxy-info
#   3. Setup panda: lsetup panda
#   4. Test locally first!
################################################################################

set -e  # Exit on error

# ==============================================================================
# Configuration
# ==============================================================================

# Paths
RUNCONFIG="LeptonsMETAnalysis/share/RunConfig-LeptonsMET.yaml"
EXECUTABLE="easyjet-ntupler"
DATASET_DIR="LeptonsMETAnalysis/datasets"

# Campaign name (will be part of output dataset name)
CAMPAIGN_NAME="ZprimeLepMET_v01"

# Dataset lists
DATA_LIST="${DATASET_DIR}/data_Run3_test.txt"
MC_LIST="${DATASET_DIR}/mc_Zprime_test.txt"

# Grid job options
NGBPERJOB=10           # GB per job (auto-splits datasets)
MEMORY=4000            # Memory per core in MB
DEST_SE=""             # Destination storage element (empty = default)
                       # Example: "DESY-ZN_LOCALGROUPDISK"

# Test job options
TEST_NFILES=2          # Number of files for test
TEST_NFILESPERJOB=1    # Files per job for test

# ==============================================================================
# Functions
# ==============================================================================

check_prerequisites() {
    echo "Checking prerequisites..."

    # Check grid proxy
    if ! voms-proxy-info --exists --valid 24:00 2>/dev/null; then
        echo "ERROR: No valid grid proxy found (need at least 24 hours remaining)"
        echo "Run: voms-proxy-init -voms atlas -valid 168:00"
        exit 1
    fi
    echo "  ✓ Grid proxy valid"

    # Check panda
    if ! which prun &>/dev/null; then
        echo "ERROR: prun command not found"
        echo "Run: lsetup panda"
        exit 1
    fi
    echo "  ✓ panda setup"

    # Check executable
    if ! which ${EXECUTABLE} &>/dev/null; then
        echo "ERROR: ${EXECUTABLE} not found"
        echo "Make sure you've sourced the build environment:"
        echo "  cd ~/private/easyjet/easyjet/build"
        echo "  source x86_64-el9-gcc14-opt/setup.sh"
        exit 1
    fi
    echo "  ✓ ${EXECUTABLE} available"

    # Check run config
    if [ ! -f "${RUNCONFIG}" ]; then
        echo "ERROR: Run config not found: ${RUNCONFIG}"
        exit 1
    fi
    echo "  ✓ Run config found"

    echo "All prerequisites met!"
    echo ""
}

submit_test() {
    echo "========================================================================"
    echo "Submitting TEST JOBS"
    echo "========================================================================"
    echo "Campaign: ${CAMPAIGN_NAME}_test"
    echo "Data list: ${DATA_LIST}"
    echo "Processing: ${TEST_NFILES} files (${TEST_NFILESPERJOB} per job)"
    echo "Config: ${RUNCONFIG}"
    echo ""
    echo "This will:"
    echo "  - Submit a small test (${TEST_NFILES} files)"
    echo "  - Skip git tag creation (--noTag)"
    echo "  - Use minimal resources for quick turnaround"
    echo ""
    read -p "Continue? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Cancelled."
        exit 0
    fi

    easyjet-gridsubmit \
        --data-list ${DATA_LIST} \
        --run-config ${RUNCONFIG} \
        --exec ${EXECUTABLE} \
        --campaign ${CAMPAIGN_NAME}_test \
        --nFiles ${TEST_NFILES} \
        --nFilesPerJob ${TEST_NFILESPERJOB} \
        --nGBperJob 5 \
        --noTag

    echo ""
    echo "Test jobs submitted!"
    echo "Monitor with: pbook"
    echo ""
}

submit_data() {
    echo "========================================================================"
    echo "Submitting DATA PRODUCTION"
    echo "========================================================================"
    echo "Campaign: ${CAMPAIGN_NAME}"
    echo "Data list: ${DATA_LIST}"
    echo "Config: ${RUNCONFIG}"
    echo ""
    echo "This will:"
    echo "  - Process all datasets in ${DATA_LIST}"
    echo "  - Create git tag: ${CAMPAIGN_NAME}"
    echo "  - Use ${NGBPERJOB} GB per job"
    echo "  - Request ${MEMORY} MB memory per core"
    echo "  - Merge output files"
    echo ""

    # Check if data list has actual datasets
    if ! grep -q "^[^#*]" ${DATA_LIST} 2>/dev/null; then
        echo "ERROR: ${DATA_LIST} is empty or has no valid datasets"
        echo "Please edit the file and add your dataset names."
        exit 1
    fi

    echo "Datasets to process:"
    grep "^[^#*]" ${DATA_LIST} | head -5
    NLINES=$(grep -c "^[^#*]" ${DATA_LIST})
    if [ $NLINES -gt 5 ]; then
        echo "... and $(($NLINES - 5)) more"
    fi
    echo ""

    read -p "Continue with full data production? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Cancelled."
        exit 0
    fi

    CMD="easyjet-gridsubmit \
        --data-list ${DATA_LIST} \
        --run-config ${RUNCONFIG} \
        --exec ${EXECUTABLE} \
        --campaign ${CAMPAIGN_NAME} \
        --nGBperJob ${NGBPERJOB} \
        --memory ${MEMORY} \
        --mergeOutput"

    if [ -n "${DEST_SE}" ]; then
        CMD="${CMD} --dest-se ${DEST_SE}"
    fi

    echo "Running: ${CMD}"
    echo ""

    eval ${CMD}

    echo ""
    echo "Data production submitted!"
    echo "Monitor with: pbook"
    echo ""
}

submit_mc() {
    echo "========================================================================"
    echo "Submitting MC PRODUCTION"
    echo "========================================================================"
    echo "Campaign: ${CAMPAIGN_NAME}"
    echo "MC list: ${MC_LIST}"
    echo "Config: ${RUNCONFIG}"
    echo ""
    echo "This will:"
    echo "  - Process all MC samples in ${MC_LIST}"
    echo "  - Use existing git tag: ${CAMPAIGN_NAME} (or create if missing)"
    echo "  - Use ${NGBPERJOB} GB per job"
    echo "  - Request ${MEMORY} MB memory per core"
    echo "  - Merge output files"
    echo ""

    # Check if MC list has actual datasets
    if ! grep -q "^[^#*]" ${MC_LIST} 2>/dev/null; then
        echo "ERROR: ${MC_LIST} is empty or has no valid datasets"
        echo "Please edit the file and add your MC dataset names."
        exit 1
    fi

    echo "MC samples to process:"
    grep "^[^#*]" ${MC_LIST} | head -5
    NLINES=$(grep -c "^[^#*]" ${MC_LIST})
    if [ $NLINES -gt 5 ]; then
        echo "... and $(($NLINES - 5)) more"
    fi
    echo ""

    read -p "Continue with full MC production? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Cancelled."
        exit 0
    fi

    CMD="easyjet-gridsubmit \
        --mc-list ${MC_LIST} \
        --run-config ${RUNCONFIG} \
        --exec ${EXECUTABLE} \
        --campaign ${CAMPAIGN_NAME} \
        --nGBperJob ${NGBPERJOB} \
        --memory ${MEMORY} \
        --mergeOutput"

    if [ -n "${DEST_SE}" ]; then
        CMD="${CMD} --dest-se ${DEST_SE}"
    fi

    echo "Running: ${CMD}"
    echo ""

    eval ${CMD}

    echo ""
    echo "MC production submitted!"
    echo "Monitor with: pbook"
    echo ""
}

show_usage() {
    echo "Usage: $0 [test|data|mc|all]"
    echo ""
    echo "Options:"
    echo "  test - Submit small test with 2 files (no git tag)"
    echo "  data - Submit full data production"
    echo "  mc   - Submit full MC production"
    echo "  all  - Submit both data and MC production"
    echo ""
    echo "Before running:"
    echo "  1. Edit dataset lists in ${DATASET_DIR}/"
    echo "  2. Test locally: easyjet-ntupler -c ${RUNCONFIG} -e 100 -o test.root input.root"
    echo "  3. Check grid proxy: voms-proxy-info"
    echo "  4. Setup panda: lsetup panda"
    echo ""
}

# ==============================================================================
# Main
# ==============================================================================

# Parse command line argument
MODE=${1:-""}

if [ -z "$MODE" ]; then
    show_usage
    exit 1
fi

# Check prerequisites for all modes
check_prerequisites

# Execute based on mode
case $MODE in
    test)
        submit_test
        ;;
    data)
        submit_data
        ;;
    mc)
        submit_mc
        ;;
    all)
        submit_data
        submit_mc
        ;;
    *)
        echo "ERROR: Unknown option: $MODE"
        echo ""
        show_usage
        exit 1
        ;;
esac

echo "========================================================================"
echo "Grid submission complete!"
echo ""
echo "Next steps:"
echo "  1. Monitor jobs: pbook"
echo "  2. Check status: pbook> show()"
echo "  3. Download outputs when finished:"
echo "     rucio download 'user.<nickname>.${CAMPAIGN_NAME}.*'"
echo ""
echo "Good luck! 🚀"
echo "========================================================================"
