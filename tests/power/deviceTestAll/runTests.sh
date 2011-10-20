#/bin/bash

export PATH="$PATH:../../../src/base:../../../src/modules:../../testUtils:../utils:."
export NEDPATH="../../../src:../.."

rm *.vec results/*.sca
for i in One 
do
 ./deviceTestAll -c $i
done

