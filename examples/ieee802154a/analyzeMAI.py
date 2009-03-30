#!/usr/bin/python
#
# copyright 2008 CSEM SA, Jerome Rousselot <jerome.rousselot@csem.ch>
#
# Compare BERs with various channel models and receiver settings
# and several interferers
#

from pylab import *
from subprocess import *
from os import listdir, remove, rename
import fileinput,re, random, sys, getopt, os, glob
from shutil import copyfile


class Usage(Exception):
  def __init__(self, msg):
    self.msg = msg

def main(argv=None):
  if argv is None:
    argv = sys.argv
  # Command-line options
  try:                                
    try:
      opts, args = getopt.getopt(sys.argv[1:], "c:agm:M:n:s:", ["config=", "analyze", "generate", "rmin=", "rmax=", "step=", "nbNodes="])
    except getopt.GetoptError, msg:   
      raise Usage(msg)        
    config = False
    doAnalyze = False
    doGenerate = False
    nbNodes = 0
    rmin = 0
    rmax = 0
    rstep = 0
    for opt, arg in opts:
      if opt in ("-c", "--config"):
	config = True
        experiment = arg
      if opt in ("-a", "--analyze"):
	doAnalyze = True
      if opt in ("-g", "--generate"):
        doGenerate = True
      if opt in ("-n", "--nbNodes"):
	nbNodes = int(arg)
      if opt in ("-m", "--rmin"):
	rmin = int(arg)
      if opt in ("-M", "--rmax"):
	rmax = int(arg)
      if opt in ("-s", "--rstep"):
	rstep = int(arg)
      
    # show usage message if errors in options
    if not config:
      raise Usage("Missing mandatory argument.")
    #if doGenerate and nbNodes == 0:
    #  raise Usage("Invalid number of nodes.")
    if not doGenerate and not doAnalyze:
      raise Usage("Please tell me what to do.")

    if doAnalyze:
      analyze(experiment)
    
    if doGenerate:
      generate(experiment, nbNodes, rmin, rmax, rstep)

  except Usage, err:
    print "\n\nError in command-line options: %s" % err
    print "\nThis script analyzes results of an omnet++ 4 simulation using IEEE802154A UWB NIC model for multiple access interference."
    print "It accepts the following options:\n"
    print " -c (--config) to select which configuration to analyze (as defined in omnetpp.ini). This is a mandatory parameter."
    print " -a (--analyze) to analyze the simulation results.\n"
    print " -g (--generate) to generate a configuration file for the selected configuration.\n"
    sys.exit(2)
  
# Place nodes on a circle in a file named $experiment_positions.ini
def generate(experiment, nbNodes, rmin, rmax, step):
  if experiment == "MAICircleR":
    assert nbNodes > 0
    fpos = open("%s_positions.ini" % experiment, "w")
    fpos.write("\n\n")
    fpos.write("[Config %s]\n\n" % experiment)
    fpos.write("phySim.numHosts = ${NbNodes=%d}\n\n\n" % nbNodes)
    size = 2*rmax + 200
    fpos.write("phySim.playgroundSizeX = %d\n" % size)
    fpos.write("phySim.playgroundSizeY = %d\n" % size)
    fpos.write("# The receiver is located at the center of the grid\n")
    size = size / 2
    fpos.write("phySim.node[0].mobility.x = ${O=%d}\n" % size)
    fpos.write("phySim.node[0].mobility.y = this.x\n")
    fpos.write("phySim.node[0].app.nodeAddr = 0\n")
    fpos.write("phySim.node[0].nic.mac.MACAddr = 0\n\n")
    fpos.write("# Everything happens on the same plane\n")
    fpos.write("phySim.node[*].mobility.z = 0\n\n")
    fpos.write("# The source is located close to the receiver\n")
    fpos.write("phySim.node[1].mobility.x = ${O} + 5\n")
    fpos.write("phySim.node[1].mobility.y =  ${O}\n")
    fpos.write("phySim.node[1].app.nodeAddr = 1\n")
    fpos.write("phySim.node[1].nic.mac.MACAddr = 1\n\n")
    fpos.write("# The coordinates of the first jammer also define the iteration variable R\n")
    fpos.write("phySim.node[2].mobility.x = ${O} + ${R=%d..%d step %d}*uniform(-1,1)\n" % (rmin, rmax, step) )
    fpos.write("phySim.node[2].mobility.y =  ${O} + sqrt( (this.x-${O})*(this.x-${O}) - 1 )\n\n") # * (2*intuniform(0,1)-1)\n\n")
    fpos.write("phySim.node[2].app.nodeAddr = 2\n")
    fpos.write("phySim.node[2].nic.mac.MACAddr = 2\n\n")

    for i in range(3, nbNodes):
      fpos.write("phySim.node[%d].mobility.x = ${O} + ${R}*uniform(-1,1)\n" % (i) )
      fpos.write("phySim.node[%d].mobility.y =  ${O} + sqrt( 1-(this.x-${O})*(this.x-${O}) )\n\n" % i) # * (2*intuniform(0,1)-1)\n" % (i))
      fpos.write("phySim.node[%d].app.nodeAddr = %d\n" % (i, i))
      fpos.write("phySim.node[%d].nic.mac.MACAddr = %d\n\n" % (i, i))
    fpos.close()
  elif experiment == "cmpPCM1D":
      #from rmin to rmax, one node every step meters
    fpos = open("%s_positions.ini" % experiment, "w")
    fpos.write("\n\n")
    fpos.write("[Config %s]\n\n" % experiment)
    totalNbNodes = (rmax - rmin)/step + 2
    fpos.write("phySim.numHosts = ${NbNodes=%d}\n\n\n" % totalNbNodes)
    playgroundX = rmax + 200
    fpos.write("phySim.playgroundSizeX = %d\n\n" % playgroundX)
    xpos = rmin
    currnode = 1
    while xpos < rmax:
        currnode = currnode + 1
        fpos.write("phySim.node[%d].mobility.x = %d\n" % (currnode, xpos))
        fpos.write("phySim.node[%d].app.nodeAddr = %d\n" % (currnode, currnode))
        fpos.write("phySim.node[%d].nic.mac.MACAddr =  %d\n" % (currnode, currnode))
        xpos = xpos + step
    fpos.close()
      
    

