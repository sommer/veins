#!/bin/sh
# This script can be used to generate the .ini file for the simulation.
# It takes two parameters
# $1 = input scenario
# $2 = output scenario
#set -x

# Show how to use this script
function show_usage {
    echo "Usage: $0 <input scenario> <omnetpp initialization file>"
}

# Preprocess the scenario
function preprocess_scenario {
    sed '
s/#.*//g; /./!d
' $1 > $2
}

# check number of arguments
if [ $# -ne 2 ]; then
    show_usage $0
    exit 1
fi

SCEN_DIR="input/scenarios/"
SCEN="$SCEN_DIR$1"
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
