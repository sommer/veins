#!/usr/bin/python

from pylab import *
from subprocess import *
from os import listdir, remove, rename
import fileinput,re, random, sys, getopt
from shutil import copyfile

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


# configuration
nbPacketsPerSource = 5
nbSources = 5
nbRunsPerConfig = 3
payloadSize = 100 # 100 bytes
phyrng = 6
macrng = 1
apprng = 1

if doGenerate:
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
  fruns.write("phySim.numHosts = 11\n")
  fruns.write("include omnetpp_positions.ini\n\n\n")

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
if doRun:
  for currRun in range(1, nbSources*nbRunsPerConfig+1):
  	check_call(["./ieee802154a", "-r " + str(currRun)])
  rename("omnetpp.sca", "out/mui/data/omnetpp.sca")
	
if doAnalyze:
  errBits = zeros( (nbSources*nbRunsPerConfig) )
  recvBits = zeros( (nbSources*nbRunsPerConfig) )
  bers = zeros( (nbSources*nbRunsPerConfig) )
  pLatency = re.compile(r"[\d.]+\s[\d.]+\s([\d.]+)")
  currRun = 0
  
  pRun = re.compile(r"run\s(\d+)")
  pErrBits = re.compile(r"Erroneous\sbits[^\d]+(\d+)")
  pRecvBits = re.compile(r"Total\sreceived\sbits[^\d]+(\d+)")

  cRun = 0
  cRecvBits = 0
  cErrBits = 0
  nbInterferers = 0
  cfgErrs = zeros((nbSources+1))
  cfgRecvBits = zeros((nbSources+1))
  cfgBers = zeros((nbSources+1))
  
  for line in fileinput.input("out/mui/data/omnetpp.sca"):
	mRun = re.search(pRun, line)
	if mRun is not None:
		currRun = int(mRun.group(1))
	else:
		mRecvBits = re.search(pRecvBits, line)
		if mRecvBits is not None:
			recvBits[currRun-1] = float(mRecvBits.group(1))
			cRecvBits = cRecvBits + recvBits[currRun-1]
		else:
			mErrBits = re.search(pErrBits, line)
			if mErrBits is not None:
				errBits[currRun-1] = float(mErrBits.group(1))
				cErrBits = cErrBits + errBits[currRun-1]
				cRun = cRun + 1
				if cRun == nbRunsPerConfig:
					print nbInterferers
					cfgErrs[nbInterferers] = cErrBits
					cfgRecvBits[nbInterferers] = cRecvBits
					cfgBers[nbInterferers] = cfgErrs[nbInterferers] / cfgRecvBits[nbInterferers]
					nbInterferers = nbInterferers + 1
					cRun = 0
					cErrBits = 0
					cRecvBits = 0
					
				
				
  bers = errBits / recvBits


  # Plot results
  axis()
  grid(True)
  hold(True)

  # simulation results

  print cfgBers[:nbInterferers]

  semilogy(arange(1,nbInterferers+1), cfgBers[:nbInterferers])
  
  xlabel('Concurrent transmissions')
  ylabel('Log10(BER)')


  title('Robustness to Multiple Access Interference')
  savefig('out/mui/figures/mui.png', dpi=300)
  show()

