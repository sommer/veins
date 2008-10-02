#/bin/bash
echo "---------------- make all tests --------------"
make

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
echo -------------------Coord----------------------
cd ../coord
./runTest.sh
echo ------------------Mapping---------------------
cd ../mapping
./runTest.sh
echo ----------------ChannelInfo-------------------
cd ../channelInfo
./runTest.sh
echo -----------------RadioState-------------------
cd ../radioState
./runTest.sh
