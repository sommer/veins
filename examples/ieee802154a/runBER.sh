#!/bin/bash
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 ; 
do 
  echo $i ; 
  ./ieee802154a -r $i ; 
  mv omnetpp.vec vectors/vector-run_$i.vec ; 
done

