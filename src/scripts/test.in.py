#!/usr/bin/env python2

# ^-- contents of out/config.py go here

"""
Runs Veins test suite in current directory
"""

import sys
import os

run_libs = set(run_libs) | {'src'}
env = {'LD_LIBRARY_PATH': ':'.join(os.path.abspath(l) for l in run_libs)}
cmdline = ['src/catch_testing_dynamic'] + sys.argv[1:]

#print(['env'] + cmdline, 'ENV:', env)
os.execvpe('env', ['env'] + cmdline, env)
