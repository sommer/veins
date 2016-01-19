#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <list>
#include <omnetpp.h>

#include "veins/base/utils/MiXiMDefs.h"
#include "veins/base/phyLayer/Mapping.h"

/**
 * @brief The signal class stores the physical representation of the
 * signal of an AirFrame.
 *
 * This includes start, duration and propagation delay of the signal,
 * Mappings which represent the transmission power, bitrate, attenuations
 * caused by effects of the channel on the signal during its transmission
 * and the receiving power.
 *
 * Note: Although the Signal itself has a signalSendingStart parameter the
 * Mappings it contains should use absolute time positions to store the values
 * at (NOT relative to the start time of the signal).
 *
 * The Signal is created at the senders MAC layer which has to define
 * the TX-power- and the bitrate Mapping.
 * Sending start time and duration is added at the sender's physical layer.
 * Attenuation Mappings are added to the Signal by the
 * AnalogueModels of the receiver's physical layer.
 * The RX-power Mapping is calculated on demand by multiplying the
 * TX-power Mapping with every attenuation Mapping of the signal.
 *
 * @ingroup phyLayer
 */
class MIXIM_API Signal {
public:
	/**
	 * @brief Shortcut type for a concatenated Mapping using multiply operator.
	 *
	 * Used to define the receiving power mapping.
	 */
	typedef ConcatConstMapping<std::multiplies<double> > MultipliedMapping;
	/** @brief Shortcut type for a list of ConstMappings.*/
	typedef std::list<ConstMapping*> ConstMappingList;

protected:
	/** @brief Sender module id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
	int senderModuleID;
	/** @brief Sender gate id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
	int senderFromGateID;
	/** @brief Receiver module id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
	int receiverModuleID;
	/** @brief Receiver gate id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
	int receiverToGateID;
	/** @brief The start of the signal transmission at the sender module.*/
	simtime_t sendingStart;
	/** @brief The duration of the signal transmission.*/
	simtime_t duration;
	/** @brief The propagation delay of the transmission. */
	simtime_t propagationDelay;

	/** @brief Stores the function which describes the power of the signal*/
	ConstMapping* power;

	/** @brief Stores the function which describes the bitrate of the signal*/
	Mapping* bitrate;

	/** @brief If propagation delay is not zero this stores the undelayed bitrate*/
	Mapping* txBitrate;

	/** @brief Stores the functions describing the attenuations of the signal*/
	ConstMappingList attenuations;

	/** @brief Stores the mapping defining the receiving power of the signal.*/
	MultipliedMapping* rcvPower;

protected:
	/**
	 * @brief Deletes the rcvPower mapping member because it became
	 * out-dated.
	 *
	 * This happens when transmission power or propagation delay changes.
	 */
	void markRcvPowerOutdated() {
		if(rcvPower){
			if(propagationDelay != 0) {
				assert(rcvPower->getRefMapping() != power);
				delete rcvPower->getRefMapping();
			}
			delete rcvPower;
			rcvPower = 0;
		}
	}
public:

	/**
	 * @brief Initializes a signal with the specified start and length.
	 */
	Signal(simtime_t_cref start = -1.0, simtime_t_cref length = -1.0);

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
	 * @brief Delete the functions of this signal.
	 */
	~Signal();

	/**
	 * @brief Returns the point in time when the sending of the Signal started
	 * at the sender module.
	 */
	simtime_t_cref getSendingStart() const;

	/**
	 * @brief Returns the point in time when the sending of the Signal ended
	 * at the sender module.
	 */
	simtime_t getSendingEnd() const;

	/**
	 * @brief Returns the point in time when the receiving of the Signal started
	 * at the receiver module. Already includes the propagation delay.
	 */
	simtime_t getReceptionStart() const;

	/**
	 * @brief Returns the point in time when the receiving of the Signal ended
	 * at the receiver module. Already includes the propagation delay.
	 */
	simtime_t getReceptionEnd() const;

	/**
	 * @brief Returns the duration of the signal transmission.
	 */
	simtime_t_cref getDuration() const;

	/**
	 * @brief Returns the propagation delay of the signal.
	 */
	simtime_t_cref getPropagationDelay() const;

	/**
	 * @brief Sets the propagation delay of the signal.
	 *
	 * This should be only set by the sending physical layer.
	 */
	void setPropagationDelay(simtime_t_cref delay);

	/**
	 * @brief Sets the function representing the transmission power
	 * of the signal.
	 *
	 * The ownership of the passed pointer goes to the signal.
	 */
	void setTransmissionPower(ConstMapping* power);

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
		//assert the attenuation wasn't already added to the list before
		assert(std::find(attenuations.begin(), attenuations.end(), att) == attenuations.end());

		attenuations.push_back(att);

		if(rcvPower)
			rcvPower->addMapping(att);
	}

	/**
	 * @brief Returns the function representing the transmission power
	 * of the signal.
	 *
	 * Be aware that the transmission power mapping is not yet affected
	 * by the propagation delay!
	 */
	ConstMapping* getTransmissionPower() {
		return power;
	}

	/**
	 * @brief Returns the function representing the transmission power
	 * of the signal.
	 *
	 * Be aware that the transmission power mapping is not yet affected
	 * by the propagation delay!
	 */
	const ConstMapping* getTransmissionPower() const {
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
			ConstMapping* tmp = power;
			if(propagationDelay != 0) {
				tmp = new ConstDelayedMapping(power, propagationDelay);
			}
			rcvPower = new MultipliedMapping(tmp,
											  attenuations.begin(),
											  attenuations.end(),
											  false, Argument::MappedZero());
		}

		return rcvPower;
	}

	/**
	 * Returns a pointer to the arrival module. It returns NULL if the signal
	 * has not been sent/received yet, or if the module was deleted
	 * in the meantime.
	 */
	cModule *getReceptionModule() const {return receiverModuleID < 0 ? NULL : getSimulation()->getModule(receiverModuleID);}

	/**
	 * Returns pointers to the gate from which the signal was sent and
	 * on which gate it arrived. A NULL pointer is returned
	 * for new (unsent) signal.
	 */
	cGate *getReceptionGate() const;

	/**
	 * Returns a pointer to the sender module. It returns NULL if the signal
	 * has not been sent/received yet, or if the sender module got deleted
	 * in the meantime.
	 */
	cModule *getSendingModule() const {return senderModuleID < 0 ? NULL : getSimulation()->getModule(senderModuleID);}

	/**
	 * Returns pointers to the gate from which the signal was sent and
	 * on which gate it arrived. A NULL pointer is returned
	 * for new (unsent) signal.
	 */
	cGate *getSendingGate() const;

	/** @brief Saves the arrival sender module information form message. */
	void setReceptionSenderInfo(const cMessage *const pMsg);
};

#endif /*SIGNAL_H_*/
