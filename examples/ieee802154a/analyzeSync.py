#!/usr/bin/python
#
# copyright 2008 CSEM SA, Jerome Rousselot <jerome.rousselot@csem.ch>
#
# Shows maximum range for various receiver configurations
#

from pylab import *
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


class Usage(Exception):
  def __init__(self, msg):
    self.msg = msg

def main(argv=None):
  if argv is None:
    argv = sys.argv
  # Command-line options
  try:                                
    try:
      opts, args = getopt.getopt(sys.argv[1:], "grac:", ["generate", "run", "analyze", "config="])
    except getopt.GetoptError, msg:   
      raise Usage(msg)        
    config = False
    doGenerate = False
    doRun = False
    doAnalyze = False
    for opt, arg in opts:
      if opt in ("-c", "--config"):
	config = True
        experiment = arg
      if opt in ("-g", "--generate"):
        doGenerate = True
      if opt in ("-r", "--run"):
        doRun = True
      if opt in ("-a", "--analyze"):
        doAnalyze = True

    if not config:
      raise Usage("Missing mandatory argument.")

    if not(doGenerate | doRun | doAnalyze):
      doGenerate = False
      doRun = False
      doAnalyze = True
    
    if doAnalyze:
      analyze(experiment)
    

  except Usage, err:
    print "This script generates configuration files for omnet++, run the simulation, and analyze the results."
    print "It accepts the following options:"
    print " -c (--config) to select which configuration to analyze (as defined in omnetpp.ini). This is a mandatory parameter."
    print " -a (--analyze) to analyze the simulation results."
    sys.exit(2)
  

def parseHeader(fres):
  pIterationvars2 = re.compile(r"iterationvars2")
  pChannel = re.compile(r"\$Channel=([\w-]+)")
  pReceiver = re.compile(r"\$Receiver=([\w-]+)")
  pDistance = re.compile(r"$distance=(\d+)")
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
	mR = re.search(pR, line)
	mChannel = re.search(pChannel, line)
        mReceiver = re.search(pReceiver, line)
	mRepetition = re.search(pRepetition, line)
 	mDistance = re.search(pDistance, line)
	if mRepetition is None or mChannel is None or mReceiver is None or mDistance is None:	
	  print mRepetition, mChannel, mReceiver, mDistance
  	  raise Exception("Could not parse iterationvars2 line of file %s." % fres.name) 
	channel = mChannel.group(1)
	receiver = mReceiver.group(1)
	distance = mDistance.group(1)
	repetition = float(mRepetition.group(1))
        search = False
  return channel, receiver, distance, repetition    

def parseData(fres):
  rxBits = -1
  errBits = -1
  distancesSet = set()
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


# Analyze

def analyze(experiment):

  resultsPath="results/%s-*.sca" % experiment
  print "\n\nLooking for files like %s...\n" % resultsPath

  receivers = set()
  channels = set()
  repetitions = set()
  bers = dict()
  detErrBits = dict()
  detRecvBits = dict()
  distancesSet = set()

  # Parse all files
  for resultsFile in glob.glob(resultsPath):
    fres = open(resultsFile, "r")
    # run metadata information
    channel, receiver, distance, repetition = parseHeader(fres)
    channels.add(channel)
    receivers.add(receiver)
    distancesSet.add(distance)
    repetitions.add(repetition)
    # update data structures
    if not detErrBits.has_key(receiver):
      detErrBits[receiver] = dict()
      detRecvBits[receiver] = dict()
    if not detErrBits[receiver].has_key(channel):
      detErrBits[receiver][channel] = dict()
      detRecvBits[receiver][channel] = dict()
    if not detErrBits[receiver][channel].has_key(repetition):
      detErrBits[receiver][channel][repetition] = dict()
      detRecvBits[receiver][channel][repetition] = dict()
    # run data information
    rxBits, errBits = parseData(fres)
    detRecvBits[receiver][channel][repetition] = rxBits
    detErrBits[receiver][channel][repetition] = errBits

  receivers = list(receivers)
  channels = list(channels)
  repetitions = list(repetitions)
  repetitions.sort()

  distances = list(distancesSet)
  distances.sort()
  print distances

  # find maximum range (recv > 0)
  # and prepare data for bar plots
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

  curr_legend = 0
  for receiver in receivers:
    for channel in channels:
     print bers[receiver, channel]
     legende = "%s %s" % (receiver, channel)
     loglog(distances[:len(bers[receiver, channel]):], bers[receiver, channel], label=legende)
  xlabel('Distance')
  ylabel('Bit Error Rate')
  legend(loc='upper left')
  savefig('figures/BERDistance/ber-cms-legend.png', dpi=300)

  show()


if __name__ == "__main__":
    sys.exit(main())



