#/bin/bash

./basePhyLayer -c Test1 > out.tmp
./basePhyLayer -c Test2 >> out.tmp
./basePhyLayer -c Test6 >> out.tmp
./basePhyLayer -c Test7 >> out.tmp

diff -I '^Assigned runID=' -I '^Loading NED files from' -w exp-output out.tmp

rm -f out.tmp


