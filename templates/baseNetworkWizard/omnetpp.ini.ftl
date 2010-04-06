[General]
cmdenv-express-mode = true
network = ${targetTypeName}


##########################################################
#			Simulation parameters                        #
##########################################################
**.**.coreDebug = false
**.playgroundSizeX = 300m
**.playgroundSizeY = 300m
**.playgroundSizeZ = 300m
**.numNodes = 5

##########################################################
#			WorldUtility parameters                      #
##########################################################
**.world.useTorus = false
**.world.use2D = false

<#if protocolName="802.11">
<#include "80211.ini.fti">
<#elseif protocolName="CSMA using old CSMAMacLayer">
<#include "CSMAMacLayer.ini.fti">
<#else>
<#include "CSMA.ini.fti">
</#if>

################ NETW layer parameters ####################

################ Mobility parameters #####################

**.node[0].mobility.x = 150
**.node[0].mobility.y = 200
**.node[0].mobility.z = 250

**.node[1].mobility.x = 250
**.node[1].mobility.y = 100
**.node[1].mobility.z = 100

**.node[2].mobility.x = 250
**.node[2].mobility.y = 200
**.node[2].mobility.z = 200

**.node[3].mobility.x = 50
**.node[3].mobility.y = 100
**.node[3].mobility.z = 110

**.node[4].mobility.x = 150
**.node[4].mobility.y = 180
**.node[4].mobility.z = 100

**.node[5].mobility.x = 50
**.node[5].mobility.y = 200
**.node[5].mobility.z = 10


**.node[*].applType = "BurstApplLayer"
**.node[*].appl.debug = false
**.node[*].appl.headerLength = 512bit
**.node[*].appl.burstSize = 3

**.node[*].mobType = "ConstSpeedMobility"
**.node[*].mobility.debug = false
**.node[*].mobility.speed = 1mps
**.node[*].mobility.updateInterval = 0.1s

**.node[*].netwType = "BaseNetwLayer"
**.node[*].net.debug = false
**.node[*].net.stats = false
**.node[*].net.headerLength = 32bit









