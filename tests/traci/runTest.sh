#!/bin/sh

./traci -u Cmdenv "$@" | egrep -i "^(Pass|FAIL)"
