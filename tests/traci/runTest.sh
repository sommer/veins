#/bin/bash

./traci $@ | egrep -i "^(Pass|FAIL)"

