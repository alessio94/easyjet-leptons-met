# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#
# Script providing a functional build and runtime environment for project
# easyjet when sourced from either BASH or ZSH.
#

# Check if we are in BASH. In that case determining the directory holding
# the script is relatively easy:
if [ "x${BASH_SOURCE[0]}" = "x" ]; then
    # This trick should do the right thing under ZSH:
    thisdir=$(dirname `print -P %x`)
    if [ $? != 0 ]; then
        echo "ERROR: This script must be sourced from BASH or ZSH"
        return 1
    fi
    # We are in ZSH, so feel free to use a ZSH specific formalism:
    if [[ $options[shwordsplit] != on ]]; then
        # Instruct ZSH to behave like BASH does while iterating over
        # space separated words:
        setopt shwordsplit
        easyjet_UNSETSHWORDSPLIT=1
    fi
else
    # The BASH solution is a bit more straight forward:
    thisdir=$(dirname ${BASH_SOURCE[0]})
fi

# Function removing the duplicates from a given environment variable
remove_duplicates() {
    # Make it clear that the function's argument is the name of the
    # environment variable:
    envname=$1
    # Make copy of the environment variable:
    eval "temp=\$$envname"
    # Ensure there is a trailing ":" (required by what follows)
    if [ "${temp: -1}" != ":" ]; then
        temp="${temp}:";
    fi
    # Reset the value of the environment variable:
    eval "${envname}="
    # Loop over the elements of the copy:
    while [ -n "${temp}" ]; do
        # Take the first element of the remaining list:
        x=${temp%%:*}
        # Check that it's not an empty string:
        if [ "$x" != "" ]; then
            # If it's an existing directory, get its absolute path:
            if [ -d "$x" ] && [ -r "$x" ] && [ -x "$x" ]; then
                x=$(\cd "$x";\pwd)
            fi
            # Get the current value of the environment variable:
            eval "envval=\$$envname"
            # Decide whether to add it to the environment or not:
            case ${envval}: in
                *:"$x":*) ;;
                *) eval "${envname}=\"${envval}:$x\"";;
            esac
        fi
        # Remove the element from the list:
        temp=${temp#*:}
    done
    # Remove the leading ":" from the path and export it:
    eval "envval=\$$envname"
    export ${envname}="${envval:1:${#envval}-1}"
    # And finally clean up:
    unset temp x envname envval
}

# Function to source setup scripts of base projects
setup_baseprojects() {
    mode=$1
    for project in ${easyjet_BASEPROJECTS}; do
        # Name of the setup file in the base release:
        fname=${easyjet_DIR}/../../../../${project}/${easyjet_VERSION}/InstallArea/${easyjet_PLATFORM}/setup.sh
        if [ -f ${fname} ]; then
            . ${fname} mode
        fi
        unset fname
    done
}

# Decide about the environment status variable to be used:
ENV_VAR=easyjet_SET_UP
if [ "$*" = "extonly" ]; then
    ENV_VAR=easyjet_EXTONLY_SET_UP
elif [ "$*" = "relonly" ]; then
    ENV_VAR=easyjet_RELONLY_SET_UP
fi
# Check what its current value is:
eval "ENV_VAR_VALUE=\$$ENV_VAR"

# Prevent the script from being called multiple times.
if [ "x${ENV_VAR_VALUE}" = "x" ]; then
    # Set the variable now:
    eval "export ${ENV_VAR}=1"

    # Set up the project's main directory:
    export easyjet_DIR=$(\cd ${thisdir};\pwd)
    unset thisdir

    # Export project name and version:
    export easyjet_PLATFORM=x86_64-el9-gcc14-opt
    export easyjet_VERSION=1.0.0

    # Base projects used for the build of easyjet:
    easyjet_BASEPROJECTS="AthAnalysis"

    # If the project has base projects, set up those first. First, we just set
    # up the external environment of the base projects:
    if [ "$*" != "relonly" ]; then
        setup_baseprojects extonly
    fi

    # Source the environment setup script, when we're not in release-only mode:
    if [ "$*" != "relonly" ]; then
        if [ -f ${easyjet_DIR}/env_setup.sh ]; then
            . ${easyjet_DIR}/env_setup.sh
        fi
    fi

    # If we are in "external only" mode, then don't do any of the rest.
    if [ "$*" != "extonly" ]; then

        # Now source the setup.sh file(s) of the base release(s), in
        # release-only mode:
        setup_baseprojects relonly

        # Set up the release's main directories:
        export CMAKE_PREFIX_PATH=${easyjet_DIR}:${CMAKE_PREFIX_PATH}

        # Set up the binary path(s):
        export PATH=${easyjet_DIR}/bin:${PATH}
        export PATH=${easyjet_DIR}/share:${PATH}

        # Set up the library path(s):
        export LD_LIBRARY_PATH=${easyjet_DIR}/lib:${LD_LIBRARY_PATH}
        export DYLD_LIBRARY_PATH=${easyjet_DIR}/lib:${DYLD_LIBRARY_PATH}

        # Set up the python path(s):
        export PYTHONPATH=${easyjet_DIR}/lib:${PYTHONPATH}
        export PYTHONPATH=${easyjet_DIR}/python:${PYTHONPATH}

        # Set up the jobOptions path:
        export JOBOPTSEARCHPATH=${easyjet_DIR}/jobOptions:${JOBOPTSEARCHPATH}

        # Set up the data path(s):
        export DATAPATH=${easyjet_DIR}/share:${DATAPATH}
        export DATAPATH=${easyjet_DIR}/data:${DATAPATH}
        export CALIBPATH=${easyjet_DIR}/share:${CALIBPATH}
        export CALIBPATH=${easyjet_DIR}/data:${CALIBPATH}

        # Set up the include path:
        export ROOT_INCLUDE_PATH=${easyjet_DIR}/include:${ROOT_INCLUDE_PATH}

        # Set up the XML path:
        export XMLPATH=${easyjet_DIR}/XML:${XMLPATH}

        # We can already not be in external-only mode. But the rest should only
        # be done if we are also not in release-only mode.
        if [ "$*" != "relonly" ]; then

            # Remove the duplicates from all the environment variables that were
            # touched by the script:
            remove_duplicates CMAKE_PREFIX_PATH
            remove_duplicates PATH
            remove_duplicates LD_LIBRARY_PATH
            remove_duplicates DYLD_LIBRARY_PATH
            remove_duplicates PYTHONPATH
            remove_duplicates JOBOPTSEARCHPATH
            remove_duplicates DATAPATH
            remove_duplicates CALIBPATH
            remove_duplicates ROOT_INCLUDE_PATH
            remove_duplicates XMLPATH

            # Add the current directory as well to JOBOPTSEARCHPATH:
            export JOBOPTSEARCHPATH=.:${JOBOPTSEARCHPATH}

        fi
    fi
fi

# Clean up:
unset ENV_VAR
if [ ! -z "${easyjet_UNSETSHWORDSPLIT}" ]; then
    unsetopt shwordsplit
    unset easyjet_UNSETSHWORDSPLIT
fi
