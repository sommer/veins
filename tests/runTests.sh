#/bin/bash

if [ -d blackboard ]; then
    echo '----------------Blackboard--------------------'
    ( ( cd blackboard >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d connectionManager ]; then
    echo '-------------ConnectionManager----------------'
    ( ( cd connectionManager >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d baseMobility ]; then
    echo '----------------BaseMobility------------------'
    ( ( cd baseMobility >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d basePhyLayer ]; then
    echo '----------------BasePhyLayer------------------'
    ( ( cd basePhyLayer >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d decider ]; then
    echo '----------------DeciderTest-------------------'
    ( ( cd decider >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d coord ]; then
    echo '-------------------Coord----------------------'
    ( ( cd coord >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d channelInfo ]; then
    echo '----------------ChannelInfo-------------------'
    ( ( cd channelInfo >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d radioState ]; then
    echo '-----------------RadioState-------------------'
    ( ( cd radioState >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d nicTest ]; then
    echo '---------------NICTests(80211)----------------'
    ( ( cd nicTest >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d mapping ]; then
    echo '---------Mapping (may take a while)-----------'
    ( ( cd mapping >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d traci ]; then
    echo '---------TraCI (may take a while)-----------'
    ( ( cd traci >/dev/null 2>&1 && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
