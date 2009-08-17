Device Test
-----------

Fundamental test case, used for basic unit testing.  The Host includes
only the Battery and the DutyCycleSimple device.

The tests go through various relations between events and parameters
(i.e.  CURRENT messages at duty cycle boundary, resolution interval
updates of battery capacity, host failure).  Also checks various
combinations of data collection parameters.  See below and omnetpp.ini
for details.

Run each test individually in order.

./deviceTest -r $i 

Compare output files with the valid/ directory.  Note that Run 13 and
14 do not have an output vector, only omnetpp.sca.

for f in *.vec *.sca ; do 
diff $f valid/$f
done


DETAILS

Run 1 - resolution < cycle, aligned to cycle boundary, battery fails at cycle boundary

Run 2 - resolution < cycle, not aligned to cycle boundary, battery fails at cycle boundary

Run 3 - resolution >> cycle (not a sensible configuration), note that failure is detected late

Run 4 - initial capacity < 1.0, battery fails mid cycle

Run 5 - sim-time ends before battery fails, not aligned to resolution

Run 6 - sim-time ends before battery fails, aligned to resolution

Run 7 - time series with publishTime only, time > resolution

Run 8 - time series with publishTime only, time <  resolution

Run 9 - time series with publishDelta only, delta large

Run 10 - time series with publishDelta only, delta small

Run 11 - check estimate vector

Run 12 - check estimate vector, initial capacity < 1.0  (matches residual)

Run 13 - turn off detail and time series

Run 14 - turn off time series

