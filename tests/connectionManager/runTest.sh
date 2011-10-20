#/bin/bash

export PATH="$PATH:../../src/base:../../src/modules:../testUtils:."
export NEDPATH="../../src:.."

./connectionManager -c Test1 >  out.tmp 2>  err.tmp
./connectionManager -c Test2 >> out.tmp 2>> err.tmp
./connectionManager -c Test3 >> out.tmp 2>> err.tmp
./connectionManager -c Test4 >> out.tmp 2>> err.tmp

diff -I '^Assigned runID=' \
     -I '^Loading NED files from' \
     -I '^OMNeT++ Discrete Event Simulation' \
     -I '^Version: ' \
     -I '^     Speed:' \
     -I '^** Event #' \
     -w exp-output out.tmp >diff.log 2>/dev/null

if [ -s diff.log ]; then
    cat diff.log
    [ "$1" = "update-exp-output" ] && \
        cat out.tmp >exp-output
    exit 1
else
    echo "PASSED $(basename $(cd $(dirname $0);pwd) )"
    rm -f out.tmp diff.log err.tmp
fi
exit 0