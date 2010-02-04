#!/bin/sh

rm -f results/Test2*
./mixim -c Test2-A -u Cmdenv 
./mixim -c Test2-B -u Cmdenv 
./mixim -c Test2-C -u Cmdenv 
