#!/bin/bash
set -e

TESTS="*.test"
VEINS_PATH="../../../../../src/"
EXTRA_INCLUDES="-I$VEINS_PATH -L$VEINS_PATH"

# ensure the working dir is ready
mkdir -p work

# generate test files
opp_test gen -v $TESTS

# build test files
(cd work; opp_makemake -f --deep -o work $EXTRA_INCLUDES ; make -j4 MODE=debug)

# run tests
opp_test run -v -p work_dbg $TESTS
