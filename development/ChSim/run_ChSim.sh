#!/bin/bash

# simple bash simulator wrapper script to enable runs vs. factors
# (c)2006, stefanv@upb.de

sim="./ChSim"
inifile="factors.ini"

#factors
v_var="0.1 1 10 20 50"

#1 loop per factor
for v in $v_var; do
(
  echo "[Parameters]"
  # output directory for the generated files -- make shure it exists
  echo "msMovementCell.FileWriter.directory_name = \"./traces/multichannel\""
  # prefix for the generated files
  echo "msMovementCell.FileWriter.filename_prefix = \"ChSim-v$v\""
  # mobile station speed in m/s e.g. 'uniform(1,5)' 
  # for varying from 1m/s to 5m/s or '5' for a constant speed of 5m/s
  echo "msMovementCell.MobileStation[*].msBehavior.speed = $v"
) > $inifile

 #execute simulator
 $sim
done
