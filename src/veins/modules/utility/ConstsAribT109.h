/*
 * ConstsAribT109.h
 *
 *  Created on: Nov 4, 2015
 *      Author: julian.heinovski
 */

#ifndef CONSTSARIBT109_H_
#define CONSTSARIBT109_H_


/**  Bit rates for ARIB T109
 *
 * as defined in 4.5.2.1.4(2) attribute default values/ranges in the IEEE 802.11-2007 standard
 */
const uint64_t NUM_BITRATES_ARIB = 6;
const uint64_t BITRATES_ARIB[] = {
    // default qpsk 1/2
    6000000,
    // bpsk 1/2
    3000000,
    // bpsk 3/4
    4500000,
    //qpsk 3/4
    9000000,
    // 16qam 1/2
    12000000,
    // 16 qam 3/4
    18000000
};

/**
 * @brief Limit of total transmission time (inclusive short interframe space) for base stations
 *
 * If this limit is reached the base station shall discard MacPkts which are to be send.
 *
 * as defined in ch 4.3.4.5 of ARIB T109 STD v1.2
 */
const simtime_t MAC_BASE_STATION_TOTAL_TRANSMISSION_TIME_LIMIT_INCLUSIVE_SHORT_INTERFRAME_SPACE = simtime_t().setRaw(
        10500000000UL);

/** @brief Slot Time for 10 MHz channel spacing
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const simtime_t SLOT_TIME_ARIB = simtime_t().setRaw(13000000UL);

/** @brief Short interframe space
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const simtime_t SIFS_ARIB = simtime_t().setRaw(32000000UL);

/** @brief Distributed space
 *
 * as defined in ch. 4.2.4.3.1 in ARIB T109 STD v1.2
 */
const simtime_t DISTRIBUTED_SPACE = SIFS_ARIB + 2 * SLOT_TIME_ARIB;

const simtime_t TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD = simtime_t().setRaw(6240000000UL);
const int TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD_US = 6240;

const simtime_t MAX_RVC_PERIOD_LENGTH = simtime_t().setRaw(3024000000UL);
const int MAX_RVC_PERIOD_LENGTH_US = 3024;

/**
 * Guard Time
 * OGT
 * The time which is added to both sides of the roadside period.
 * In us.
 * Values 4..63 * 16us
 * Default value is 4*16 (64us)
 */
const simtime_t GUARD_TIME_ARIB = SimTime().setRaw(64000000UL);

#endif /* CONSTSARIBT109_H_ */
