#/bin/bash

./baseMobility -c Test1 > out.tmp

diff -I '^Assigned runID=' -I '^Loading NED files from' -w exp-output out.tmp

rm -f out.tmp
