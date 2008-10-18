#include "ChannelState.h"

/**
 * Returns true if the channel is considered idle, meaning
 * no currently incoming signals.
 */
bool ChannelState::isIdle() {
	
	return idle;
}

/**
 * Returns the current RSSI value of the channel.
 */
double ChannelState::getRSSI() {
	
	return rssi;
}

