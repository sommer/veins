#!/bin/sh
rm -f results/Test1*
opp_runall $1 ./ieee802154Narrow -c Test1-A -u Cmdenv -r 0..19
opp_runall $1 ./ieee802154Narrow -c Test1-B -u Cmdenv -r 0..19
opp_runall $1 ./ieee802154Narrow -c Test1-C -u Cmdenv -r 0..19
opp_runall $1 ./ieee802154Narrow -c Test1-D -u Cmdenv -r 0..19
opp_runall $1 ./ieee802154Narrow -c Test1-E -u Cmdenv -r 0..19
