#!/usr/bin/python

import sys
from re import compile

node = compile("node\[(\d+)\]")

files = {}

for line in sys.stdin.xreadlines():
	if len(line.strip())==0:
		continue
	
	id = node.search(line)
	if id == None:
		continue
	id = "%02d"%int(id.groups()[0])
		
	if not files.has_key(id):
		files[id] = open(id+".log","w")
	files[id].write(line)

for id in files.keys():
	files[id].close()
