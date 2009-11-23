#!/bin/sh

rm -f results/Test2*
./csma -c Test2-A -u Cmdenv 
./csma -c Test2-B -u Cmdenv 
./csma -c Test2-C -u Cmdenv 