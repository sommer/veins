#/bin/bash

export NEDPATH="../../../src:../.."

LIBSREF=( -l ../../../src/base/miximbase \
          -l ../../testUtils/miximtestUtils \
          -l ../utils/powerTestUtils -l \
          ../../../src/modules/miximmodules )
          
rm *.vec results/*.sca
for i in One 
do
 ./deviceTestAll -c $i "${LIBSREF[@]}"
done
