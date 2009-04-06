/* -*- mode:c++ -*- ********************************************************
 * file:        IEEE802154A.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this class holds constants specified in IEEE 802.15.4A UWB-IR Phy
 ***************************************************************************/

#ifndef _IEEE802154A_H
#define	_IEEE802154A_H

#include "Signal_.h"
#include <vector>
#include <utility>

using namespace std;

class IEEE802154A {
public:
	/**@brief bit rate (850 kbps) */
    static const int mandatory_bitrate;
    /**@brief mandatory data symbol length (1025 ns) */
    static const_simtime_t mandatory_symbol;
    /**@brief 0.5 * mandatory_symbol (0.5 ms) */
    static const_simtime_t mandatory_timeShift;
    /**@brief mandatory pulse duration ( = 1 / bandwidth = 2 ns) */
    static const_simtime_t mandatory_pulse;
    /**@brief burst duration */
    static const_simtime_t mandatory_burst;
    /**@brief number of consecutive pulses forming a burst */
    static const int mandatory_pulses_per_burst;
    /**@brief Center frequency of band 3 in UWB lower band (500 MHz wide channel) */
    static const double mandatory_centerFreq; // in MHz !
    /**@brief default sync preamble length */
    static const_simtime_t mandatory_preambleLength;
    /**@brief Total triangular pulse peak energy in mW (0 dBm / 50 MHz over 500 MHz) */
    static const double maxPulse;

    static const short C31[8][31];

    static const short Ci = 5;

    static const short shortSFD[8];

    static const_simtime_t MaxFrameDuration;


    static const int maxS = 5000;
    static short s_array[maxS];
    static int last_s;

    /**@brief Number of Repetitions of the sync symbol in the SYNC preamble */
	static const int NSync = 64;
	/**@brief Length of the preamble code */
	static const short CLength = 31;
	/**@brief sync preamble spreading factor L */
	static const short spreadingdL = 16;
	/**@brief duration of a synchronization preamble symbol */
	static const_simtime_t Tpsym;

	/**@brief bit length of a Reed-Solomon symbol */
	static const short RSSymbolLength = 6;
	/**@brief Maximum number of erroneous symbols that the Reed-Solomon code can correct in ieee802.15.4a */
	static const short RSMaxSymbolErrors = 4;

	/**@brief Maximum size of message that is accepted by the Phy layer (in bytes). */
	static const int MaxPSDULength = 128;

	/**@brief Position of the first pulse max in the frame. */
	static const_simtime_t tFirstSyncPulseMax;

    enum UWBPRF {
    	PRF_OFF,
    	NOMINAL_4_M,
    	NOMINAL_16_M,
    	NOMINAL_64_M
    };

    enum Ranging {
        	NON_RANGING,
        	ALL_RANGING,
        	PHY_HEADER_ONLY
    };

    enum UWBPreambleSymbolRepetitions {
    	PSR_0,
    	PSR_16,
    	PSR_64,
    	PSR_1024,
    	PSR_4096
    };

    enum DataRate {
            	DATA_RATE_0,
            	DATA_RATE_1,
            	DATA_RATE_2,
            	DATA_RATE_3,
            	DATA_RATE_4
    };

    /**@brief currently unused */
    struct config {
    	int channel;
    	UWBPRF prf;
    	Ranging ranging;
    	UWBPreambleSymbolRepetitions preSymRep;
    	int bitrate;
    	int nbPulsesPerBurst;


    	const_simtime_t symbol_duration;
    	const_simtime_t shift_duration;
    	const_simtime_t pulse_duration;
    	const_simtime_t burst_duration;
    	const_simtime_t preambleLength;
    	double centerFrequency;
    };




    typedef std::pair<Signal *, vector<bool> * > signalAndData;

    static void selectConfig();
    static void setPSDULength(int _psduLength);

    /*@brief Generates psduLength bytes of data and encodes them into a Signal object.  */
    static signalAndData generateIEEE802154AUWBSignal(simtime_t signalStart);


// Constants from standard
    /* @brief Always 16 symbols for the PHY header */
    static const int Nhdr = 16;


protected:
	static void generateSyncPreamble(Mapping* mapping, Argument* arg);
	static void generateSFD(Mapping* mapping, Argument* arg);
	static void generatePhyHeader(Mapping* mapping, Argument* arg);
    static void generateBurst(Mapping* mapping, Argument* arg, simtime_t burstStart, short polarity);
    static void generatePulse(Mapping* mapping, Argument* arg, short polarity,
    		double peak=IEEE802154A::maxPulse, simtime_t chip=IEEE802154A::mandatory_pulse);
    static void setBitRate(Signal* s);
    static int s(int n);


public:
	static int psduLength;
	static config cfg;

	// Compute derived parameters
    static simtime_t getPhyMaxFrameDuration();
    static simtime_t getThdr();
    static int getHoppingPos(int sym);

};


#endif	/* _IEEE802154A_H */

