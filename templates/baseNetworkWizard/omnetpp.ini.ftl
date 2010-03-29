[General]
cmdenv-config-name = perftest
cmdenv-express-mode = true
network = ${targetTypeName}


##########################################################
#			Simulation parameters                        #
##########################################################
tkenv-default-config = 
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

##########################################################
#			         channel parameters                  #
##########################################################
**.connectionManager.sendDirect = false
**.connectionManager.pMax = 100mW
**.connectionManager.sat = -84dBm
**.connectionManager.alpha = 3.0
**.connectionManager.carrierFrequency = 2.412e+9Hz


################ PhyLayer parameters #####################
**.node[*].nic.phy.usePropagationDelay = false
**.node[*].nic.phy.thermalNoise = -100dBm
**.node[*].nic.phy.useThermalNoise = true
        
**.node[*].nic.phy.analogueModels = xmldoc("config.xml")
**.node[*].nic.phy.decider = xmldoc("config.xml")

**.node[*].nic.phy.timeRXToTX = 0.00021s
**.node[*].nic.phy.timeRXToSleep = 0.000031s

**.node[*].nic.phy.timeTXToRX = 0.00012s
**.node[*].nic.phy.timeTXToSleep = 0.000032s

**.node[*].nic.phy.timeSleepToRX = 0.000102s
**.node[*].nic.phy.timeSleepToTX = 0.000203s

**.node[*].nic.phy.sensitivity = -80dBm
**.node[*].nic.phy.maxTXPower = 100.0mW

**.node[*].nic.phy.initialRadioState = 0

################ MAC layer parameters ####################
**.node[*].nic.mac.queueLength = 5
**.node[*].nic.mac.headerLength = 24bit
**.node[*].nic.mac.slotDuration = 0.04s
**.node[*].nic.mac.difs = 0.0005s
**.node[*].nic.mac.maxTxAttempts = 14
**.node[*].nic.mac.defaultChannel = 0
**.node[*].nic.mac.bitrate = 15360bps
**.node[*].nic.mac.contentionWindow = 20
**.node[*].nic.mac.txPower = 100mW  # [mW]

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









