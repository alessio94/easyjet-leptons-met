# function to add tab complete to unit tests

# set the environment variable TABTEST to dump output to that file,
# and read with `tail -f`

# Check if we are in BASH, if not get out of here
if [ "x${BASH_SOURCE[0]}" = "x" ]; then
    return
fi

# add a function to help with the tests
function _easyjet-test() {
    local current=${COMP_WORDS[COMP_CWORD]}
    local previous=${COMP_WORDS[COMP_CWORD-1]}
    local log=${TABTEST-/dev/null}
    echo -n "current: ${current}, previous: ${previous}," >> ${log}
    # try to use a few special cases first
    if [[ ${previous} == '-l' ]]; then
        local verb="VERBOSE DEBUG INFO WARNING ERROR"
        COMPREPLY=($(compgen -W "${verb}" -- ${current}))
        echo " log" >> ${log}
        return 0
    fi
    if [[ ${previous} == '-d' ]]; then
        COMPREPLY=($(compgen -d -- ${current}))
        echo " dir" >> ${log}
        return 0
    fi
    # if we're down here just ask the test script for options
    local opts=$(easyjet-test -c)
    COMPREPLY=($(compgen -W "${opts}" -- ${current}))
    echo " all options" >> ${log}
}
complete -F _easyjet-test easyjet-test
