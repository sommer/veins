#!/usr/bin/python

import fileinput, re
from pylab import *
from math import *

pBER = re.compile(r".*BER\D+([\d.]+)")
pRun = re.compile(r"run (\d+) .*")

i = 1
bers = []
runs = []

for line in fileinput.input("omnetpp.sca"):
  # lookup BER
  mBER = re.search(pBER, line)
  if mBER is not None:
      currBER = float(mBER.group(1))
      if currBER > 0:
           logBER = log10(currBER)
      else:
           logBER = -3
      bers.append(logBER)
      i = i + 1
  # lookup run number
  mRun = re.search(pRun, line)
  if mRun is not None:
    runs.append(int(mRun.group(1)))

print runs
    
#print bers


berDetails = [[] for i in range(10) ]
pDetails = re.compile(r"[\d.]+\s([\d.]+)")

# retrieve detailed data for each run
for run in runs:
  # build histogram for each run
  for line in fileinput.input("vectors/vector-run_" + str(run) + "-0.vec"):
    # retrieve all values
    mDetail = re.search(pDetails, line)
    if mDetail is not None:
      currBER = float(mDetail.group(1))
      if currBER > 0:
           logBER = log10(currBER)
      else:
           logBER = -3
      berDetails[run-1].append(logBER)


# Generate graph
#print distances
#print bers

#plot(distances, bers, marker='s', markerfacecolor='r', markeredgecolor='b', linestyle='--')
boxplot(berDetails, positions=runs)
xlabel('Number of concurrent transmissions')
ylabel('Bit Error Rate')
hold(True)
plot(runs, bers, marker='s', markerfacecolor='r', markeredgecolor='b', linestyle='--')
savefig('berPlot-MUI-Histogram.png')
show()



# Generate graph
#print runs
#print bers
plot(runs, bers, marker='s', markerfacecolor='r', markeredgecolor='b', linestyle='--')
xlabel('Number of concurrent emissions')
ylabel('Bit Error Rate')
savefig('berPlot-MUI.png')
show()

