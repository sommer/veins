#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <omnetpp.h>
#include "Move.h"
#include "Mapping.h"
#include <list>

/**
 * TODO: Write Description at latest the modelation of the signal is final.
 */
class Signal {
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
	
	typedef std::list<ConstMapping*> ConstMappingList;
	
	/** @brief Stores the functions describing the attenuations of the signal*/
	ConstMappingList attenuations;
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
	 * Sets the point in time when the receiving of the Signal started.
	 * This methods is used by the receiving Physical layer to adjust
	 * the signal start by the propagation delay.
	 */
	void setSignalStart(simtime_t start);

	
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
	void addAttenuation(ConstMapping* power) {
		attenuations.push_back(power);
	}
	
	/**
	 * @brief Returns the function representing the transmission power
	 * of the signal.
	 */
	Mapping* getTransmissionPower() {
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
	 * @brief Calculates and returns the receiving power of this Signal
	 * 
	 * The receiving power is calculated by multipliing the transimission
	 * power with the attenuation of every analogue model.
	 */
	Mapping* getReceivingPower() const {
		Mapping* result = power->clone();
		
		//rather slow but streight forward multiply implementation
		//this can be optimized by implementing a multiply method
		//which is able to multiply multiple Mappings at once. 
		for(ConstMappingList::const_iterator it = attenuations.begin();
			it != attenuations.end(); it++){
			
			Mapping* tmp = Mapping::multiply(*result, **it);
			delete result;
			result = tmp;
		}
		return result;
	}
};

#endif /*SIGNAL_H_*/
