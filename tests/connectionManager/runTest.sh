#/bin/bash

./connectionManager -c Test1 > out.tmp
./connectionManager -c Test2 >> out.tmp
./connectionManager -c Test3 >> out.tmp
./connectionManager -c Test4 >> out.tmp

diff -I '^Assigned runID=' -w exp-output4 out.tmp

rm -f out.tmp
