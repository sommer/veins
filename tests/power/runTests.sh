echo ==== Running test ====
for f in deviceTest deviceTestMulti deviceTestAccts deviceTestAll
do
 echo --- $f ---
 cd $f
 sh runTests.sh
 cd ..
done
echo ==== Checking results of tests ====
for f in deviceTest deviceTestMulti deviceTestAccts deviceTestAll
do
 echo --- $f ---
 cd $f
 sh ../checkResults.sh
 cd ..
done
