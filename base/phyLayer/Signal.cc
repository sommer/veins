#include "Signal_.h"

Signal::Signal(simtime_t start, simtime_t length):
	signalStart(start), signalLength(length),
	power(0), bitrate(0),
	rcvPower(0)
{}

Signal::Signal(const Signal & o):
	signalStart(o.signalStart), signalLength(o.signalLength),
	senderMovement(o.senderMovement),
	power(0), bitrate(0),
	rcvPower(0)
{
	if (o.power != 0) {
		power = o.power->clone();
	}

	if (o.bitrate != 0) {
		bitrate = o.bitrate->clone();
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

	markRcvPowerOutdated();

	if(power){
		delete power;
	}

	if(bitrate){
		delete bitrate;
	}

	if(o.power){
		power = o.power->clone();
	}else{
		power = 0;
	}

	if(o.bitrate){
		bitrate = o.bitrate->clone();
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

	if(rcvPower)
		delete rcvPower;

	for(ConstMappingList::iterator it = attenuations.begin();
		it != attenuations.end(); it++) {

		delete *it;
	}
}

simtime_t Signal::getSignalStart() const {
	return signalStart;
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

	this->power = power;
}

void Signal::setBitrate(Mapping *bitrate)
{
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
