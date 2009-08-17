DeviceTestMulti
---------------

Similar to deviceTest, but the Host now has two DutyCycleSimple devices,
each with different parameters.  

Run each test individually in order.

./deviceTest -r $i 

Compare output files with the valid/ directory.

for f in *.vec *.sca ; do
diff $f valid/$f
done


DETAILS

The 2nd device has period half of the 1st device. 
 
Run 1 - 1st and 2nd devices turn on at the same time, battery fails

Run 2  - 2nd device turns off while 1st device is on

Run 3 - 1st device turns off while 2nd device is off, battery fails

Run 4 - 2nd device turns on while first device is off


