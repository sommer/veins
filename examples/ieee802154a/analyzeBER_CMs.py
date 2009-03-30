#!/usr/bin/python

#
# Compare BERs with various channel models

from pylab import *
from subprocess import *
from os import listdir, remove, rename
import fileinput,re, random, sys, getopt
from shutil import copyfile

# configuration
nbPackets = 5
nbRunsPerConfig = 2
payloadSize = 100 #  bytes
distances = array( [7, 50, 100])
phyrng = 6
macrng = 1
apprng = 1

channels = array(['channel-CM1.xml', 'channel-CM2.xml', 'channel-CM3.xml', 'channel-CM5.xml', 'channel-CM6.xml', 'channel-CM7.xml'])
legendes = array(['CM1 Residential LOS', 'CM2 - Residential NLOS', 'CM3 - Office LOS', 'CM5 - Outdoor LOS', 'CM6 - Outdoor NLOS', 'CM7 - Open Outdoor NLOS'])

chans = array([1, 2, 3, 5, 6, 7])

# Command-line options
try:                                
  opts, args = getopt.getopt(sys.argv[1:], "gra", ["generate", "run", "analyze"])
except getopt.GetoptError:           
  print "This script generates configuration files for omnet++, run the simulation, and analyze the results."
  print "It accepts the following options:"
  print " -g (--generate) to generate the configuration files."
  print " -r (--run)      to run the simulations."
  print " -a (--analyze) to analyze the simulation results."
  sys.exit(2)

doGenerate = False
doRun = False
doAnalyze = False
for opt, arg in opts:
  if opt in ("-g", "--generate"):
      doGenerate = True
  if opt in ("-r", "--run"):
      doRun = True
  if opt in ("-a", "--analyze"):
      doAnalyze = True

# If no options given, do everything
if not(doGenerate | doRun | doAnalyze):
    doGenerate = True
    doRun = True
    doAnalyze = True
 
print doGenerate
print doRun
print doAnalyze
# generation
## RNG Configuration

if doGenerate:
  frng = open("omnetpp_rng.ini", "w")
  frng.write("\n\n")
  frng.write("num-rngs=" + str( 2 * (phyrng+macrng+apprng) ) + "\n\n")
  frng.write("\n\n## RNG Configuration ##\n\n")
  rngID=1
  for node in range(2):
    for rng in range(phyrng):
      frng.write("phySim.node[" + str(node) +"].nic.phy.rng-" + str(rng) + " = " + str(node*(phyrng+macrng+apprng)+rng) + "\n")
    for rng in range(macrng):
      frng.write("phySim.node[" + str(node) +"].nic.mac.rng-" + str(rng) + " = " + str(node*(phyrng+macrng+apprng)+phyrng+rng) + "\n")
    for rng in range(apprng):
      frng.write("phySim.node[" + str(node) +"].app.rng-" + str(rng) + " = " + str(node*(phyrng+macrng+apprng)+phyrng+macrng+rng) + "\n")
    frng.write("\n")
  frng.close()
  
  ## Runs configuration
  
  fruns = open("omnetpp_runs.ini", "w")
  run = 0
  fruns.write("\n\n")
  fruns.write("phySim.numHosts = 2\n\n\n")

  # application packet rate
  fruns.write("phySim.node[0].app.nbPackets = 0\n")
  fruns.write('phySim.node[1].app.nbPackets = %d\n' % nbPackets)
  fruns.write("phySim.node[*].app.flood = true\n")
  fruns.write("phySim.node[*].app.stats = true\n")
  fruns.write("phySim.node[*].app.payloadSize = " + str(payloadSize) + "\n")
  fruns.write("\n")
  fruns.write("################ Mobility parameters #####################\n")
  fruns.write("phySim.node[0].mobility.x = 100\n")
  fruns.write("phySim.node[0].mobility.y = 100\n")
  fruns.write("phySim.node[0].mobility.z = 100\n")
  fruns.write('phySim.node[1].mobility.y = 100\n')
  fruns.write('phySim.node[1].mobility.z = 100\n')
  fruns.write("\n\n")

  for d in range(distances.size):
    for run in range(nbRunsPerConfig):
      fruns.write("[Run " + str(d*nbRunsPerConfig+run+1) + "]\n")
      dx = 100 + distances[d]
      fruns.write('phySim.node[1].mobility.x = %d\n\n' % dx)
  
  fruns.close()



# Loop over channels

if doRun:
  for channel in range(channels.size):
    copyfile(channels[channel], 'channel.xml')
   
    # clean up destination directory
    datapath = "./out/ber-CM/data/CM%d/" % chans[channel]
    oldData = listdir(datapath)
    for datafile in oldData:
      remove(datapath + datafile)

    # execute runs
    for currRun in range(1, distances.size*nbRunsPerConfig+1):
      check_call(["./ieee802154a", "-r " + str(currRun)])
      rename('omnetpp.sca', 'out/ber-CM//data/CM%d/omnetpp-%d.sca' % (chans[channel], currRun))


if doAnalyze:
  bers = zeros( (channels.size, distances.size) )
  for channel in range(channels.size):
    errBits = zeros( (distances.size) )
    recvBits = zeros( (distances.size) )

    currRun = 0
    pErrBits = re.compile(r"Erroneous\sbits[^\d]+(\d+)")
    pRecvBits = re.compile(r"Total\sreceived\sbits[^\d]+(\d+)")

    RecvBits = 0
    cErrBits = 0
    nbInterferers = 0

    for d in range(distances.size):
      nbItems = 0
      for localRun in range(nbRunsPerConfig):
        run = d*nbRunsPerConfig+localRun+1
        for line in fileinput.input('out/ber-CM/data/CM%d/omnetpp-%d.sca' % (chans[channel], run)):
		  mRecvBits = re.search(pRecvBits, line)
		  if mRecvBits is not None:
		  	recvBits[d] = recvBits[d] + float(mRecvBits.group(1))
		  mErrBits = re.search(pErrBits, line)
		  if mErrBits is not None:
				errBits[d] = errBits[d] + float(mErrBits.group(1))
				
    bers[channel] = errBits / recvBits


  # Plot results
  figure()
  axis()
  grid(True)
  hold(True)

# simulation results

  print distances
  print bers

  for channel in range(channels.size):
    loglog(distances, bers[channel], label=legendes[channel])

  xlabel('Distance')
  ylabel('Bit Error Rate')
  savefig('out/ber-CM/figures/ber-cms-nolegend.png', dpi=300)

  figure()
  
  axis()
  grid(True)
  hold(True)

  # simulation results

  print distances
  print bers

  for channel in range(channels.size):
      loglog(distances, bers[channel], label=legendes[channel])

  xlabel('Distance')
  ylabel('Bit Error Rate')
  legend()
  savefig('out/ber-CM/figures/ber-cms-legend.png', dpi=300)

  show()

