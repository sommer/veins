#!/usr/bin/python
#
# copyright 2008 CSEM SA, Jerome Rousselot <jerome.rousselot@csem.ch>
#
# Shows maximum range for various receiver configurations
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

styles = ['-g^', '-rs', '-bo']

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
  pDistance = re.compile(r"\$distance=\s*(\d+)")
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
#	mR = re.search(pR, line)
	mChannel = re.search(pChannel, line)
        mReceiver = re.search(pReceiver, line)
	mRepetition = re.search(pRepetition, line)
 	mDistance = re.search(pDistance, line)
	if mRepetition is None or mChannel is None or mReceiver is None or mDistance is None:	
	  print mRepetition, mChannel, mReceiver, mDistance
  	  raise Exception("Could not parse iterationvars2 line of file %s." % fres.name) 
	channel = mChannel.group(1)
	receiver = mReceiver.group(1)
	distance = int(mDistance.group(1))
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
    if not detErrBits[receiver].has_key(distance):
      detErrBits[receiver][distance] = dict()
      detRecvBits[receiver][distance] = dict()
    if not detErrBits[receiver][distance].has_key(repetition):
      detErrBits[receiver][distance][repetition] = dict()
      detRecvBits[receiver][distance][repetition] = dict()
    # run data information
    rxBits, errBits = parseData(fres)
    detRecvBits[receiver][distance][repetition] = rxBits
    detErrBits[receiver][distance][repetition] = errBits

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
      errs = array( [ detErrBits[receiver][d] for d in sorted(detErrBits[receiver].keys()) ] )
      recvs = array( [ detRecvBits[receiver][d] for d in sorted(detRecvBits[receiver].keys()) ] )
      bersCfg = list()
      print "errs:"
      print errs
      for d in range(len(distances)):
	rxBits = 0
	errBits = 0
        for rep in repetitions:
	  rxBits = rxBits + recvs[d][rep]
	  errBits = errBits + errs[d][rep]
	if rxBits > 0:
          bersCfg.append(errBits/rxBits)
	else:
	  print "Warning! Divide by zero."
	  bersCfg.append(0)
      print receiver
      bers[receiver, channel] = array(bersCfg)
      print bers[receiver, channel]
#      print "BER for receiver %s, channel %s: " % (receiver, channel)
#      print errs[recvs > 0]
#      print recvs[recvs > 0]

#   for receiver in receivers:
#     errBits = 
#     errs = array ( [detErrBits[receiver][distance][d] for d in sorted(detErrBits[receiver][channel].keys()) ] )
#     recvs = 
#      print bers[receiver, channel]
#     bers[receiver] = errs[recvs > 0] / recvs[recvs > 0]

  # Plot results
  figure()
  axis()
  grid(True)
  hold(True)
  
  # simulation results
  
#  print distances
#  print bers

  currLine = 0
  print channels
  receivers = list(['1dB', '3dB', '10dB'])
  print receivers
  for receiver in receivers:
    print bers[receiver, channel]
    legende = "%s" % (receiver, )
    loglog(distances[:len(bers[receiver, channel]):], bers[receiver, channel], styles[currLine], label=legende, markersize=10)
    currLine = currLine + 1
  xlabel('Distance')
  ylabel('Bit Error Rate')
  legend(loc='upper left', prop = FontProperties( size="smaller" ))
  savefig('figures/Sensitivity/ber-sens-legend.png', dpi=300)

  show()


if __name__ == "__main__":
    sys.exit(main())



