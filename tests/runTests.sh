#/bin/bash

if [ -d blackboard ]; then
    echo '----------------Blackboard--------------------'
    ( ( cd blackboard && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d connectionManager ]; then
    echo '-------------ConnectionManager----------------'
    ( ( cd connectionManager && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d baseMobility ]; then
    echo '----------------BaseMobility------------------'
    ( ( cd baseMobility && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d basePhyLayer ]; then
    echo '----------------BasePhyLayer------------------'
    ( ( cd basePhyLayer && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d decider ]; then
    echo '----------------DeciderTest-------------------'
    ( ( cd decider && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d coord ]; then
    echo '-------------------Coord----------------------'
    ( ( cd coord && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d channelInfo ]; then
    echo '----------------ChannelInfo-------------------'
    ( ( cd channelInfo && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d radioState ]; then
    echo '-----------------RadioState-------------------'
    ( ( cd radioState && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d nicTest ]; then
    echo '---------------NICTests(80211)----------------'
    ( ( cd nicTest && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
if [ -d mapping ]; then
    echo '---------Mapping (may take a while)-----------'
    ( ( cd mapping && \
    ./runTest.sh $1 ) && echo "PASSED" ) || echo "FAILED"
fi
