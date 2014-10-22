//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef CONSTANTS_802_11p
#define CONSTANTS_802_11p

#include <stdint.h>

/** @brief Bit rates for 802.11p
 *
 * as defined in Table 17-14 MIB attribute default values/ranges in the IEEE 802.11-2007 standard
 */
const uint64_t NUM_BITRATES_80211P = 8;
const uint64_t BITRATES_80211P[] = {
	3000000,
	4500000,
	6000000,
	9000000,
	12000000,
	18000000,
	24000000,
	27000000
};

/** @brief Number of Data Bits Per Symbol (N_NBPS) corresponding to bitrates in BITRATES_80211P
 *
 * as defined in Table 17-3 in the IEEE 802.11-2007 standard
 */
const uint32_t N_DBPS_80211P[] = {
	24,
	36,
	48,
	72,
	96,
	144,
	192,
	216
};

/** @brief Symbol interval
 *
 * as defined in Table 17-4 in the IEEE 802.11-2007 standard
 */
const double T_SYM_80211P = 8e-6;


/** @brief Length of PHY HEADER
 *
 * as defined in 17.3.2 PLCP frame format in the IEEE 802.11-2007 standard
 * 40bit header + 6 bit tail */
const int PHY_HDR_TOTAL_LENGTH = 46;

/** @brief Duration of the PLCP Preamble
 *
 * as defined in Table 17.4 Timing-related parameters in the IEEE 802.11-2007 standard
 */
const double PHY_HDR_PREAMBLE_DURATION = 32e-6;

/** @brief Duration of the PLCP Signal
 *
 * as defined in Table 17.4 Timing-related parameters in the IEEE 802.11-2007 standard
 */
const double PHY_HDR_PLCPSIGNAL_DURATION = 8e-6;

/** @brief Length of the PLCP Signal
 *
 * as defined in Figure 17.1 PPDU frame format in the IEEE 802.11-2007 standard
 */
const int PHY_HDR_PLCPSIGNAL_LENGTH = 24;

/** @brief Lenght of the PhyHeader sent with normal bitrate
 *
 */
const int PHY_HDR_PSDU_HEADER_LENGTH = PHY_HDR_TOTAL_LENGTH - PHY_HDR_PLCPSIGNAL_LENGTH;

/** @brief Bitrate of the PLCP Signal
 *
 * as defined in Table 17.4 Timing-related parameters in the IEEE 802.11-2007 standard
 * 24 bits in 8e-6 seconds
 */
const uint64_t PHY_HDR_BITRATE = 3000000;

/** @brief Slot Time for 10 MHz channel spacing
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const SimTime SLOTLENGTH_11P = SimTime().setRaw(13000000UL);

/** @brief Short interframe space
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const SimTime SIFS_11P = SimTime().setRaw(32000000UL);

/** @brief Time it takes to switch from Rx to Tx Mode
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const SimTime RADIODELAY_11P = SimTime().setRaw(1000000UL);

/** @brief Contention Window minimal size
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const unsigned CWMIN_11P = 15;

/** @brief Contention Window maximal size
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const unsigned CWMAX_11P = 1023;

/** @brief 1609.4 slot length
 *
 * as defined in Table H.1 in the IEEE 1609.4-2010 standard
 */
const SimTime SWITCHING_INTERVAL_11P = SimTime().setRaw(50000000000UL);

/** @brief 1609.4 slot length
 *
 * as defined in Table H.1 in the IEEE 1609.4-2010 standard
 * It is the sum of SyncTolerance and MaxChSwitchTime as defined in 6.2.5 in the IEEE 1609.4-2010 Standard
 */
const SimTime GUARD_INTERVAL_11P = SimTime().setRaw(4000000000UL);


/** @brief Channels as reserved by the FCC
 *
 */
namespace Channels {
enum ChannelNumber {
	CRIT_SOL = 172,
	SCH1 = 174,
	SCH2 = 176,
	CCH = 178,
	SCH3 = 180,
	SCH4 = 182,
	HPPS = 184
};
}

enum t_channel {
	type_CCH=0,
	type_SCH,
};

#endif
