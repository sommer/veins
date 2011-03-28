#/bin/bash

./nicTest -c Test1 > out.tmp
./nicTest -c Test2 >> out.tmp
./nicTest -c Test3 >> out.tmp
./nicTest -c Test4 >> out.tmp
./nicTest -c Test5 >> out.tmp

diff -I '^Assigned runID=' -I '^Loading NED files from' -w exp-output out.tmp

rm -f out.tmp


