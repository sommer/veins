#/bin/bash

export NEDPATH="../../../src:../.."

LIBSREF=( -l ../../../src/base/miximbase \
          -l ../../testUtils/miximtestUtils \
          -l ../utils/powerTestUtils -l \
          ../../../src/modules/miximmodules )
          
rm *.vec results/*.sca 2>/dev/null
for i in One Two Three Four
do
 ./deviceTestMulti -c $i -u Cmdenv "${LIBSREF[@]}"
done
