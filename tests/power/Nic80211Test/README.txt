80211Test
---------

Tests of the implementation of the Nic80211Battery for the Energy
Framework.  These are the primary tests of the Energy Framework.

The first seven runs are simple unit tests.  The last three are more
complex, see details below.

Run each test individually in order, i.e.

./80211Test -r $i

The .vec and .sca files should match the corresponding files in the
valid/ directory.  Output of run 8 can also be compared with output
from the AFTest compatibility modules.  Output of run 9 and 10 can be
compared with each other.

for f in *.vec *.sca ; do 
diff $f valid/$f
done

Note that in some cases, the battery consumption parameters are
unrealistically chosen to implement a test case.  See
networks/80211Battery for a an 802.11 simulation.

DETAILS

Run 1-3 are simple two-host single-message tests that check that the
correct CURRENT messages are generated.  

Run 1 - nodes are in range: each host sends and receives one BCAST and
one REPLY

Run 2 - nodes are out of range: each host transmits one BCAST and
receives nothing

Run 3 - nodes are error range: each host transmits one BCAST and
receives an ERROR

Manually, it is easiest to check that the times are correct.  For
reference these are (from instrumented Mac80211):

broadcast 0.00086
rts       0.00028
cts       0.00024
data      0.00086
ack       0.00025

Run 4-7 - are simple three-host bursty traffic scenarios that check
battery depletion and host failure notification by sending very long
bursts and comparing the total number of responses to the burst.  All
runs show that in/outbound frames are properly halted at the NIC; runs
5 and 7 also show that the appl layer cancels pending burst.

Run 4 - all bursts run to completion
Run 5 - host 2 fails halfway through 1st burst (host 0's) 
Run 6 - host 2 fails halfway through 2nd (own) burst 
Run 7 - high idleCurrent; hosts fail in quiescent network


Run 8 - compare output to tests/AFTest

The point of run 8 is to compare the output Nic80211Battery with the
output of the compatibility modules from tests/AFTest (run 2).  The
output should also match the output in the valid/ directory.

Note that the omnetpp.vec output for run 8 is written to
omnetpp_80211.vec.

These cannot be compared directly, since the Nic80211Battery records
additional data.  In the omnetpp.sca file, the Tx times are the same,
but because the NIC80211Battery distinguishes between Rx and Idle, the
sum of these times must be compared to the Rx time in the AFTest
output. 

Because the test is configured to use the same current for idle and
recv, the total energy consumed at each node should be the same.

The convertvec.py script can be used to generate time series data:

../../scripts/convertvec.py -i omnetpp_80211.vec -o battery80211 -p 4

Each file battery_80211/node(i).power can then be compared with the
corresponding file AFTest/battery_80211/node(i).power.  The data still
display a few variations at 4 significant digits.  This is because the
battery-aware 802.11 module generates a quite different sequence of
floating point operations as it depletes the battery.  (Given the very
crude battery model, it is clear that meaningful data is much less
than 4 significant digits in any case.)

for f in battery80211/*.power ; do 
diff $f ../AFTest/$f
done

Run 9-10 - realistic hardware parameters and sanity check

Run the burstApplLayer with burst sizes 10 and 50. It can be seen
directly in omnetpp.sca that each host uses approximately 4 1/2 times
more energy for SEND and RECV in the latter case. The exact ratio
varies due to different RNG sequences for traffic, collisions, overlapping 
broadcasts, etc.

To compare the two cases graphically using gnuplot, generate 
output from the vector files

convertvec.py -i omnet.10.vec -o out_10 -c "#"

convertvec.py -i omnet.50.vec -o out_50 -c "#"

The -c "#" is the comment character for gnuplot (so that it will ignore
column headers in the output).  Then plot the capacity curves for some
pair of hosts, say host 3.

gnuplot
> plot "out_10/node3.power", "out_50/node3.power"

The plots are time (x-axis) vs capacity (y-axis). 

There are clear differences between the two plots, in addition to the
obviously greater energy consumption of the larger burst.

The disparity between the publishTime (250ms) and the resolution (1s)
is visible (it's not really a sensible configuration; the one in
networks/80211Battery is more reasonable).  When the channel is idle,
there are up to four data points published between updates.  This is
particularly obvious at the end, once both bursts are complete, but
there are also some quiescent periods in the middle of the lower
traffic scenario.  It's also clear that the longer burst also takes a
little longer to complete.

