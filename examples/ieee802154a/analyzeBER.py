#!/usr/bin/python

from pylab import *
from subprocess import *
from os import listdir, remove, rename
import fileinput,re, random

# configuration
nbPackets = 40
nbRunsPerConfig = 1
payloadSize = 100 #  bytes
#distances = array( [7, 10, 15, 20])
distances = array([5, 10, 20, 50, 100])
phyrng = 6
macrng = 1
apprng = 1

# generation
## RNG Configuration

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
fruns.write("phySim.node[0].mobility.x = 0\n")
fruns.write("phySim.node[0].mobility.y = 0\n")
fruns.write("phySim.node[0].mobility.z = 0\n")
fruns.write('phySim.node[1].mobility.y = 0\n')
fruns.write('phySim.node[1].mobility.z = 0\n')
fruns.write("\n\n")

for d in range(distances.size):
  for run in range(nbRunsPerConfig):
    fruns.write("[Run " + str(d*nbRunsPerConfig+run+1) + "]\n")
    dx = distances[d]
    fruns.write('phySim.node[1].mobility.x = %d\n\n' % dx)


fruns.close()

# clean up destination directory
datapath = "./out/ber/data/"
oldData = listdir(datapath)
for datafile in oldData:
    remove(datapath + datafile)


# execute runs



for currRun in range(1, distances.size*nbRunsPerConfig+1):
    check_call(["./ieee802154a", "-r " + str(currRun)])
    rename('omnetpp.sca', 'out/ber/data/omnetpp-%d.sca' % currRun)
	

errBits = zeros( (distances.size) )
recvBits = zeros( (distances.size) )
bers = zeros( (distances.size) )
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
        for line in fileinput.input('out/ber/data/omnetpp-%d.sca' % run):
		mRecvBits = re.search(pRecvBits, line)
		if mRecvBits is not None:
			recvBits[d] = recvBits[d] + float(mRecvBits.group(1))
		else:
			mErrBits = re.search(pErrBits, line)
			if mErrBits is not None:
				errBits[d] = errBits[d] + float(mErrBits.group(1))
				
bers = errBits / recvBits


# Plot results
figure()
axis()
grid(True)
hold(True)

# simulation results

print distances
print bers


semilogy(distances, bers)
#plot(distances, bers)

xlabel('Distance')
ylabel('Bit Error Rate')


title('Pathloss')
savefig('out/ber/figures/ber.png', dpi=300)
show()

