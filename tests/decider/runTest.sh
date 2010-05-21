#/bin/bash

# old testing setup
# Test configurations with TestBaseDecider as Decider

./decider > out.tmp

diff -I '^Assigned runID=' -I '^Loading NED files from' -w exp-output out.tmp

rm -f out.tmp


