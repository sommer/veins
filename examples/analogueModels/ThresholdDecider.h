#ifndef THRESHOLDDECIDER_H_
#define THRESHOLDDECIDER_H_

#include <string>
#include <cmath>
#include <map>

#include <Decider.h>

/**
 * @brief A simple Decider implementation which only checks for
 * a received signal if the receiving power is above a certain value.
 *
 * Note: This implementation is only meant to be as a quick and ugly
 * Decider to demonstrate (and print) how the AnalogueModels affect the signal.
 * You should not take it as template for a real Decider!
 *
 * @see SNRThresholdDecider for a simple Decider implementation or take
 * a look at the "How to write your own Decider" tutorial at the MiXiM wiki.
 *
 * @ingroup Decider
 * @ingroup exampleAM
 */
class ThresholdDecider:public Decider {
protected:
	int myIndex;

	double threshold;

	/** @brief stores the currently receiving signals together with their
	 * current state.*/
	std::map<Signal*, int> currentSignals;

	enum {
		FIRST,
		HEADER_OVER,
		SIGNAL_OVER
	};

protected:

	/**
	 * @brief handles Signals passed the first time.
	 */
	simtime_t handleNewSignal(Signal* s){
		//set the state the signal will be in the next time we get it
		currentSignals[s] = HEADER_OVER;

		log("First processing of this signal. Scheduling it to end of header to decide if Signal should be received.");
		//we say that the length of the header is at 10% of the length of the signal
		return s->getReceptionStart() + 0.10 * s->getDuration();
	}

	/**
	 * @brief handles the Signal when passed after after end of the header.
	 */
	simtime_t handleHeaderOver(std::map<Signal*, int>::iterator& it){
		log("Second receive of a signal from Phy - Deciding if packet should be received - Let's try to receive it.");
		//we don't really do something after the header, so we only update the next state
		it->second = SIGNAL_OVER;
		return it->first->getReceptionEnd();
	}

	/**
	 * @brief handles the signal when passed at the end of the signal.
	 */
	simtime_t handleSignalOver(std::map<Signal*, int>::iterator& it, AirFrame* frame){
		log("Last receive of signal from Phy - Deciding if the packet could be received correctly...");

		//get the receiving power from the signal (calculated at each call of
		//this method
		//Mapping* receivingPower = it->first->getReceivingPower();


		//------print the mappings----------------------
		ev << "Sending power mapping: " << endl;
		printMapping(it->first->getTransmissionPower());

		ConstMapping* receivingPower;

		simtime_t delay = it->first->getPropagationDelay();
		if(delay == 0)
			receivingPower = it->first->getTransmissionPower()->constClone();
		else
			receivingPower = new ConstDelayedMapping(it->first->getTransmissionPower(), delay);

		std::list<ConstMapping*> attList = it->first->getAttenuation();
		int count = 1;
		for(std::list<ConstMapping*>::const_iterator aIt = attList.begin();
			aIt != attList.end(); ++aIt){

			ev << endl;
			ev << " multiplied with Attenuation " << count;
			switch(count){
			case 1:
				ev << "(Radiostate)";
				break;
			case 2:
				ev << "(Pathloss)";
				break;
			case 3:
				ev << "(Random time and freq attenuation)";
				break;
			case 4:
				ev << "(Random freq only attenuation)";
				break;
			}
			ev << ":" << endl;
			printMapping(*aIt);
			++count;

			ConstMapping* tmp = MappingUtils::multiply(*receivingPower, **aIt, Argument::MappedZero);
			delete receivingPower;
			receivingPower = tmp;

			ev << endl;
			ev << " result:" << endl;
			printMapping(receivingPower);
		}

		ev << endl;
		ev << "Receiving power calculated.\n";
		ev << "Signal receiving power. (Should be same as above)\n";
		printMapping(it->first->getReceivingPower());

		ev << endl;
		ev << "Threshold is " << toDecibel(threshold) << "dbm" << endl;
		//----------------------------------------------

		bool toWeak = false;

		//iterate over receiving power mapping and chekc if every value is bigger
		//then the threshold
		ConstMappingIterator* mIt = receivingPower->createConstIterator();

		while(mIt->inRange()){
			if(mIt->getValue() < threshold){
				// print receiving power if too weak
				ev << "receiving power: " << mIt->getValue() << endl;
				toWeak = true;
				break;
			}

			if(!mIt->hasNext())
				break;

			mIt->next();
		}

		delete mIt;
		delete receivingPower;

		if(toWeak){
			log("...signal is to weak -> discard.");
		} else {
			log("...strong enough -> forwarding it to Mac layer.");
			phy->sendUp(frame, new DeciderResult(true));
		}

		currentSignals.erase(it);
		return -1;
	}


	//----------Utility methods----------------------------
	void log(std::string msg) {
		ev << "[Host " << myIndex << "] - PhyLayer(Decider): " << msg << endl;
	}

	double toDecibel(double v){
		return 10.0 * log10(v);
	}

	template<typename T>
	std::string toString(T v, unsigned int length){
		char* tmp = new char[255];
		sprintf(tmp, "%.2f", v);

		std::string result(tmp);
		delete[] tmp;
		while(result.length() < length)
			result += " ";
		return result;
	}

	std::string toString(simtime_t_cref v, unsigned int length){
		return toString(SIMTIME_DBL(v), length);
	}

	/**
	 * @brief Quick and ugly printing of a two dimensional mapping.
	 */
	void printMapping(ConstMapping* m){
		m->print(ev.getOStream(), 1000., 1e-9, "GHz\\ms", &Dimension::frequency);
	}

public:
	ThresholdDecider(DeciderToPhyInterface* phy, int myIndex, double threshold):
		Decider(phy), myIndex(myIndex), threshold(threshold) {}

	/**
	 * @brief this method is called by the BasePhylayer whenever it gets
	 * a AirFrame (from another phy or self scheduled).
	 */
	virtual simtime_t processSignal(AirFrame* frame) {
		Signal* s = &frame->getSignal();

		//Check if we already know this signal...
		std::map<Signal*, int>::iterator it = currentSignals.find(s);

		//if not, we handle it as a new signal
		if(it == currentSignals.end()) {
			return handleNewSignal(s);

		//otherwise handle it depending on the state it currently is in
		} else {
			switch(it->second) {
			case HEADER_OVER:
				return handleHeaderOver(it);

			case SIGNAL_OVER:
				return handleSignalOver(it, frame);

			default:
				break;
			}
		}

		//we should never get here!
		assert(false);
		return 0;
	}

	virtual ChannelState getChannelState() {
		return ChannelState(false, 0);
	}
	virtual simtime_t handleChannelSenseRequest(ChannelSenseRequest*) {
		return -1;
	}
};

#endif /*TESTDECIDER_H_*/
