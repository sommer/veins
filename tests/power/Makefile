#
# OMNeT++/OMNEST Makefile for MiXiMorig
#
# This file was generated with the command:
#  opp_makemake -f --nolink -O out -d deviceTestAll -d utils -d deviceTestMulti -d deviceTestAccts -d deviceTest -I/home/karl/job/omnetpp/MiXiM/include -L/home/karl/job/omnetpp/MiXiM/lib -Lgcc-release/src/base -L../../out/$(CONFIGNAME)/tests/power/utils -L../../out/$(CONFIGNAME)/base -L../../out/$(CONFIGNAME)/modules -L../../out/$(CONFIGNAME)/tests/testUtils -DMIXIMBASE=/home/willkomm/mixim/mixim/trunk/
#

# C++ include paths (with -I)
INCLUDE_PATH = -I/home/karl/job/omnetpp/MiXiM/include -I.

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = tests/power
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS =

# Message files
MSGFILES =

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

COPTS = $(CFLAGS) -DMIXIMBASE=/home/willkomm/mixim/mixim/trunk/ $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target

all: $(OBJS) submakedirs Makefile
	@# Do nothing

submakedirs:  deviceTestAll_dir utils_dir deviceTestMulti_dir deviceTestAccts_dir deviceTest_dir

.PHONY: deviceTestAll utils deviceTestMulti deviceTestAccts deviceTest
deviceTestAll: deviceTestAll_dir
utils: utils_dir
deviceTestMulti: deviceTestMulti_dir
deviceTestAccts: deviceTestAccts_dir
deviceTest: deviceTest_dir

deviceTestAll_dir:
	cd deviceTestAll && $(MAKE)

utils_dir:
	cd utils && $(MAKE)

deviceTestMulti_dir:
	cd deviceTestMulti && $(MAKE)

deviceTestAccts_dir:
	cd deviceTestAccts && $(MAKE)

deviceTest_dir:
	cd deviceTest && $(MAKE)

.SUFFIXES: .cc

$O/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CXX) -c $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)
	cd deviceTestAll && $(MAKE) msgheaders
	cd utils && $(MAKE) msgheaders
	cd deviceTestMulti && $(MAKE) msgheaders
	cd deviceTestAccts && $(MAKE) msgheaders
	cd deviceTest && $(MAKE) msgheaders

clean:
	-rm -rf $O
	-rm -f MiXiMorig MiXiMorig.exe libMiXiMorig.so libMiXiMorig.a libMiXiMorig.dll libMiXiMorig.dylib
	-rm -f ./*_m.cc ./*_m.h

	-cd deviceTestAll && $(MAKE) clean
	-cd utils && $(MAKE) clean
	-cd deviceTestMulti && $(MAKE) clean
	-cd deviceTestAccts && $(MAKE) clean
	-cd deviceTest && $(MAKE) clean

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc
	-cd deviceTestAll && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd utils && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd deviceTestMulti && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd deviceTestAccts && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd deviceTest && if [ -f Makefile ]; then $(MAKE) depend; fi

# DO NOT DELETE THIS LINE -- make depend depends on it.

