#include <NicControlType.h>

#ifndef CONSTANTS_802_11
#define CONSTANTS_802_11

enum ControlTypes_802_11 {
    NOTHING = NicControlType::NOTHING,
    TRANSMISSION_OVER = NicControlType::TRANSMISSION_OVER,
    PACKET_DROPPED = NicControlType::PACKET_DROPPED, 
    BITERROR,       //the phy has recognized a bit error in the packet
    COLLISION       //packet lost due to collision
};

/** @brief frame kinds */
enum frameType_802_11 {

  //between MAC layers of two nodes
  RTS = COLLISION + 1, // request to send
  CTS,                 // clear to send
  ACK,                 // acknowledgement
  DATA,
  BROADCAST
};

/** @brief Bit rates for 802.11b */
const double BITRATES_80211[] = {
    1000000,
    2000000,
    5500000,
    11000000
};


/** @brief duration of the PHY header
 *
 * If the radio was switched to
 * late, a partially received header is still ok, the total received
 * packet duration can be shorter by this amount compared to the send
 * packet.
 */ 
const double RED_PHY_HEADER_DURATION = 0.000120;

/** @brief Length of PLCP header and Preamble */
const int PHY_HEADER_LENGTH=192;
/** @brief PLCP header */
const int HEADER_WITHOUT_PREAMBLE=48;
/** @brief Bitrate with which the header is send */
const double BITRATE_HEADER=1E+6;
/** @brief 3dB filter bandwidth -- channels overlap! */
const double BANDWIDTH=20E+6;

const int MAC_GENERATOR = 5;

/** @brief standard frame lengths
    @{ */
const unsigned int LENGTH_RTS = 160;
const unsigned int LENGTH_CTS = 112;
const unsigned int LENGTH_ACK = 112;
/** @} */

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
const double DIFS = 2*ST + SIFS;
/** @brief extended interframe space
 *  whenever a corrupted frame is received, defer actions for this time
 */
const double EIFS = SIFS + DIFS +  (PHY_HEADER_LENGTH + LENGTH_ACK)/BITRATE_HEADER;

/** @brief We have to stop transmitting somewhen.
 *
 * This is the default value from the standard, it should be configurable
 */
const int LONG_RETRY_LIMIT = 4;

/** @brief Imcrement longRetryCounter when this limit is met
 *
 * This is the default value from the standard
 */
const int SHORT_RETRY_LIMIT = 7;

/**
 * Minimum size (initial size) of contention window, assuming DS PHY
 */
const int CW_MIN = 31;

/** Maximum size of contention window, assuming DS PHY */
const int CW_MAX = 1023;

#endif

