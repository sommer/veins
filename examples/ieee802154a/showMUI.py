#!/usr/bin/python

from pylab import *
from subprocess import *
from os import rename
import fileinput,re

# configuration
nbPacketsPerSource = 10
nbSources = 10
nbRunsPerConfig = 3
payloadSize = 16 # 16 bytes
phyrng = 6
macrng = 1
apprng = 1

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
grid()
hold(True)

# simulation results
semilogy(arange(1,nbInterferers+1), cfgBers[:nbInterferers])

xlabel('Concurrent transmissions')
ylabel('Log10(BER)')

# show success rates on second y axis
#ax2 = twinx()
#bar(TrafficRates, successRates, width=0.2)
#ylabel('success rate')
#ax2.yaxis.tick_right()


title('Robustness to Multiple Access Interference')

show()
