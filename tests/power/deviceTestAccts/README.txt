DeviceTestAccts
---------------

Similar to deviceTest, but uses the two-phase DutyCycle device, which
assigns each of the two ON periods to a different activity.  Both
wakeups are assigned to a third activity.

Run each test individually in order.

./deviceTest -r $i 

Compare output files with the valid/ directory.

for f in *.vec *.sca ; do
diff $f valid/$f
done


DETAILS

Run 1 -  sim-time < lifetime

Run 2 -  sim-time >  lifetime