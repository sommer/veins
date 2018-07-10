#!/usr/bin/env python2

# ^-- contents of out/config.py go here

"""
Runs Veins simulation in current directory
"""

import os
import argparse

def relpath(s):
    veins_root = os.path.dirname(os.path.realpath(__file__))
    return os.path.relpath(os.path.join(veins_root, s), '.')

parser = argparse.ArgumentParser('Run a Veins simulation')
parser.add_argument('-d', '--debug', action='store_true', help='Run using opp_run_dbg (instead of opp_run)')
parser.add_argument('-t', '--tool', metavar='TOOL', dest='tool', choices=['lldb', 'gdb', 'memcheck'], help='Wrap opp_run execution in TOOL (lldb, gdb or memcheck)')
parser.add_argument('-v', '--verbose', action='store_true', help='Print command line before executing')
parser.add_argument('--', dest='arguments', help='Arguments to pass to opp_run')
args, omnet_args = parser.parse_known_args()
if (len(omnet_args) > 0) and omnet_args[0] == '--':
    omnet_args = omnet_args[1:]

run_libs = [relpath(s) for s in run_libs]
run_neds = [relpath(s) for s in run_neds] + ['.']

opp_run = 'opp_run'
if args.debug:
    opp_run = 'opp_run_dbg'

lib_flags = ['-l%s' % s for s in run_libs]
ned_flags = ['-n' + ';'.join(run_neds)]

prefix = []
if args.tool == 'lldb':
    prefix = ['lldb', '--']
if args.tool == 'gdb':
    prefix = ['gdb', '--args']
if args.tool == 'memcheck':
    prefix = ['valgrind', '--tool=memcheck', '--leak-check=full', '--dsymutil=yes', '--log-file=valgrind.out']

cmdline = prefix + [opp_run] + lib_flags + ned_flags + omnet_args

if args.verbose:
    print "Running with command line arguments: %s" % ' '.join(['"%s"' % arg for arg in cmdline])

os.execvp('env', ['env'] + cmdline)

