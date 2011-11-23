#!/bin/sh

rm -f results/Test2*
opp_runall $1 ./ieee802154Narrow -c Test2-A -u Cmdenv -r 0..38
opp_runall $1 ./ieee802154Narrow -c Test2-B -u Cmdenv -r 0..38
opp_runall $1 ./ieee802154Narrow -c Test2-C -u Cmdenv -r 0..38
