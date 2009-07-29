#!/usr/bin/python
#
# copyright 2009 CSEM SA, Jerome Rousselot <jerome.rousselot@csem.ch>
#


from pylab import *
from matplotlib.font_manager import *
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
      opts, args = getopt.getopt(sys.argv[1:], "t:hn:s:", ["topo=", "help", "nodes=", "size="])
    except getopt.GetoptError, msg:   
      raise Usage(msg)        
    topo = "circle"
    nodes = 10
    size = 10
    for opt, arg in opts:
      print opt
      print arg
      if opt in ("-t", "--topo"):
        topo = arg
      elif opt in ("-h", "--help"):
        raise(Usage(""))
      elif opt in ("-n", "--nodes"):
        nodes = int(arg)
      elif opt in ("-s", "--size"):
        size = float(arg)
    
    if(topo=="square"):
      generateSquare(topo,nodes,size)
    elif(topo=="circle"):
      generateCircle(topo, nodes, size)

  except Usage, err:
    print "This script generates a topology file for Omnet++.\n"
    print "It accepts the following options:"
    print " -t (--topo) to select a topology among: circle, square (not yet implemented)."
    print " -n (--nodes) to define the number of nodes to place on the topology (in addition to node 0 at the center)."
    print " -s (--size) to give the characteristic dimension of the topology (radius for circle, width for square).\n<n"
    sys.exit(2)


def generateCircle(topo, nodes, size):
  f = open("topo.ini", "w")
  f.write("\n\n")
  f.write("[General]\n")
  f.write("sim.node[*].mobility.z = 0\n")
  ox = 10 + size / 2.0 # center - x
  oy = 10 + size / 2.0 # center - y
  f.write("sim.node[0].mobility.x = %d\n" % ox)
  f.write("sim.node[0].mobility.y = %d\n" % oy)
  theta = 0
  step = 360/nodes
  for node in range(nodes):
    posx = ox + size*cos(theta)
    posy = oy + size*sin(theta)
    f.write("sim.node[%d].mobility.x = %d\n" % (node+1, posx) )
    f.write("sim.node[%d].mobility.y = %d\n" % (node+1, posy) )
    theta = theta + step
  f.write("\n\n")
  f.close()

def generateSquare(topo, nodes, size):
  print "Not implemented yet.\n"


if __name__ == "__main__":
    sys.exit(main())


