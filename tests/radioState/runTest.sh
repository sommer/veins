#/bin/bash

./radioState > out.tmp

diff -I '^Assigned runID=' -w exp-output out.tmp

rm -f out.tmp
