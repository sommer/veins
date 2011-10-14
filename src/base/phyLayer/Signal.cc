#include "Signal_.h"

Signal::Signal(simtime_t sendingStart, simtime_t duration):
	sendingStart(sendingStart), duration(duration),
	propagationDelay(0),
	power(0), bitrate(0),
	txBitrate(0),
	rcvPower(0)
{}

Signal::Signal(const Signal & o):
	sendingStart(o.sendingStart), duration(o.duration),
	propagationDelay(o.propagationDelay),
	senderMovement(o.senderMovement),
	power(0), bitrate(0),
	txBitrate(0),
	rcvPower(0)
{
	if (o.power) {
		power = o.power->constClone();
	}

	if (o.bitrate) {
		bitrate = o.bitrate->clone();
	}

	if (o.txBitrate) {
		txBitrate = o.txBitrate->clone();
	}

	for(ConstMappingList::const_iterator it = o.attenuations.begin();
		it != o.attenuations.end(); it++){
		attenuations.push_back((*it)->constClone());
	}
}

const Signal& Signal::operator=(const Signal& o) {
	sendingStart     = o.sendingStart;
	duration         = o.duration;
	propagationDelay = o.propagationDelay;
	senderMovement   = o.senderMovement;

	markRcvPowerOutdated();

	if(power){
		delete power;
		power = 0;
	}

	if(bitrate){
		delete bitrate;
		bitrate = 0;
	}

	if(txBitrate){
		delete txBitrate;
		txBitrate = 0;
	}

	if(o.power)
		power = o.power->constClone();

	if(o.bitrate)
		bitrate = o.bitrate->clone();

	if(o.txBitrate)
		txBitrate = o.txBitrate->clone();

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
	if(rcvPower){
		if(propagationDelay != 0){
			assert(rcvPower->getRefMapping() != power);
			delete rcvPower->getRefMapping();
		}

		delete rcvPower;
	}

	if(power)
		delete power;

	if(bitrate)
		delete bitrate;

	if(txBitrate)
		delete txBitrate;

	for(ConstMappingList::iterator it = attenuations.begin();
		it != attenuations.end(); it++) {

		delete *it;
	}
}

simtime_t Signal::getSendingStart() const {
	return sendingStart;
}

simtime_t Signal::getSendingEnd() const {
	return sendingStart + duration;
}

simtime_t Signal::getReceptionStart() const {
	return sendingStart + propagationDelay;
}

simtime_t Signal::getReceptionEnd() const {
	return sendingStart + propagationDelay + duration;
}

simtime_t Signal::getDuration() const{
	return duration;
}

simtime_t Signal::getPropagationDelay() const {
	return propagationDelay;
}

void Signal::setPropagationDelay(simtime_t delay) {
	assert(propagationDelay == 0);
	assert(!txBitrate);

	markRcvPowerOutdated();

	propagationDelay = delay;

	if(bitrate) {
		txBitrate = bitrate;
		bitrate = new DelayedMapping(txBitrate, propagationDelay);
	}
}

void Signal::setTransmissionPower(ConstMapping *power)
{
	if(this->power){
		markRcvPowerOutdated();
		delete this->power;
	}

	this->power = power;
}

void Signal::setBitrate(Mapping *bitrate)
{
	assert(!txBitrate);

	if(this->bitrate)
		delete this->bitrate;

	this->bitrate = bitrate;
}

void Signal::setMove(Move& move) {
	senderMovement = move;
}

Move Signal::getMove() const{
	return senderMovement;
}
