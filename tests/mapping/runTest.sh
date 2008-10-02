#/bin/bash

./mapping > out.tmp

diff -I '^Assigned runID=' -w exp-output4 out.tmp

rm -f out.tmp
