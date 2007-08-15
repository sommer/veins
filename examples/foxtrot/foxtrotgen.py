from sys import stdout
from random import uniform

out = stdout
print >>out,"""
[General]
debug-on-errors=1
;ini-warnings = true
network = baseSim
#random-seed = 13
baseSim-time-limit = 50s
#perform-gc = true

[Tkenv]
;bitmap-path="../../bitmaps"
default-run=1
use-mainwindow = yes
print-banners = yes
slowexec-delay = 300ms
update-freq-fast = 10
update-freq-express = 100
breakpoints-enabled = yes

[Cmdenv]
runs-to-execute = 1
event-banners = no
module-messages = yes
; verbose-snrSimulation = no
; verbose-snrSimulation = yes

[DisplayStrings]


[Parameters]





# uncomment to enable debug messages for all modules

##############################################################################
#       Parameters for the propagationmodel                                    #
##############################################################################
baseSim.prop = "UnitDisk"
baseSim.propagationmodel.radioRange = 10

# junk numbers 
baseSim.propagationmodel.pMax = 10
baseSim.propagationmodel.sat = 10
baseSim.propagationmodel.alpha = 10
baseSim.propagationmodel.carrierFrequency = 10

baseSim.world.useTorus = 0
baseSim.world.use2D = 1

baseSim.node[*].applType = "FoxApplLayer"
#baseSim.node[*].aggType = "BaseAggLayer"
#baseSim.node[*].aggType = "FoxtrotPatterned"
baseSim.node[*].aggType = "Foxtrot"
baseSim.node[*].agg.headerLength=0
baseSim.node[*].agg.maxLatency=5
baseSim.node[*].agg.debug = 1

##############################################################################
#       Parameters for the utility Module                                   #
##############################################################################
# starting position for the nodes "-1" means random staring point

##############################################################################
#       Parameters for the entire simulation                                 #
##############################################################################
"""

size = (500,350)

print "baseSim.playgroundSizeX = %d"%size[0]
print "baseSim.playgroundSizeY = %d"%size[1]
print "baseSim.playgroundSizeZ = 0"

count = 3
print "baseSim.numNodes = %d"%count

for i in range(count):
	if i == 0:
		print "baseSim.node[%d].isSink = True"%i
	else:
		print "baseSim.node[%d].isSink = False"%i
	print "baseSim.node[%d].mobility.x = %d"%(i,i*10)
	print "baseSim.node[%d].mobility.y = 0"%i
	#print "baseSim.node[%d].mobility.x = %f"%(i,uniform(0,100))
	#print "baseSim.node[%d].mobility.y = %f"%(i,uniform(0,100))
	print "baseSim.node[%d].mobility.z = 0"%i

print >>out,"""

##############################################################################
#       Parameters for the Host                                              #
##############################################################################

##############################################################################
#       Parameters for the Application Layer                                 #
##############################################################################

# debug switch
baseSim.node[*].appl.debug = 1
baseSim.node[*].appl.headerLength=12
baseSim.node[*].appl.burstSize=3


##############################################################################
#       Parameters for the Network Layer                                     #
##############################################################################
baseSim.node[*].net.headerLength=24
baseSim.node[*].net.debug = 1
baseSim.node[*].net.stats = 1
baseSim.node[*].routingType = "BasicSinkRouting";
baseSim.node[*].net.autoForward = false

##############################################################################
#       Parameters for ARP
##############################################################################
baseSim.node[*].arp.debug = 1


##############################################################################
#       Parameters for the MAC Layer                                         #
##############################################################################

#baseSim.node[*].nic.mac.debug = 1
baseSim.node[*].nic.mac.coreDebug = 1
baseSim.node[*].nic.mac.queueLength=5
baseSim.node[*].nic.mac.headerLength=24
baseSim.node[*].nic.mac.defaultChannel = 0
baseSim.node[*].nic.mac.bitrate = 15360


##############################################################################
#       Parameters for the radio
##############################################################################

baseSim.node[*].nic.radio.swSleep = 0
baseSim.node[*].nic.radio.swSend = 0.001
baseSim.node[*].nic.radio.swRecv = 0.003
baseSim.node[*].nic.radio.debug = 0


##############################################################################
#       Parameters for the Physical Layer                                    #
##############################################################################

# debug switch
baseSim.node[*].nic.phyType = "CollisionsPhy"
baseSim.node[*].nic.phy.debug = 1
baseSim.node[*].nic.phy.publishRSSIAlways = 0
baseSim.node[*].nic.phy.headerLength=16
# transmission power [mW]
baseSim.node[*].nic.phy.transmitterPower=3

baseSim.node[*].nic.phy.carrierFrequency=868E+6
baseSim.node[*].nic.phy.thermalNoise=-120
baseSim.node[*].nic.phy.sensitivity=-110
baseSim.node[*].nic.phy.pathLossAlpha=3.5
baseSim.node[*].nic.phy.bitrate=1000

baseSim.node[*].nic.decider.debug = 1
baseSim.node[*].nic.decider.snrThresholdLevel=10;[dB]

##############################################################################
#       Parameters for the Utility Layer                                     #
##############################################################################
baseSim.node[*].utility.baseDebug=0

**.debug = 1
**.coreDebug = 1"""
