#!/bin/sh

rm -f results/Test2*
./MiXiM -c Test2-A -u Cmdenv 
./MiXiM -c Test2-B -u Cmdenv 
./MiXiM -c Test2-C -u Cmdenv 