def parseHeader(fres, experiment): 
  pIterationvars2 = re.compile(r"iterationvars2")
  pNbNodes = re.compile(r"\$NbNodes=(\d+)")
  if experiment == "MAIUnif":
    pR = pNbNodes # to avoid parsing error later
  else:
    pR = re.compile(r"\$R=(\d+)")
  pChannel = re.compile(r"\$Channel=([\w-]+)")
  pReceiver = re.compile(r"\$Receiver=([\w-]+)")
  pRepetition = re.compile(r"\$repetition=(\d+)")
  search = True
  while search:
    line = fres.readline()
    if line == "":
      search = False
      raise Exception("Could not parse header information in file %s." % fres.name)
    else:
      mIterationvars2 = re.search(pIterationvars2, line)
      if mIterationvars2 is not None:
        mNbNodes = re.search(pNbNodes, line)
	mR = re.search(pR, line)
	mChannel = re.search(pChannel, line)
        mReceiver = re.search(pReceiver, line)
	mRepetition = re.search(pRepetition, line)
	if mR is None or mRepetition is None or mNbNodes is None or mChannel is None or mReceiver is None:
  	  raise Exception("Could not parse iterationvars2 line of file %s." % fres.name) 
        nbNodes = float(mNbNodes.group(1))
        if experiment == "MAIUnif":
	  R = 10 # we should store it in file and parse it here
	else:
 	  R = float(mR.group(1))
	channel = mChannel.group(1)
	receiver = mReceiver.group(1)
	repetition = float(mRepetition.group(1))
        search = False
  return nbNodes, R, channel, receiver, repetition    
    
def parseData(fres):
  rxBits = -1
  errBits = -1
  pErrBits = re.compile(r"Erroneous\sbits[^\d]+(\d+)")
  pRecvBits = re.compile(r"Total\sreceived\sbits[^\d]+(\d+)")
  while errBits == -1 or rxBits == -1:
    line = fres.readline()
    if line == "":
      raise Exception("Reached end of file %s without finding the requested information." % fres.name)
    else:
      if rxBits == -1:
        mRecvBits = re.search(pRecvBits, line)
	if mRecvBits is not None:
          rxBits = float(mRecvBits.group(1))
	  line = fres.readline()
      if errBits == -1:
	mErrBits = re.search(pErrBits, line)
 	if mErrBits is not None:
 	  errBits = float(mErrBits.group(1))
  return rxBits, errBits
  
