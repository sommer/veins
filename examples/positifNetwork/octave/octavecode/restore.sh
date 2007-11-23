#!/bin/sh
set -x

function restore_file {
    if [ -f $1~ ]
	then
	mv $1~ $1
    fi
	
}

OCTAVE_SRCS="$(ls *.m)"
for f in $OCTAVE_SRCS
do
  restore_file $f
done
