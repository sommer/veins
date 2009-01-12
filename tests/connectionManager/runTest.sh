#/bin/bash

./connectionManager -c Test1 > out.tmp
./connectionManager -c Test2 >> out.tmp
./connectionManager -c Test3 >> out.tmp
./connectionManager -c Test4 >> out.tmp

diff -I '^Assigned runID=' -I '^Loading NED files from' -w exp-output out.tmp

rm -f out.tmp
