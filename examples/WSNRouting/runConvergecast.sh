#!/bin/sh
rm -f results/convergecast-*
./WSNRouting -l ../../out/gcc-debug/base/miximbase -l ../../out/gcc-debug/modules/miximmodules -u Cmdenv -c convergecast

