#include "Signal_.h"

/**
 * Initializes a singal with the specified start and length.
 */
Signal::Signal(simtime_t start, simtime_t length):
	signalStart(start), signalLength(length),
	power(0), bitrate(0),
	delayedPower(0), delayedBitrate(0),
	delayed(false),
	rcvPower(0) {}

Signal::Signal(const Signal & o):
	signalStart(o.signalStart), signalLength(o.signalLength),
	senderMovement(o.senderMovement),
	power(0), bitrate(0),
	delayedPower(0), delayedBitrate(0),
	delayed(o.delayed),
	rcvPower(0) {

	if (o.power != 0) {
		power = o.power->clone();

		if (delayed) {
			delayedPower = new DelayedMapping(power, o.delayedPower->getDelay());
		}
	}

	if (o.bitrate != 0) {
		bitrate = o.bitrate->clone();

		if (delayed) {
			delayedBitrate = new DelayedMapping(bitrate, o.delayedBitrate->getDelay());
		}
	}

	for(ConstMappingList::const_iterator it = o.attenuations.begin();
		it != o.attenuations.end(); it++){
		attenuations.push_back((*it)->constClone());
	}
}

const Signal& Signal::operator=(const Signal& o) {
	signalStart = o.signalStart;
	signalLength = o.signalLength;
	senderMovement = o.senderMovement;
	delayed = o.delayed;

	markRcvPowerOutdated();

	if(power){
		delete power;

		if(delayedPower)
			delete delayedPower;
	}

	if(bitrate){
		delete bitrate;

		if(delayedBitrate)
			delete delayedBitrate;
	}

	if(o.power){
		power = o.power->clone();

		if(delayed)
			delayedPower = new DelayedMapping(power, o.delayedPower->getDelay());
		else
			delayedPower = 0;
	}else{
		power = 0;
		delayedPower = 0;
	}

	if(o.bitrate){
		bitrate = o.bitrate->clone();

		if(delayed)
			delayedBitrate = new DelayedMapping(bitrate, o.delayedBitrate->getDelay());
		else
			delayedBitrate = 0;
	}else{
		bitrate = 0;
	}

	for(ConstMappingList::const_iterator it = attenuations.begin();
		it != attenuations.end(); it++){
		delete(*it);
	}

	attenuations.clear();

	for(ConstMappingList::const_iterator it = o.attenuations.begin();
		it != o.attenuations.end(); it++){
		attenuations.push_back((*it)->constClone());
	}

	return *this;
}

Signal::~Signal()
{
	if(power)
		delete power;

	if(bitrate)
		delete bitrate;

	if(delayedPower)
		delete delayedPower;

	if (delayedBitrate)
		delete delayedBitrate;

	if(rcvPower)
		delete rcvPower;

	for(ConstMappingList::iterator it = attenuations.begin();
		it != attenuations.end(); it++) {

		delete *it;
	}
}

/**
 * Returns the point in time when the receiving of the Signal started.
 */
simtime_t Signal::getSignalStart() const {
	return signalStart;
}

void Signal::delaySignalStart(simtime_t delay) {
	signalStart += delay;

	if(!delayed) {
		delayed = true;
		if(power)
			delayedPower = new DelayedMapping(power, delay);
		if(bitrate)
			delayedBitrate = new DelayedMapping(bitrate, delay);
	} else {
		if(delayedPower)
			delayedPower->delayMapping(delay);
		if(delayedBitrate)
			delayedBitrate->delayMapping(delay);
	}
}

simtime_t Signal::getSignalLength() const{
	return signalLength;
}

void Signal::setTransmissionPower(Mapping *power)
{
	if(this->power){
		markRcvPowerOutdated();
		delete this->power;
	}

	assert(!delayed);

	this->power = power;
}

void Signal::setBitrate(Mapping *bitrate)
{
	if(this->bitrate)
		delete this->bitrate;

	assert(!delayed);

	this->bitrate = bitrate;
}

void Signal::setMove(Move& move) {
	senderMovement = move;
}

Move Signal::getMove() const{
	return senderMovement;
}
