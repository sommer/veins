#!/usr/bin/python

import fileinput, re
from pylab import *

pBER = re.compile(r".*BER\D+([\d.]+)")
pRun = re.compile(r"run (\d+) .*")

i = 1
bers = []
runs = []

# (1) Count runs and identify associated BERs

for line in fileinput.input("omnetpp.sca"):
  # lookup BER
  mBER = re.search(pBER, line)
  if mBER is not None:
    bers.append(float(mBER.group(1)))
    i = i + 1
  # lookup run number
  mRun = re.search(pRun, line)
  if mRun is not None:
    runs.append(int(mRun.group(1)))

    
#print bers



pNode0 = re.compile(r"phySim\.node\[0\]\.mobility\.x = (\d+)")
pNode1 = re.compile(r"phySim\.node\[1\]\.mobility\.x = (\d+)")

positions1 = []
position0 = -1
distances = []

# (2) Retrieve nodes positions at each run
for line in fileinput.input("omnetpp.ini"):
  #lookup receiver position
  if position0 == -1:
    m0 = re.search(pNode0, line)
    if m0 is not None:
      position0 = float(m0.group(1))

  #lookup emitter position
  m1 = re.search(pNode1, line)
  if m1 is not None:
    positions1.append(float(m1.group(1)))

for pos in positions1:
  distances.append(pos-position0)

berDetails = [[] for i in range(20) ]
pDetails = re.compile(r"[\d.]+\s([\d.]+)")

# retrieve detailed data for each run
for run in runs:
  # build histogram for each run
  for line in fileinput.input("vectors/vector-run_" + str(run) + "-0.vec"):
    # retrieve all values
    mDetail = re.search(pDetails, line)
    if mDetail is not None:
      berDetails[run-1].append(float(mDetail.group(1)))


# Generate graph
#print distances
#print bers

#plot(distances, bers, marker='s', markerfacecolor='r', markeredgecolor='b', linestyle='--')
boxplot(berDetails, sym='', positions=distances, widths=10)
xlabel('Distance')
ylabel('Bit Error Rate')
hold(True)
plot(distances, bers, marker='s', markerfacecolor='r', markeredgecolor='b', linestyle='-')
savefig('berPlot-TwoNodes-Histogram.png')
show()

