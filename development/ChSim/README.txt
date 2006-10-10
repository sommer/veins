*************************************************
* Program: ChSim  				*
* (c)2006 Universitaet Paderborn                *
*						*
* Authors: 					*
*  Thorsten Pawlak  tpawlak(at)upb.de    	* 
*  Stefan Valentin  stefanv(at)upb.de		*
*                                               *
* Original by: Randy Vu                         *
*		Technische Universitaet Berlin  *
*                                               *
*************************************************

Description:
============
The program generates channel trace files containing channel state values for 
simulating mobile Networks. The calculation can contain path loss, shadow loss
and fading loss. The ChSim contains a trigger generator and a filewriter, but the
channel can also be embedded in other simulations in order to generate the channel
state values on the fly. A random waypoint mobility model is integrated to simulate
different speeds and distances.

Structure:
**********************************************************
* triggerGenerator-1---1-channelManager-1---1-fileWriter *
**********************************************************
                             1
                             |
                             J
                     ********************
                     * mobileStation(s) *
                     *       1          *
                     *       |          *
                     *       C          *
                     *    channel       *
                     ********************

Calculation 'request' are generated with the triggerGenerator. The channelManager
sends C empty csMessages to each of the J mobileStations (C*J messages in
total). The mobileStations send the messages to C messages to channel 1..C.
Each channel calculates the loss and fills out the appropriate fields in the
csMessage. The messages are send back to the channelManager wich forwards them to
the fileWriter. The fileWriter creates J*C files, one for each channel of each
mobileStation containing the channel state values calculated in the channel modules. In
each file the values for each sub band are listed in column 1..S and row 1..T
contains the time steps.

File format interpretation:

  column 1 to S
     row 1 to T
     
	*-------- S
	| 2 3 9 2
	| 1 4 6 1
	| 2 5 7 3
	T 

one file for each mobile Station and Channel -> J*C files

T: time
S: # subbands
J: # mobile stations
C: # channels for each mobile station

Prerequisits:
=============
ChSim is written for OMNeTpp3.1 but should work with all 3.x
installations. An working installation of OMNeTpp (http://www.omnetpp.org/) is
assumed. ChSim also works with Akaroa in order to use its better
RNG's or for distributed simulations.
Make sure that OMNeTpp works correctly meaning, the PATH, LD_LIBRARY_PATH and
INCLUDE_PATH are set correctly. 

Installation:
=============
- unpack the archive
- change to base directory
- call opp_makemake -f -u Tkenv
- call make

The created executable can now be launched. The trace files can be found in
./traces/ subdirectory. If you would like to use the command line interface 
change the appropriate lines in Makefile and rebuild or call
	opp_makemake -f -u Cmdenv; make
again.

Have fun.

Known problems:
===============

Changelog:
==========
v2.1.0 - 13.05.06
-----------------
Some bugfixes concerning the channel model. Mobility models added. Many
small improvements.

v2.0.1 - 24.03.06
-----------------
Consistence in naming. cnr is calles channel state and bs is channelManager from now on.

v2.0.0 - 01.02.06
-----------------
Complete redesign. The architecture is now split in several modules an compound
modules. Calculations are now done in an independent channel module.
Preparations for mobility models are made. Code was speed optimized and adapted
to Omnetpp 3.x. RNG's are now all under Omnet controll. Renamed to ChSim. Some
variables may still have the old names (e.g. cnr for channel state).

v1.3.1 - 02.03.04
-----------------
setw() causes problems with some compiler/filesystem combinations. therefore it was removed

v1.3.0 - 09.07.03
-----------------
There is a new parameter known as shad_freq.  This refers to the frequency with which you want to calculate
the shadowing in the program.  Because it's in terms of symbols, if you set it to 250, which is the number
of symbols in one Downlink Phase, it'll calculate shadowing once every downlink.  If you set it to 125000, which
is the number of symbols in one second, it'll calculate shadowing once per second.

v1.2.0 - 18.03.03
-----------------
In order to select the different signal attenuations that you want to consider, there are new flags in the
omnetpp.ini file: FAD_FLAG, PATH_FLAG and SHAD_FLAG, which allow you to select fading, path loss and shadowing.
To set the flags, just equate the flags to 1 and this will include the corresponding calculation.

Also, within BaseStation.cc, the fading function has been changed.  Before, when you called the function,
it calculated the fading for all elements in the Subband SNR Matrix. Now, when you call the function, it only
calculates the fading for the selected element of the matrix.

v1.1.2 - 17.03.03
-----------------
There are currently two versions of the snrGenerator available.  One version is v1.1.2 and the
other belongs to Irene, v1.0.1i.  The difference between Irene's and v1.1.2 is that for her version, regardless
of the wt speed specified, the sampling rate was always 1.  Normally, if a speed of i.e 50m/s is selected,
then fading is done 50 times a DL and the ref 50 times a second.  For 1m/s, it was 1 time per DL and
once per second.  For Irene's, if you enter 50m/s, the sampling will be done as if it were 1m/s.  So, there
is a new parameter inside of the v1.1.1 to allow for Irene's operations as well as a general operation.

The parameter, wtmovement.basestation.samp_per_dl, inside of the omnetpp.ini file.  wtmovement.max_speed
is still there and it represents the speed for calculation purposes.  samp_per_dl indicates the frequency
of sampling.  If it is set to 1, that just means that the fading is performed 1 per DL and the ref(shadowing
and path loss) are done once per second as well.  If it is set to 50, then fading is down 50 times per DL
and ref is 50 times per second.

v1.1.0 - 24.02.03
-----------------
The process by which the terminals move in the cell has been changed.  Previously, a random direction
was calculated and the terminal moved, with a maximum speed, in that particular way.  But, that meant
that there were only 6 different paths the terminal could use to traverse the screen.  4 times along
the edges and then from one corner to the opposite corner; hence, 6 paths.  Now, the terminal
calculates a destination point and heads toward that point.  Once it has been reached, a new point
is calculated and the terminal continues like that.  The previous movement is still available but
is saved as WT.h.old and WT.cc.old.  The new one is obviously WT.h and WT.cc.
v1.1.1 - 27.02.03
-----------------
Sometimes, it is necessary to generate snr values such that the terminals are uniformly distributed 
throughout the cell.  To force the terminals to be stationary or uniformly distributed in the cell,
set the uni_flag in the omnetpp.ini file to 1 and ensure that max_speed=1 and samp_per_dl=1.

v1.0.1 - 19.02.03
-----------------
There were many revised versions of snrGenerator and they were never noted.  So, this is 
essentially the "first" version of the program.  From the older program, there have been
a few changes within the source and the omnetpp.ini files that are worth noting.

First off, there are two new parameters in the omnetpp.ini: dl_time and ul_time.  These
are merely the lengths of both the downlink(dl) and uplink(ul) in seconds.  For most of
our purposes, we have been using dl_time=1e-3 and ul_time=1e-3.

Within BaseStation.cc, a new timing scheme has been used which is less confusing and more
accurate than the older one.  This does affect the snr files because of the times that
the correlated fading utilizes.