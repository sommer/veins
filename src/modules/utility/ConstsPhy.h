//
// Copyright (C) 2014 Michele Segata <segata@ccs-labs.org>
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

#ifndef CONSTANTS_PHY
#define CONSTANTS_PHY

#include <omnetpp.h>
#include <cmath>
#include <stdint.h>

/** @brief Modulation and coding scheme to be used for transmission */
enum PHY_MCS {
	//use the default MCS
	MCS_DEFAULT = -1,
	MCS_OFDM_BPSK_R_1_2,
	MCS_OFDM_BPSK_R_3_4,
	MCS_OFDM_QPSK_R_1_2,
	MCS_OFDM_QPSK_R_3_4,
	MCS_OFDM_QAM16_R_1_2,
	MCS_OFDM_QAM16_R_3_4,
	MCS_OFDM_QAM64_R_2_3,
	MCS_OFDM_QAM64_R_3_4
};

/** @brief Available bandwidths */
enum Bandwidth {
	BW_OFDM_5_MHZ,
	BW_OFDM_10_MHZ,
	BW_OFDM_20_MHZ
};

/** @brief Given bandwidth and MCS returns datarate in bits per second */
inline uint64_t getOfdmDatarate(enum PHY_MCS mcs, enum Bandwidth bw) {
	//divide datarate by div, depending on bandwidth
	uint64_t div;
	//datarate to be returned
	uint64_t dr;
	switch (bw) {
	case BW_OFDM_5_MHZ:
		div = 4;
		break;
	case BW_OFDM_10_MHZ:
		div = 2;
		break;
	case BW_OFDM_20_MHZ:
	default:
		div = 1;
		break;
	}
	switch (mcs) {
	case MCS_OFDM_BPSK_R_1_2:
		dr = 6000000;
		break;
	case MCS_OFDM_BPSK_R_3_4:
		dr = 9000000;
		break;
	case MCS_OFDM_QPSK_R_1_2:
		dr = 12000000;
		break;
	case MCS_OFDM_QPSK_R_3_4:
		dr = 18000000;
		break;
	case MCS_OFDM_QAM16_R_1_2:
		dr = 24000000;
		break;
	case MCS_OFDM_QAM16_R_3_4:
		dr = 36000000;
		break;
	case MCS_OFDM_QAM64_R_2_3:
		dr = 48000000;
		break;
	case MCS_OFDM_QAM64_R_3_4:
		dr = 54000000;
		break;
	default:
		dr = 6000000;
		break;
	}
	return (dr / div);
}

/** @brief returns the number of databits per ofdm symbol */
inline uint32_t getNDBPS(enum PHY_MCS mcs) {
	uint32_t ndbps;
	switch (mcs) {
	case MCS_OFDM_BPSK_R_1_2:
		ndbps = 24;
		break;
	case MCS_OFDM_BPSK_R_3_4:
		ndbps = 36;
		break;
	case MCS_OFDM_QPSK_R_1_2:
		ndbps = 48;
		break;
	case MCS_OFDM_QPSK_R_3_4:
		ndbps = 72;
		break;
	case MCS_OFDM_QAM16_R_1_2:
		ndbps = 96;
		break;
	case MCS_OFDM_QAM16_R_3_4:
		ndbps = 144;
		break;
	case MCS_OFDM_QAM64_R_2_3:
		ndbps = 192;
		break;
	case MCS_OFDM_QAM64_R_3_4:
		ndbps = 216;
		break;
	default:
		ndbps = 24;
		break;
	}
	return ndbps;
}

/** @brief returns the bandwidth in Hz */
inline uint64_t getBandwidth(enum Bandwidth bw) {
	switch (bw) {
	case BW_OFDM_5_MHZ:
		return 5000000;
		break;
	case BW_OFDM_10_MHZ:
		return 10000000;
		break;
	default:
		return 20000000;
		break;
	}
	return 20000000;
}

/** @brief returns encoding given datarate */
inline enum PHY_MCS getMCS(uint64_t datarate, enum Bandwidth bw) {
	if (bw == BW_OFDM_10_MHZ) {
		if (datarate == 3000000) {
			return MCS_OFDM_BPSK_R_1_2;
		}
		if (datarate == 4500000) {
			return MCS_OFDM_BPSK_R_3_4;
		}
		if (datarate == 6000000) {
			return MCS_OFDM_QPSK_R_1_2;
		}
		if (datarate == 9000000) {
			return MCS_OFDM_QPSK_R_3_4;
		}
		if (datarate == 12000000) {
			return MCS_OFDM_QAM16_R_1_2;
		}
		if (datarate == 18000000) {
			return MCS_OFDM_QAM16_R_3_4;
		}
		if (datarate == 24000000) {
			return MCS_OFDM_QAM64_R_2_3;
		}
		if (datarate == 27000000) {
			return MCS_OFDM_QAM64_R_3_4;
		}
	}
	if (bw == BW_OFDM_20_MHZ) {
		if (datarate == 6000000) {
			return MCS_OFDM_BPSK_R_1_2;
		}
		if (datarate == 9000000) {
			return MCS_OFDM_BPSK_R_3_4;
		}
		if (datarate == 12000000) {
			return MCS_OFDM_QPSK_R_1_2;
		}
		if (datarate == 18000000) {
			return MCS_OFDM_QPSK_R_3_4;
		}
		if (datarate == 24000000) {
			return MCS_OFDM_QAM16_R_1_2;
		}
		if (datarate == 36000000) {
			return MCS_OFDM_QAM16_R_3_4;
		}
		if (datarate == 48000000) {
			return MCS_OFDM_QAM64_R_2_3;
		}
		if (datarate == 54000000) {
			return MCS_OFDM_QAM64_R_3_4;
		}
	}
	if (bw == BW_OFDM_5_MHZ) {
		if (datarate == 1500000) {
			return MCS_OFDM_BPSK_R_1_2;
		}
		if (datarate == 2250000) {
			return MCS_OFDM_BPSK_R_3_4;
		}
		if (datarate == 3000000) {
			return MCS_OFDM_QPSK_R_1_2;
		}
		if (datarate == 4500000) {
			return MCS_OFDM_QPSK_R_3_4;
		}
		if (datarate == 6000000) {
			return MCS_OFDM_QAM16_R_1_2;
		}
		if (datarate == 9000000) {
			return MCS_OFDM_QAM16_R_3_4;
		}
		if (datarate == 12000000) {
			return MCS_OFDM_QAM64_R_2_3;
		}
		if (datarate == 13500000) {
			return MCS_OFDM_QAM64_R_3_4;
		}
	}
	ASSERT2(false, "Invalid datarate for required bandwidth");
	return MCS_DEFAULT;
}

#endif
