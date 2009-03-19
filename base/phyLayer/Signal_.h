#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <omnetpp.h>
#include "Move.h"
#include "Mapping.h"
#include <list>

/**
 * @brief The signal class stores the physical representation of the signal of an AirFrame.
 *
 * This includes start and duration of the signal, the sender hosts move pattern as well
 * as Mappings which represent the transmission power, bitrate, attenuations caused by
 * effects of the channel on the signal during its transmission and the receiving power.
 *
 * The Signal is created at the senders MAC layer which has to define the TX-power- and the
 * bitrate Mapping.
 * The sender hosts move pattern as well as start and duration is added at the senders
 * physical layer.
 * Attenuation Mappings are added to the Signal by the  AnalogueModels of the receivers
 * physical layer.
 * The RX-power Mapping is calculated on demand by multiplying the TX-power Mapping with
 * every attenuation Mapping of the signal.
 *
 * @ingroup phyLayer
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

	/** @brief Stores the function which describes the power of the signal*/
	Mapping* power;

	/** @brief Stores the function which describes the bitrate of the signal*/
	Mapping* bitrate;

	typedef std::list<ConstMapping*> ConstMappingList;

	/** @brief Stores the functions describing the attenuations of the signal*/
	ConstMappingList attenuations;

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
	 * @brief Returns the movement of the sending host.
	 */
	Move getMove() const;

	/**
	 * @brief Sets the movement of the sending host.
	 */
	void setMove(Move& move);

	/**
	 * @brief Returns the length of the signal transmission.
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
		return power;
	}

	/**
	 * @brief Returns the function representing the transmission power
	 * of the signal.
	 */
	ConstMapping* getTransmissionPower() const {
		return power;
	}

	/**
	 * @brief Returns the function representing the bitrate of the
	 * signal.
	 */
	Mapping* getBitrate() {
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
	 * The receiving power is calculated by multiplying the transmission
	 * power with the attenuation of every receiving phys AnalogueModel.
	 */
	MultipliedMapping* getReceivingPower() {
		if(!rcvPower)
		{
			rcvPower = new MultipliedMapping(power,
											  attenuations.begin(),
											  attenuations.end(),
											  std::multiplies<double>());
		}

		return rcvPower;
	}
};

#endif /*SIGNAL_H_*/
