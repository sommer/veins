#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <omnetpp.h>
#include "Move.h"
#include "Mapping.h"
#include <list>

class DelayedMappingIterator: public FilteredMappingIterator {
protected:
	simtime_t delay;

	Argument position;
	Argument nextPosition;
protected:
	Argument delayPosition(const Argument& pos) const{
		Argument res(pos);
		res.setTime(res.getTime() - delay);
		return res;
	}

	Argument undelayPosition(const Argument& pos) const {
		Argument res(pos);
			res.setTime(res.getTime() + delay);
			return res;
	}

	void updatePosition() {
		nextPosition = undelayPosition(origIterator->getNextPosition());
		position = undelayPosition(origIterator->getPosition());
	}

public:
	DelayedMappingIterator(MappingIterator* it, simtime_t delay):
		FilteredMappingIterator(it), delay(delay) {

		updatePosition();
	}

	virtual const Argument& getNextPosition() const { return nextPosition; }

	virtual void jumpTo(const Argument& pos) {
		origIterator->jumpTo(delayPosition(pos));
		updatePosition();
	}

	virtual void jumpToBegin() {
		origIterator->jumpToBegin();
		updatePosition();
	}

	virtual void iterateTo(const Argument& pos) {
		origIterator->iterateTo(delayPosition(pos));
		updatePosition();
	}

	virtual void next() {
		origIterator->next();
		updatePosition();
	}

	virtual const Argument& getPosition() const {
		return position; }
};

class DelayedMapping: public Mapping {
protected:
	Mapping* mapping;
	simtime_t delay;


protected:
	Argument delayPosition(const Argument& pos) const {
		Argument res(pos);
		res.setTime(res.getTime() - delay);
		return res;
	}

public:
	DelayedMapping(Mapping* mapping, simtime_t delay):
		Mapping(mapping->getDimensionSet()), mapping(mapping), delay(delay) {}

	virtual double getValue(const Argument& pos) const {
		return mapping->getValue(delayPosition(pos));
	}

	virtual void setValue(const Argument& pos, double value) {
		mapping->setValue(delayPosition(pos), value);
	}

	virtual Mapping* clone() const {
		return new DelayedMapping(mapping->clone(), delay);
	}

	virtual MappingIterator* createIterator() {
		return new DelayedMappingIterator(mapping->createIterator(), delay);
	}

	virtual MappingIterator* createIterator(const Argument& pos) {
		return new DelayedMappingIterator(mapping->createIterator(delayPosition(pos)), delay);
	}

	/**
	 * @brief Returns the delay used by this mapping.
	 */
	virtual simtime_t getDelay() const {
		return delay;
	}

	/**
	 * @brief Changes the delay to the passed value.
	 */
	virtual void delayMapping(simtime_t d) {
		delay += d;
	}
};

/**
 * TODO: Write Description at latest the modelation of the signal is final.
 */
class Signal {
public:
	typedef ConcatConstMapping<std::multiplies<double> > MultipliedMapping;

protected:
	/** @brief The start of the signal transmission.*/
	simtime_t signalStart;
	/** @brief The length of the signal transmission.*/
	simtime_t signalLength;
	/** @brief The movement of the sending host.*/
	Move senderMovement;

	/** @brief Stores the function which deschribes the power of the signal*/
	Mapping* power;

	/** @brief Stores the function which describes the bitrate of the signal*/
	Mapping* bitrate;

	DelayedMapping* delayedPower;

	DelayedMapping* delayedBitrate;

	typedef std::list<ConstMapping*> ConstMappingList;

	/** @brief Stores the functions describing the attenuations of the signal*/
	ConstMappingList attenuations;

	/** @brief Stores if the Signal has been delayed (by propagation delay for example).*/
	bool delayed;

	MultipliedMapping* rcvPower;

protected:
	void markRcvPowerOutdated() {
		if(rcvPower){
			delete rcvPower;
			rcvPower = 0;
		}
	}
public:

	/**
	 * Initializes a signal with the specified start and length.
	 */
	Signal(simtime_t start = -1.0, simtime_t length = -1.0);

	/**
	 * @brief Overwrites the copy constructor to make sure that the
	 * mappings are cloned correct.
	 */
	Signal(const Signal& o);

	/**
	 * @brief Overwrites the copy operator to make sure that the
	 * mappings are cloned correct.
	 */
	const Signal& operator=(const Signal& o);

	/**
	 * Delete the functions of this signal.
	 */
	~Signal();

	/**
	 * Returns the point in time when the receiving of the Signal started.
	 */
	simtime_t getSignalStart() const;

	/**
	 * @brief Delays the point in time when the receiving of the Signal started.
	 *
	 * The delay affects only transmission power and bitrate. It is assumed
	 * that attenuations and receiving power are set and get after delaying the signal.
	 *
	 * This methods is used by the receiving Physical layer to adjust
	 * the signal start by the propagation delay.
	 */
	void delaySignalStart(simtime_t delay);


	/**
	 * @brief Returns the movement of the sending host.
	 */
	Move getMove() const;

	/**
	 * @brief Sets the movement of the sending host.
	 */
	void setMove(Move& move);

	/**
	 * @brief Returns the lenth of the signal transmission.
	 */
	simtime_t getSignalLength() const;

	/**
	 * @brief Sets the function representing the transmission power
	 * of the signal.
	 *
	 * The ownership of the passed pointer goes to the signal.
	 */
	void setTransmissionPower(Mapping* power);

	/**
	 * @brief Sets the function representing the bitrate of the signal.
	 *
	 * The ownership of the passed pointer goes to the signal.
	 */
	void setBitrate(Mapping* bitrate);

	/**
	 * @brief Adds a function representing an attenuation of the signal.
	 *
	 * The ownership of the passed pointer goes to the signal.
	 */
	void addAttenuation(ConstMapping* att) {
		attenuations.push_back(att);

		if(rcvPower)
			rcvPower->addMapping(att);
	}

	/**
	 * @brief Returns the function representing the transmission power
	 * of the signal.
	 */
	Mapping* getTransmissionPower() {
		if(delayed)
			return delayedPower;

		return power;
	}

	/**
	 * @brief Returns the function representing the transmission power
	 * of the signal.
	 */
	ConstMapping* getTransmissionPower() const {
		if(delayed)
			return delayedPower;

		return power;
	}

	/**
	 * @brief Returns the function representing the bitrate of the
	 * signal.
	 */
	Mapping* getBitrate() {
		if(delayed)
			return delayedBitrate;

		return bitrate;
	}

	/**
	 * @brief Returns a list of functions representing the attenuations
	 * of the signal.
	 */
	const ConstMappingList& getAttenuation() const {
		return attenuations;
	}

	/**
	 * @brief Calculates and returns the receiving power of this Signal.
	 * Ownership of the returned mapping belongs to this class.
	 *
	 * The receiving power is calculated by multipliing the transimission
	 * power with the attenuation of every analogue model.
	 */
	MultipliedMapping* getReceivingPower() {
		if(!rcvPower)
		{
			Mapping* actPower = power;
			if(delayed)
				actPower = delayedPower;

			rcvPower = new MultipliedMapping(actPower,
											  attenuations.begin(),
											  attenuations.end(),
											  std::multiplies<double>());
		}

		return rcvPower;
	}
};

#endif /*SIGNAL_H_*/
