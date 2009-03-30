#!/usr/bin/python
#
# copyright 2008 CSEM SA, Jerome Rousselot <jerome.rousselot@csem.ch>
#
# Compare BERs with various channel models and receiver settings
#
#

from pylab import *
from matplotlib.font_manager import *
from subprocess import *
from os import listdir, remove, rename
import fileinput,re, random, sys, getopt, os, glob
from shutil import copyfile

#
# Configuration (module-wide parameters)
#

experiment = "BERDistance"
repeat = 3
resultsPath="results/BERDistance-*.sca"

#channels = array(['channel-CM1.xml', 'channel-CM2.xml', 'channel-CM3.xml', 'channel-CM5.xml', 'channel-CM6.xml', 'channel-CM7.xml'])
#legendes = array(['CM1 Residential LOS', 'CM2 - Residential NLOS', 'CM3 - Office LOS', 'CM5 - Outdoor LOS', 'CM6 - Outdoor NLOS', 'CM7 - Open Outdoor NLOS'])
#chans = array([1, 2, 3, 5, 6, 7])

styles = ['-g^', '-k+', '-rs', '-bo']

class Usage(Exception):
  def __init__(self, msg):
    self.msg = msg

def main(argv=None):
  if argv is None:
    argv = sys.argv
  # Command-line options
  try:                                
    try:
      opts, args = getopt.getopt(sys.argv[1:], "gra", ["generate", "run", "analyze"])
    except getopt.GetoptError, msg:   
      raise Usage(msg)        
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
      doGenerate = False
      doRun = False
      doAnalyze = True
    
    print "Generate configuration: %d" % doGenerate
    print "Execute runs: %d" % doRun
    print "Analyze results: %d" % doAnalyze
    
#    if doGenerate:
#      generate()
#    if doRun:
#      run()
    if doAnalyze:
      analyze()

  except Usage, err:
    print "This script generates configuration files for omnet++, run the simulation, and analyze the results."
    print "It accepts the following options:"
    print " -g (--generate) to generate the configuration files."
    print " -r (--run)      to run the simulations."
    print " -a (--analyze) to analyze the simulation results."
    sys.exit(2)
  

# generation
## RNG Configuration

def generate():
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
      fruns.write('phySim.node[1].mobility.x = %f\n\n' % dx)
  
  fruns.close()



# Loop over channels and deciders
def run():
  for receiver in range(receivers.size):
    copyfile(receivers[receiver], 'decider.xml')
    for channel in range(channels.size):
      copyfile(channels[channel], 'channel.xml')
      # clean up destination directory
      datapath = "./out/ber-cm-sens/data/sens%sdB-%s/" % (sens[receiver], chans[channel])
      if os.path.exists(datapath):
        oldData = listdir(datapath)
        for datafile in oldData:
          remove(datapath + datafile)
      else:
        os.makedirs(datapath)
      # execute runs
      for currRun in range(1, distances.size*nbRunsPerConfig+1):
        check_call(["./ieee802154a", "-r " + str(currRun)])
        rename('omnetpp.sca', 'out/ber-cm-sens/data/sens%sdB-%s/omnetpp-%d.sca' % (sens[receiver], chans[channel], currRun))
        #rename('omnetpp.vec', 'out/ber-cm-sens/data/sens%sdB-%s/omnetpp-%d.vec' % (sens[receiver], chans[channel], currRun))


def analyze():
  distancesSet = set()
  receivers = set()
  channels = set()
  # store bit error rates for each evaluated configuration
  bers = dict()
  receiver = "3dB"
  receivers.add(receiver)
  errBits = dict()
  errBits[receiver] = dict()
  recvBits = dict()
  recvBits[receiver] = dict()
  currRun = 0
  pErrBits = re.compile(r"Erroneous\sbits[^\d]+(\d+)")
  pRecvBits = re.compile(r"Total\sreceived\sbits[^\d]+(\d+)")
  pIterationvars2 = re.compile(r"iterationvars2")
  pDistance = re.compile(r"\$distance=(\s*\d+)")
  pChannel = re.compile(r"\$Channel=([\w-]+)")
  pReceiver = re.compile(r"\$Receiver=([\w-]+)")
  pRepetition = re.compile(r"\$repetition=(\d+)")
  RecvBits = 0
  cErrBits = 0
  for resultsFile in glob.glob(resultsPath):
#      print "File %s: " % resultsFile
    distance = -1
    repetition = -1
    channel = ""
    for line in fileinput.input(resultsFile):
      if distance == -1:
        mIterationvars2 = re.search(pIterationvars2, line)
        if mIterationvars2 is not None:
	  mDistance = re.search(pDistance, line)
	  mChannel = re.search(pChannel, line)
	  mRepetition = re.search(pRepetition, line)
	  if mDistance is None or mChannel is None or mRepetition is None:
  	    raise Exception("Could not parse iterationvars2 line of file %s." % resultsFile) 
	  distance = float(mDistance.group(1))
	  distancesSet.add(distance)
	  channel = mChannel.group(1)
	  channels.add(channel)
	  if not errBits[receiver].has_key(channel):
	    errBits[receiver][channel] = dict()
	    recvBits[receiver][channel] = dict()
	  repetition = float(mRepetition.group(1))
	  #print "  ch=%s, repetition = %s, distance = %s" % (channel, repetition, distance)
      else: # config part of file already parsed, search for data
	mRecvBits = re.search(pRecvBits, line)
	if mRecvBits is not None:
	  if recvBits[receiver][channel].has_key(distance):
	    recvBits[receiver][channel][distance] = recvBits[receiver][channel][distance] + float(mRecvBits.group(1))
	  else:
	    recvBits[receiver][channel][distance] = float(mRecvBits.group(1))
	else:
	  mErrBits = re.search(pErrBits, line)
	  if mErrBits is not None:
	    if errBits[receiver][channel].has_key(distance):
	      errBits[receiver][channel][distance] = errBits[receiver][channel][distance] + float(mErrBits.group(1))
	    else:
	      errBits[receiver][channel][distance] = float(mErrBits.group(1))


  distances = list(distancesSet)
  distances.sort()
  print distances

  for receiver in receivers:
    for channel in channels: 
      errs = array( [ errBits[receiver][channel][d] for d in sorted(errBits[receiver][channel].keys()) ] )
      recvs = array( [ recvBits[receiver][channel][d] for d in sorted(recvBits[receiver][channel].keys()) ] )
      bers[receiver, channel] = errs[recvs > 0] / recvs[recvs > 0]
      print "BER for receiver %s, channel %s: " % (receiver, channel)
      print errs[recvs > 0]
      print recvs[recvs > 0]
      print bers[receiver, channel]


  # Plot results
  figure()
  axis()
  grid(True)
  hold(True)
  
  # simulation results
  
#  print distances
#  print bers

  currLine = 0
  for receiver in receivers:
    for channel in channels:
     print bers[receiver, channel]
     legende = "%s %s" % (receiver, channel)
     loglog(distances[:len(bers[receiver, channel]):], bers[receiver, channel], styles[currLine], label=legende, markersize=10)
     currLine = currLine + 1
  #a = gca()
  #a.yaxis.major.formatter.set_powerlimits((0.01,100))
  #a.ticklabel_format(style='sci', scilimits=[0.01, 1000], axis='y')
  xlabel('Distance')
  ylabel('Bit Error Rate')
  legend(loc='upper left', prop = FontProperties( size="smaller" ))
  savefig('figures/BERDistance/ber-cms-legend.png', dpi=300)

  show()


if __name__ == "__main__":
    sys.exit(main())