def plotMAICircleN(experiment, detRecvBits, detErrBits, receivers, channels, nbTransmitters, ranges, repetitions):
  assert(len(ranges) == 1)
  R = ranges[0]
  bers = zeros( (len(receivers), len(channels), nbTransmitters[-1]) )
  if not os.path.exists("figures/%s" % experiment):
    os.makedirs("figures/%s" % experiment)

  for receiver in range(len(receivers)):
    for channel in range(len(channels)):
      boxPlotData = zeros( ( repetitions[-1] + 1, nbTransmitters[-1] ) )
      for nbTx in nbTransmitters:
        rxBits = 0
        errBits = 0
        for rep in repetitions:
          rxBitsRep = detRecvBits[receivers[receiver]][channels[channel]][nbTx+1][R][rep]
          errBitsRep = detErrBits[receivers[receiver]][channels[channel]][nbTx+1][R][rep]
          boxPlotData[rep, nbTx-1] = errBitsRep / rxBitsRep
  	  rxBits = rxBits + rxBitsRep
	  errBits = errBits + errBitsRep
      bers[receiver][channel][nbTx-1] = errBits / rxBits

    figure()
    axis()
    grid(True)
    hold(True)
    titre = "%s %s" % (receiver, channel)
    boxplot(boxPlotData, positions=nbTransmitters, whis=5)
    ax = gca()
    ax.set_yscale('log')
    draw()
    xlabel('Active nodes')
    ylabel('Bit Error Rate')
    title(titre)
    savefig('figures/%s/ber-%s_%s.png' % (experiment, receiver, channel), dpi=300)

  # Overview
  figure()
  axis()
  grid(True)
  hold(True)
  # One line per channel and decider combination
  curr_legend = 0
  for receiver in range(len(receivers)):
    for channel in range(len(channels)):
      legende = "%s %s" % (receivers[receiver], channels[channel])
      semilogy(nbTransmitters, bers[receiver][channel], label=legende)
  xlabel('Active nodes')
  ylabel('Bit Error Rate')
  legend(loc='upper left')
  savefig('figures/%s/ber-overview.png' % experiment, dpi=300)



def plotMAICircleR(experiment, detRecvBits, detErrBits, receivers, channels, nbTransmitters, ranges, repetitions):
  assert(len(nbTransmitters) == 1)
  nbTx = nbTransmitters[-1]
  bers = zeros( (len(receivers), len(channels), len(ranges)) )
  if not os.path.exists("figures/%s" % experiment):
    os.makedirs("figures/%s" % experiment)
  
  for receiver in range(len(receivers)):
    for channel in range(len(channels)):
      boxPlotData = zeros( (len(repetitions), len(ranges)) )
      for R in range(len(ranges)):
        rxBits = 0
        errBits = 0
        for rep in repetitions:
          rxBitsRep = detRecvBits[receivers[receiver]][channels[channel]][nbTx+1][ranges[R]][rep]
          errBitsRep = detErrBits[receivers[receiver]][channels[channel]][nbTx+1][ranges[R]][rep]
          boxPlotData[rep, R] = errBitsRep / rxBitsRep
  	  rxBits = rxBits + rxBitsRep
	  errBits = errBits + errBitsRep
        bers[receiver][channel][R] = errBits / rxBits

    figure()
    axis()
    grid(True)
    hold(True)
    titre = "%s %s" % (receiver, channel)
    print boxPlotData
    boxplot(boxPlotData, whis=5)
    ax = gca()
    ax.set_yscale('log')
    draw()
    xlabel('Distance of interfers')
    ylabel('Bit Error Rate')
    title(titre)
    savefig('figures/%s/ber-%s_%s.png' % (experiment, receiver, channel), dpi=300)

  # Overview
  figure()
  axis()
  grid(True)
  hold(True)
  # One line per channel and decider combination
  curr_legend = 0
  for receiver in range(len(receivers)):
    for channel in range(len(channels)):
      print bers[receiver][channel]
      legende = "%s %s" % (receivers[receiver], channels[channel])
      semilogy(ranges, bers[receiver][channel], label=legende)
  xlabel('Distance of interferers')
  ylabel('Bit Error Rate')
  legend(loc='upper left')
  savefig('figures/%s/ber-overview.png' % experiment, dpi=300)


