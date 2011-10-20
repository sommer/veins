#/bin/bash

export PATH="$PATH:../../../src/base:../../../src/modules:../../testUtils:../utils:."
export NEDPATH="../../../src:../.."

rm *.vec results/*.sca
for i in One Two 
do
 ./deviceTestAccts -c $i
done

