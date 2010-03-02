#!/bin/sh

rm -f results/Test1*
./ieee802154Narrow -c Test1-A -u Cmdenv 
./ieee802154Narrow -c Test1-B -u Cmdenv 
./ieee802154Narrow -c Test1-C -u Cmdenv 
./ieee802154Narrow -c Test1-D -u Cmdenv 
./ieee802154Narrow -c Test1-E -u Cmdenv 