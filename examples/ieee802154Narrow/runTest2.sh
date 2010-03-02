#!/bin/sh

rm -f results/Test2*
./ieee802154Narrow -c Test2-A -u Cmdenv 
./ieee802154Narrow -c Test2-B -u Cmdenv 
./ieee802154Narrow -c Test2-C -u Cmdenv 