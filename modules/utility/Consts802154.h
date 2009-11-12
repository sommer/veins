/* -*- mode:c++ -*- ********************************************************
 * file:        ConstsAccNoise3.h
 *
 * author:      Jerome Rousselot, Amre El-Hoiydi
 *
 * copyright:   (C) 2007-2008 CSEM SA, Neuchatel, Switzerland
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/

#ifndef CONSTANTS_ACCNOISE3
#define CONSTANTS_ACCNOISE3

//#define DEFAULT_BITRATE_802154 250000
#define DEFAULT_BITRATE_802154 25000
#define DEFAULT_BANDWIDTH_802154 2000000	/* 2 MHz channels at 2.4 GHz */

#define DEFAULT_SFD_LENGTH 16
#define DEFAULT_BER_LOWER_BOUND 0.000001

// Radio timers
#define DEFAULT_DELAY_SETUP_RX 3.85E-3
#define DEFAULT_DELAY_SETUP_TX 3.85E-3
#define DEFAULT_DELAY_SWITCH_RX_TX 0.64E-3
#define DEFAULT_DELAY_SWITCH_TX_RX 0.64E-3
// Radio power consumption
#define DEFAULT_RADIO_SLEEP_CONSUMED_POWER 			0.0000006
#define DEFAULT_RADIO_SETUP_RX_CONSUMED_POWER		0.0147
#define DEFAULT_RADIO_SETUP_TX_CONSUMED_POWER		0.0147
#define DEFAULT_RADIO_RX_CONSUMED_POWER				0.0597
#define DEFAULT_RADIO_TX_CONSUMED_POWER				0.0753
#define DEFAULT_RADIO_SWITCH_RX_TX_CONSUMED_POWER	0.0597
#define DEFAULT_RADIO_SWITCH_TX_RX_CONSUMED_POWER	0.0597

#define DEFAULT_THERMAL_NOISE -100

// Mac parameters
#define DEFAULT_HEADER_LENGTH 88	// 11 bytes
#define DEFAULT_ACK_DURATION  0.000352	// 11 bytes @ 250000 bps
#define DEFAULT_SIFS 1E-3	// minimum time between a data frame and a ack
//const double DEFAULT_DIFS=DEFAULT_SIFS+DEFAULT_ACK_DURATION;
#define DEFAULT_DIFS 0.0014

/** @brief Bit rates for 802.15.4 */
//const double BITRATES_802154[] = {
//    1000000,
//    2000000,
//    5500000,
//    11000000
//};


/** @brief duration of the PHY header
 *
 * If the radio was switched to
 * late, a partially received header is still ok, the total received
 * packet duration can be shorter by this amount compared to the send
 * packet.
 */
const double RED_PHY_HEADER_DURATION = 0.000020;

/** @brief Length of PLCP header and Preamble */
//const double PHY_HEADER_LENGTH=48; // 4 bytes for bit sync preamble, 1 byte SFD, 1 byte length
/** @brief PLCP header */
//const double HEADER_WITHOUT_PREAMBLE=48;
/** @brief Bitrate with which the header is send */
//const double BITRATE_HEADER=1E-6;
/** @brief 3dB filter bandwidth -- channels overlap! */
//const double BANDWIDTH=20E+6;

const int MAC_GENERATOR = 5;

/** @brief standard frame lengths
    @{ */
const double LENGTH_RTS = 160;	// RTS to be removed from 802154
const double LENGTH_CTS = 112;	// CRS to be removed from 802154
const double LENGTH_ACK = 88;	// 11 bytes

/** @} */
const int MAC802154_MAC_HEADER_LENGTH = 72;	// 2 bytes Control, 1 byte Seq, 4 bytes addressing, 2 bytes FCS
const int MAC802154_PHY_HEADER_LENGTH = 48;	// 4 bytes preamble, 1 byte SFD, 1 byte length, 2 bytes CRC

//time slot ST, short interframe space SIFS, distributed interframe
//space DIFS, and extended interframe space EIFS
/** @brief Standard time values
    @{ */
/** @brief Slot time  */
const double ST = 20E-6;

/** @brief Short interframe space
 *  between DATA and ACK
 */
const double SIFS = 10E-6;

/** @brief Distributed interframe space
 * medium must be clear for this time in DCF
 */
const double DIFS = 2 * ST + SIFS;

/** @brief extended interframe space
 *  whenever a corrupted frame is received, defer actions for this time
 */
const double EIFS =
  SIFS + DIFS + (MAC802154_PHY_HEADER_LENGTH + LENGTH_ACK) / 250000;

/** @brief We have to stop transmitting somewhen.
 *
 * This is the default value from the standard, it should be configurable
 */
const unsigned LONG_RETRY_LIMIT = 4;

/** @brief Imcrement longRetryCounter when this limit is met
 *
 * This is the default value from the standard
 */
const unsigned SHORT_RETRY_LIMIT = 7;

/**
 * Minimum size (initial size) of contention window, assuming DS PHY
 */
const unsigned CW_MIN = 31;

/** Maximum size of contention window, assuming DS PHY */
const unsigned CW_MAX = 1023;

#endif
