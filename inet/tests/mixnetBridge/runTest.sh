#/bin/bash

./testCase -c Test1 > out.tmp
./testCase -c Test2 >> out.tmp
./testCase -c Test3 >> out.tmp
./testCase -c Test4 >> out.tmp
./testCase -c Test5 >> out.tmp
./testCase -c Test6 >> out.tmp

diff -I '^Assigned runID=' -I '^Loading NED files from' -w exp-output out.tmp

rm -f out.tmp
