#ifndef TESTPHYLAYER_H_
#define TESTPHYLAYER_H_

#include <BasePhyLayer.h>


class TestPhyLayer:public BasePhyLayer{
private:
	class TestDecider:public Decider {
	protected:
		int myIndex;
		std::map<Signal*, int> currentSignals;

		enum {
			FIRST,
			HEADER_OVER,
			SIGNAL_OVER
		};
	public:
		TestDecider(DeciderToPhyInterface* phy, int myIndex):
			Decider(phy), myIndex(myIndex) {}

		virtual simtime_t processSignal(AirFrame* frame) {
			Signal* s = &frame->getSignal();

			std::map<Signal*, int>::iterator it = currentSignals.find(s);
			if(it == currentSignals.end()) {
				currentSignals[s] = HEADER_OVER;

				log("First processing of this signal. Scheduling it to end of header to decide if Signal should be received.");
				return s->getSignalStart() + 0.10 * s->getSignalLength();
			}

			switch(it->second) {
			case HEADER_OVER:
				log("Second receive of a signal from Phy - Deciding if packet should be received - Let's try to receive it.");
				it->second = SIGNAL_OVER;
				return s->getSignalStart() + s->getSignalLength();

			case SIGNAL_OVER:
				log("Last receive of signal from Phy - Deciding if the packet could be received correctly - Let's say its correct.");


				phy->sendUp(frame, new DeciderResult(true));
				currentSignals.erase(it);
				return -1;
			default:
				break;
			}

			//we should never get here!
			return 0;
		}

		void log(std::string msg) {
			ev << "[Host " << myIndex << "] - PhyLayer(Decider): " << msg << endl;
		}
	};

	class TestAnalogueModel:public AnalogueModel {
	public:
		double att;
		int myIndex;
		std::string myName;

		TestAnalogueModel(std::string name, double attenuation, int index):
			att(attenuation), myIndex(index), myName(name) {}

		void filterSignal(Signal& s) {
			log("Filtering signal.");
		}

		void log(std::string msg) {
			ev << "[Host " << myIndex << "] - PhyLayer(" << myName << "): " << msg << endl;
		}
	};
protected:
	int myIndex;

	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

public:
	virtual void initialize(int stage);

	virtual void handleMessage(cMessage* msg);

	void log(std::string msg);
};

#endif /*TESTPHYLAYER_H_*/
