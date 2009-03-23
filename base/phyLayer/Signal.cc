#include "Signal_.h"

Signal::Signal(simtime_t start, simtime_t length):
	signalStart(start), signalLength(length),
	propDelay(0),
	power(0), bitrate(0),
	txBitrate(0),
	rcvPower(0)
{}

Signal::Signal(const Signal & o):
	signalStart(o.signalStart), signalLength(o.signalLength),
	propDelay(o.propDelay),
	senderMovement(o.senderMovement),
	power(0), bitrate(0),
	txBitrate(0),
	rcvPower(0)
{
	if (o.power) {
		power = o.power->clone();
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
	signalStart = o.signalStart;
	signalLength = o.signalLength;
	propDelay = o.propDelay;
	senderMovement = o.senderMovement;

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
		power = o.power->clone();

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
		if(propDelay != 0){
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

simtime_t Signal::getSignalStart() const {
	return signalStart + propDelay;
}

simtime_t Signal::getSignalLength() const{
	return signalLength;
}

simtime_t Signal::getPropagationDelay() const {
	return propDelay;
}

void Signal::setPropagationDelay(simtime_t delay) {
	assert(propDelay == 0);
	assert(!txBitrate);

	markRcvPowerOutdated();

	propDelay = delay;
	txBitrate = bitrate;
	bitrate = new DelayedMapping(txBitrate, propDelay);
}

void Signal::setTransmissionPower(Mapping *power)
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
