#/bin/bash



echo ----------------Blackboard--------------------
cd blackboard
./runTest.sh
echo -------------ConnectionManager----------------
cd ../connectionManager
./runTest.sh
echo ----------------BaseMobility------------------
cd ../baseMobility
./runTest.sh
echo ----------------BasePhyLayer------------------
cd ../basePhyLayer
./runTest.sh
echo ----------------DeciderTest------------------
cd ../decider
./runTest.sh
echo -------------------Coord----------------------
cd ../coord
./runTest.sh
echo ----------------ChannelInfo-------------------
cd ../channelInfo
./runTest.sh
echo -----------------RadioState-------------------
cd ../radioState
./runTest.sh
echo "---------Mapping (may take a while)-----------"
cd ../mapping
./runTest.sh
echo "----------TraCI (may take a while)------------"
cd ../traci
./runTest.sh
