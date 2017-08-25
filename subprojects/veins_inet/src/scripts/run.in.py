#!/usr/bin/env python2

# ^-- contents of out/config.py go here

"""
Runs Veins simulation in current directory
"""

import sys
import os

def relpath(s):
    veins_root = os.path.dirname(os.path.realpath(__file__))
    return os.path.relpath(os.path.join(veins_root, s), '.')

run_libs = [relpath(s) for s in run_libs]
run_neds = [relpath(s) for s in run_neds] + ['.']

lib_flags = ['-l%s' % s for s in run_libs]
ned_flags = ['-n' + ';'.join(run_neds)]

prefix = []
cmdline = prefix + ['opp_run'] + lib_flags + ned_flags + sys.argv[1:]

os.execvp('env', ['env'] + cmdline)

