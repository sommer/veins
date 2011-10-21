#/bin/bash

export PATH="$PATH:../../../src/base:../../../src/modules:../../testUtils:../utils:."
export NEDPATH="../../../src:../.."

LIBSREF=( -l ../../../src/base/miximbase \
          -l ../../testUtils/miximtestUtils \
          -l ../utils/powerTestUtils -l \
          ../../../src/modules/miximmodules )
          
rm *.vec *.sca 2>/dev/null
for i in One Two Three Four Five Six Seven Eight Nine Ten
do
 ./Nic80211Test -u Cmdenv -c $i "${LIBSREF[@]}"
done
