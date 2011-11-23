#!/bin/bash

BasePath="$( cd $(dirname $0); pwd )"

echo '========= Running test ============'
for f in deviceTest deviceTestMulti deviceTestAccts deviceTestAll
do
 if [ -d "${BasePath}/$f" -a -f "${BasePath}/$f/runTests.sh" ]; then
  echo "--- $f ---"
  ( cd "${BasePath}/$f" && \
    ./runTests.sh >run.log )
 fi
done
echo '==== Checking results of tests ===='
for f in deviceTest deviceTestMulti deviceTestAccts deviceTestAll
do
 if [ -d "${BasePath}/$f" -a -f "${BasePath}/checkResults.sh" ]; then
  echo "--- $f ---"
  ( cd "${BasePath}/$f" && \
    ../checkResults.sh )
 fi
done
