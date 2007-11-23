#!/bin/sh
set -x

function rewrite_file {
    cp $1 $1~
    sed 's/gset/__gnuplot_set__/g' $1~ > $$$1
    sed 's/gplot/__gnuplot_plot__/g' $$$1 > $1
    rm $$$1
}

OCTAVE_SRCS="$(ls *.m)"
for f in $OCTAVE_SRCS
do
  rewrite_file $f
done
