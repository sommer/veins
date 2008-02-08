#include "Signal_.h"

/**
 * Returns the point in time when the receiving of the Signal started.
 */
simtime_t Signal::getSignalStart() const {
	return signalStart;
}

/**
 * Sets the point in time when the receiving of the Signal started.
 * This methods is used by the receiving Physical layer to adjust
 * the signal start by the propagation delay.
 */
void Signal::setSignalStart(simtime_t start) {
	signalStart = start;
}

simtime_t Signal::getSignalLength() {
	return signalLength;
}

void Signal::setMove(Move& move) {
	senderMovement = move;
}

Move Signal::getMove() {
	return senderMovement;
}
