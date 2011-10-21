#/bin/bash

export PATH="${PATH}:../../src/base:../../src/modules:../testUtils:."
export NEDPATH="../../src:.."
LIBSREF=( '-l' '../../src/base/miximbase' \
          '-l' '../testUtils/miximtestUtils' \
          '-l' '../../src/modules/miximmodules' )

lCombined='tests'
lSingle='basePhyLayer'
if [ ! -e ${lSingle} -a ! -e ${lSingle}.exe ]; then
    if [ -e ../${lCombined}.exe ]; then
        ln -s ../${lCombined}.exe ${lSingle}.exe
    elif [ -e ../${lCombined} ]; then
        ln -s ../${lCombined}     ${lSingle}
    fi
fi

./${lSingle} -c Test1 "${LIBSREF[@]}">  out.tmp 2>  err.tmp
./${lSingle} -c Test2 "${LIBSREF[@]}">> out.tmp 2>> err.tmp
./${lSingle} -c Test6 "${LIBSREF[@]}">> out.tmp 2>> err.tmp
./${lSingle} -c Test7 "${LIBSREF[@]}">> out.tmp 2>> err.tmp

diff -I '^Assigned runID=' \
     -I '^Loading NED files from' \
     -I '^OMNeT++ Discrete Event Simulation' \
     -I '^Version: ' \
     -I '^     Speed:' \
     -I '^** Event #' \
     -w exp-output out.tmp >diff.log 2>/dev/null

if [ -s diff.log ]; then
    echo "FAILED counted $(( 1 + $(grep -c -e '^---$' diff.log) )) differences where #<=$(grep -c -e '^<' diff.log) and #>=$(grep -c -e '^>' diff.log); see $(basename $(cd $(dirname $0);pwd) )/diff.log"
    [ "$1" = "update-exp-output" ] && \
        cat out.tmp >exp-output
    exit 1
else
    echo "PASSED $(basename $(cd $(dirname $0);pwd) )"
    rm -f out.tmp diff.log err.tmp
fi
exit 0