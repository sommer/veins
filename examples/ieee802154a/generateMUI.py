#!/usr/bin/python

from pylab import *
from subprocess import *
from os import rename
import fileinput,re

# configuration
nbPacketsPerSource = 100
nbSources = 10
nbRunsPerConfig = 3
payloadSize = 16 # 16 bytes
phyrng = 6
macrng = 1
apprng = 1

# generation
## RNG Configuration

frng = open("omnetpp_rng.ini", "w")
frng.write("\n\n")
frng.write("num-rngs=" + str( (nbSources+1) * (phyrng+macrng+apprng) ) + "\n\n")
frng.write("\n\n## RNG Configuration ##\n\n")
rngID=1
for node in range(nbSources+1):
  for rng in range(phyrng):
    frng.write("phySim.node[" + str(node) +"].nic.phy.rng-" + str(rng) + " = " + str(node*(phyrng+macrng+apprng)+rng) + "\n")
  for rng in range(macrng):
    frng.write("phySim.node[" + str(node) +"].nic.mac.rng-" + str(rng) + " = " + str(node*(phyrng+macrng+apprng)+phyrng+rng) + "\n")
  for rng in range(apprng):
    frng.write("phySim.node[" + str(node) +"].app.rng-" + str(rng) + " = " + str(node*(phyrng+macrng+apprng)+phyrng+macrng+rng) + "\n")
  frng.write("\n")

frng.close()

## Runs configuration

nodes = arange(1, nbSources+1, 1)
fruns = open("omnetpp_runs.ini", "w")
run = 0

fruns.write("\n\n")

# application packet rate
fruns.write("phySim.node[0].app.nbPackets = 0\n")
fruns.write("phySim.node[*].app.flood = true\n")
fruns.write("phySim.node[*].app.stats = false\n")
fruns.write("phySim.node[*].app.trace = false\n")
fruns.write("phySim.node[*].app.debug = false\n")
fruns.write("phySim.node[*].app.payloadSize = " + str(payloadSize) + "\n")
fruns.write("\n\n")

for source in range(nbSources):
  for run in range(nbRunsPerConfig):
    fruns.write("[Run " + str(source*nbRunsPerConfig+run+1) + "]\n")
    for i in range(source+1):
      fruns.write("phySim.node[" + str(i+1) + "].app.nbPackets = " + str(nbPacketsPerSource) + "\n")
    fruns.write("phySim.node[*].app.nbPackets = 0\n\n")	

fruns.close()


# execute runs

for currRun in range(1, nbSources*nbRunsPerConfig):
	check_call(["./ieee802154a", "-r " + str(currRun)])

rename("omnetpp.sca", "out/mui/omnetpp-" + str(currRun) + ".sca")


