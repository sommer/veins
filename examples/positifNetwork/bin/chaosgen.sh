#!/bin/sh
# This script can be used to generate the .ini file for the simulation.
#set -x

# get directory name of this script
pushd $(dirname `which "$0"`) > /dev/null
DIRNAME="$PWD"; 
popd > /dev/null
# include needed libraries
. $DIRNAME/util.sh

# This function shows how to use this script
function show_usage {
    echo "Usage: $1 <node count> <anchor count> <omnetpp initialization file>"
    echo "where <omnetpp initialization file> is probably 'omnetpp.ini'"
}

# check number of arguments
if [ $# -ne 3 ]; then
    show_usage $0
    exit 1
fi

# avoid overwriting previously generated file
if [ -f "$3" ]; then
    echo -n "<omnetpp initialization file>: \"$3\" already existed, backing up as: "
    CNT=0
    while [ -f "$3.$CNT.bck" ]; do
	let CNT=CNT+1
    done
    echo "\"$3.$CNT.bck\""
    mv $3 "$3.$CNT.bck"
fi

INI_DIR="input/ini/"
# generate the file
cat $INI_DIR"omnetpp.header" > $3
bin/ChaosScenario $1 $2 >> $3
cat $INI_DIR"omnetpp.in" >> $3
