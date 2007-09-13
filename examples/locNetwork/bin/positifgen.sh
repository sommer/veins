#!/bin/sh
# This script can be used to generate the .ini file for the simulation.
# It takes two parameters
# $1 = input scenario
# $2 = output scenario
#set -x

# get directory name of this script
pushd $(dirname `which "$0"`) > /dev/null
DIRNAME="$PWD"; 
popd > /dev/null
# include needed libraries
. $DIRNAME/util.sh

SCEN_DIR="$DIRNAME/../input/scenarios"

# This function shows how to use this script
function show_usage {
    echo "Usage: $1 <input scenario> <omnetpp initialization file>"
    echo "where <input scenario> is one of:"
    SCENS="$(ls $SCEN_DIR)"
    for i in $SCENS ;
    do
      echo  "      $i"
    done
    echo "and <omnetpp initialization file> is probably 'omnetpp.ini'"
}

# check number of arguments
if [ $# -ne 2 ]; then
    show_usage $0
    exit 1
fi

SCEN="$SCEN_DIR/$1"
# check first argument
if ! [ -f $SCEN ]; then
    show_usage $0
    echo "	<input scenario>: \"$1\" not found in \"$SCEN_DIR\""
    exit 1
fi

# avoid overwriting previously generated file
if [ -f "$2" ]; then
    echo -n "<omnetpp initialization file>: \"$2\" already existed, backing up as: "
    CNT=0
    while [ -f "$2.$CNT.bck" ]; do
	let CNT=CNT+1
    done
    echo "\"$2.$CNT.bck\""
    mv $2 "$2.$CNT.bck"
fi

INI_DIR="input/ini/"
# generate the file
preprocess_scenario $SCEN $1
cat $INI_DIR"omnetpp.header" > $2
bin/PositifScenario $1 >> $2
cat $INI_DIR"omnetpp.in" >> $2
rm $1
