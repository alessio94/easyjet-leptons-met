#!/usr/bin/bash
#
# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#
# This script is used by CTest to run the test ut_HiggsWeightTool_test with the correct
# environment setup, and post processing.
#

# Set up the runtime environment:
source /afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/x86_64-el9-gcc14-opt/setup.sh

# Turn off xAOD monitoring for the test:
export XAOD_ACCESSTRACER_FRACTION=0

# Set variables for post-processing script:
export ATLAS_CTEST_PACKAGE='TruthWeightTools'
export ATLAS_CTEST_TESTNAME='ut_HiggsWeightTool_test'
export ATLAS_CTEST_LOG_SELECT_PATTERN=''
export ATLAS_CTEST_LOG_IGNORE_PATTERN=''

# Run a possible pre-exec script:
# No pre-exec necessary

# Run the test:
/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/TruthWeightTools/test-bin/ut_HiggsWeightTool_test.exe 2>&1 | tee ut_HiggsWeightTool_test.log

# Set the test's return code:
export ATLAS_CTEST_TESTSTATUS=${PIPESTATUS[0]}
# temporary for backwards-compatibilty:
export testStatus=${ATLAS_CTEST_TESTSTATUS}

# Put the reference file in place if it exists:
if [ -f /afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/TruthWeightTools/share/ut_HiggsWeightTool_test.ref ] &&
    [ "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/TruthWeightTools" != "/afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/build/TruthWeightTools" ]; then
    /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Cmake/4.0.1/Linux-x86_64/bin/cmake -E make_directory ../share
    /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Cmake/4.0.1/Linux-x86_64/bin/cmake -E create_symlink \
     /afs/cern.ch/user/a/alpizzin/private/easyjet/easyjet/easyjet/TruthWeightTools/share/ut_HiggsWeightTool_test.ref ../share/ut_HiggsWeightTool_test.ref
fi

# Set exit code in case no post-processing follows:
default_return() { return ${ATLAS_CTEST_TESTSTATUS} ; }
default_return

# Run a possible post-exec script:
if type post.sh >/dev/null 2>&1; then
    post.sh ut_HiggsWeightTool_test 
else
    exit $testStatus
fi
