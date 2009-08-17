DeviceTestAll
-------------

Combines deviceTestMulti and deviceTestAll.  Uses three instances of
the DeviceDuty and DeviceDutySimple devices, one of which has no
continuous CURRENT draw, only discrete ENERGY draw from the Battery.

Run each test individually in order.

./deviceTest -r $i 

Compare output files with the valid/ directory.

for f in *.vec *.sca ; do
diff $f valid/$f
done

DETAILS

Run 1 - just the one run