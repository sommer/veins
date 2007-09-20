#!/bin/sh
# This script preprocesses a scenario

# get directory name of this script
pushd $(dirname `which "$0"`) > /dev/null
DIRNAME="$PWD"; 
popd > /dev/null
# include needed libraries
. $DIRNAME/util.sh

SCEN_DIR="$DIRNAME/../input/scenarios"

# This function shows how to use this script
function show_usage {
    echo "Usage: $1 <input scenario>"
    echo "where <input scenario> is one of:"
    SCENS="$(ls $SCEN_DIR)"
    for i in $SCENS ;
    do
      echo  "      $i"
    done
}

# check number of arguments
if [ $# -ne 1 ]; then
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

preprocess_scenario $SCEN $1
