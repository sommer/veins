#!/bin/sh
# This script reformats akaroa simulation results.
# It generates one csv file per parameter.
# Data files should have the name ak-$config-$run.out

#CONFIG="MAICircleNPoisson"
#CONFIG="maicirclen"
CONFIG="berdistance"

NBRUNS=$((`ls ak-$CONFIG-*.out | wc -l`-1))
NBPARAMS=`grep   ^[[:space:]] ak-$CONFIG-0.out | wc -l`

# create destination files
for p in `seq 1 $NBPARAMS`; do
#  touch $CONFIG-$p.csv
  echo "Param    Estimate       Delta  Conf TotalObs TransObs" > $CONFIG-$p.csv
done

# extract data from each run file
for r in `seq 0 $NBRUNS`; do
  for p in `seq 1 $NBPARAMS`; do
    grep ^[[:space:]]\\+$p ak-$CONFIG-$r.out >> $CONFIG-$p.csv
  done
done