def plotMAIUnif(experiment, detRecvBits, detErrBits, receivers, channels, nbTransmitters, ranges, repetitions):
  assert(len(ranges) == 1)
  R = ranges[0]
  bers = zeros( (len(receivers), len(channels), nbTransmitters[-2]) )
  if not os.path.exists("figures/%s" % experiment):
    os.makedirs("figures/%s" % experiment)

  for receiver in range(len(receivers)):
    for channel in range(len(channels)):
      boxPlotData = zeros( ( repetitions[-1] + 1, nbTransmitters[-2] ) )
      for nbTx in nbTransmitters:
        rxBits = 0
        errBits = 0
        print "nbTransmitter:"
	print nbTx
        for rep in repetitions:
          rxBitsRep = detRecvBits[receivers[receiver]][channels[channel]][nbTx+1][R][rep]
          errBitsRep = detErrBits[receivers[receiver]][channels[channel]][nbTx+1][R][rep]
          boxPlotData[rep, nbTx-2] = errBitsRep / rxBitsRep
  	  rxBits = rxBits + rxBitsRep
	  errBits = errBits + errBitsRep
	print errBits
	print rxBits
        bers[receiver][channel][nbTx-2] = errBits / rxBits

    if len(repetitions) > 1:
      figure()
      axis()
      grid(True)
      hold(True)
      titre = "%s %s" % (receiver, channel)
      print boxPlotData
      boxplot(boxPlotData, positions=nbTransmitters, whis=5)	
      #plot(nbTransmitters, bers[receiver][channel])
      ax = gca()
      ax.set_yscale('log')
      draw()
      xlabel('Simultaneous transmissions')
      ylabel('Bit Error Rate')
      #title(titre)
      savefig('figures/%s/ber-%s_%s.png' % (experiment, receiver, channel), dpi=300)

  # Overview
  figure()
  axis()
  grid(True)
  hold(True)
  # One line per channel and decider combination
  curr_legend = 0
  for receiver in range(len(receivers)):
    for channel in range(len(channels)):
      legende = "%s %s" % (receivers[receiver], channels[channel])
      semilogy(nbTransmitters, bers[receiver][channel], label=legende)
  xlabel('Active nodes')
  ylabel('Bit Error Rate')
  legend(loc='upper left')
  savefig('figures/%s/ber-overview.png' % experiment, dpi=300)


def analyze(experiment):
  resultsPath="results/%s-*.sca" % experiment
  print "\n\nLooking for files like %s...\n" % resultsPath

  nbNodesSet = set()
  ranges = set()
  receivers = set()
  channels = set()
  repetitions = set()
  bers = dict()
  detErrBits = dict()
  detRecvBits = dict()

  # Parse all files
  for resultsFile in glob.glob(resultsPath):
    fres = open(resultsFile, "r")
    # run metadata information
    nbNodes, R, channel, receiver, repetition = parseHeader(fres, experiment)
    nbNodesSet.add(nbNodes)
    channels.add(channel)
    ranges.add(R)
    receivers.add(receiver)
    repetitions.add(repetition)
    # update data structures
    if not detErrBits.has_key(receiver):
      detErrBits[receiver] = dict()
      detRecvBits[receiver] = dict()
    if not detErrBits[receiver].has_key(channel):
      detErrBits[receiver][channel] = dict()
      detRecvBits[receiver][channel] = dict()
    if not detErrBits[receiver][channel].has_key(nbNodes):
      detErrBits[receiver][channel][nbNodes] = dict()
      detRecvBits[receiver][channel][nbNodes] = dict()
    if not detErrBits[receiver][channel][nbNodes].has_key(R):
      detErrBits[receiver][channel][nbNodes][R] = dict()
      detRecvBits[receiver][channel][nbNodes][R] = dict()
    if not detErrBits[receiver][channel][nbNodes][R].has_key(repetition):
      detErrBits[receiver][channel][nbNodes][R][repetition] = dict()
      detRecvBits[receiver][channel][nbNodes][R][repetition] = dict()
    # run data information
    rxBits, errBits = parseData(fres)
    detRecvBits[receiver][channel][nbNodes][R][repetition] = rxBits
    detErrBits[receiver][channel][nbNodes][R][repetition] = errBits

  receivers = list(receivers)
  channels = list(channels)
  nbTransmitters1 = list(nbNodesSet)
  nbTransmitters1.sort()
  nbTransmitters = array(nbTransmitters1) - 1
  repetitions = list(repetitions)
  repetitions.sort()
  ranges = list(ranges)
  ranges.sort() 

  # Plot results

  if experiment == "MAICircleN":
    plotMAICircleN(experiment, detRecvBits, detErrBits, receivers, channels, nbTransmitters, ranges, repetitions)      
  if experiment == "MAICircleR":
    plotMAICircleR(experiment, detRecvBits, detErrBits, receivers, channels, nbTransmitters, ranges, repetitions)
  if experiment == "MAIUnif":
    plotMAIUnif(experiment, detRecvBits, detErrBits, receivers, channels, nbTransmitters, ranges, repetitions)
  # The end -- show the graphs
  show()


if __name__ == "__main__":
    sys.exit(main())



