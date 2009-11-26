#!/bin/bash
#
# This script extract vectors of interest, pads them with zeros
# to reach the required number of lines per run and concatenates them.
# Input: omnet++ vector files
# Output: one file per selected vector

# configure here

params=(packetsBER success successNoRS) 
# associative arrays
declare -A vectors
declare -A defaults
cfg=flury07
expectedNbLines=1000  # expected number of lines per run

# retrieve vector numbers
for param in ${params[@]}; do
  vectors[$param]=$(grep ^vector.*$param[[:space:]] $cfg-0.vec | awk '{print $2}')
done 
# set default values (missing)
defaults[packetsBER]=0.5
defaults[success]=0
defaults[successNoRS]=0

# Operates as follows:
# Loop over each vector file, in numerical order
# In each file, get the required values for each selected vector 
# output these values to the assembled file, and pads if necessary
echo -e \\n

for fichier in `ls -v $cfg-*.vec`; do
  echo $fichier ... 
  for param in ${params[@]} ; do
    vec=${vectors[$param]} # retrieve the vector number
    nbLines=$(grep ^$vec[[:space:]] $fichier | wc -l) # count the number of lines
    grep ^$vec[[:space:]] $fichier | awk '{print $4}' >> $cfg-$param.csv # save them
    padding=$nbLines # pad the file
    while [[ padding -lt expectedNbLines ]] ; do
      echo -e ${defaults[$param]}\\n >> $cfg-$param.csv
      padding=$((padding+1))
    done
  done
done
echo -e \\n

# legacy code below
#for param in ${params[@]} ; do
#  rm $cfg-$param.vec
#  for fichier in `ls -v $cfg-*-$param.vec` ; do
#    grep -v ^# $fichier >> $cfg-$param.vec
#  done
#done

