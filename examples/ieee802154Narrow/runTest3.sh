#!/bin/sh

rm -f results/Test3*
opp_runall $1 ./ieee802154Narrow -c Test3 -u Cmdenv -r 0..6 &
./ieee802154Narrow -c Test3 -u Cmdenv -r 7