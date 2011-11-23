//# Energy Framework for Omnet++, version 0.9
//#
//# Author:  Laura Marie Feeney 
//# 
//# Copyright 2009 Swedish Institute of Computer Science.  
//# 
//# This software is provided `as is' and without any express or implied
//# warranties, including, but not limited to, the implied warranties of
//# merchantability and fitness for a particular purpose.
//#
//######################################################################
// Test module of a simple duty cycle device
// 
// Alternates between ON (draws current CURRENT) and OFF state.  Switching
// from OFF to ON (draws wakeup ENERGY).  
//    
// Primarily used for testing, as it's not connected to the operation
// of the host.  Can also be used to model e.g. a simple sensor. 

// parameters: 	consume 'current'(mA) for 'dutyCycle' * 'period'(sec)
//		consume 0 mA for (1 - 'dutyCycle') * 'period'(sec)
//		consume 'wakeup' mWs for each off->on transition
package org.mixim.tests.power.utils;

simple DeviceDutySimple

{
    parameters:
        bool debug = default(false);
        volatile double period @unit(s);
        volatile double dutyCycle;
        volatile double current;
        volatile double wakeup;
}
