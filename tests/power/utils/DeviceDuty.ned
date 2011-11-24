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
// Test module of a two phase duty cycle device.  Primarily for
// testing multiple accounts.
// 
// Alternates between ON and OFF state twice in each period.
// Switching from OFF to ON (draws wakeup ENERGY).  The energy
// consumed in each of the two ON periods and by the transitions are
// assigned to accounts DUTY0, DUTY1, and WAKE respectively.
//
// parameters:  draw 'current0'(mA) for dutyCycle0 * period(sec)
//              draw 0 mA for 'gap' * period(sec)
//              draw 'current1'(mA) for dutyCycle1 * period(sec)
//              draw 0 mA for rest of period 
//              draw 'wakeup' (mWs) for each off->on transition
package org.mixim.tests.power.utils;

simple DeviceDuty

{
    parameters:
        bool debug = default(false);
        volatile double period @unit(s);
        volatile double dutyCycle0;
        volatile double current0;
        volatile double gap;
        volatile double dutyCycle1;
        volatile double current1;
        volatile double wakeup;
}
