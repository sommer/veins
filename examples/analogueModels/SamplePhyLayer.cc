#include "SamplePhyLayer.h"

#include "ThresholdDecider.h"
#include "RandomFreqTimeModel.h"
#include "RandomFrequencyOnlyModel.h"

Define_Module(SamplePhyLayer);


void SamplePhyLayer::initialize(int stage) {
	//call BasePhy's initialize
	PhyLayer::initialize(stage);

	if(stage == 0) {
		myIndex = findHost()->getIndex();

	} else if(stage == 1) {
		//Decider and AnalogueModels are created by the PhyLayer in this stage
	}
}

void SamplePhyLayer::handleMessage(cMessage* msg) {
	if(msg->getKind() == AIR_FRAME) {
		AirFrame* frame = static_cast<AirFrame*>(msg);

		//normally a subclassed phylayer doesn't has to care about these
		//events, we only catch them to display some messages telling the
		//current state of the receiving process
		switch(frame->getState()) {
		case START_RECEIVE:
			if(frame->getSignal().getReceptionStart() != simTime())
				log("Received delayed AirFrame (state=START_RECEIVE). Proceeding it directly to RECEIVING state");
			else
				log("Received AirFrame (state=START_RECEIVE). Proceeding it directly to RECEIVING state");
			break;

		case RECEIVING:
			log("Received scheduled AirFrame for further processing through the decider.");
			break;

		case END_RECEIVE:
			log("Last receive of scheduled AirFrame because AirFrame transmission is over. (state=END_RECEIVE");
			break;

		default:
			break;
		}
	}

	//IF a subclass of PhyLayer overrides the handleMessage method it should
	//make sure to call the base method.
	PhyLayer::handleMessage(msg);
}

void SamplePhyLayer::log(std::string msg) {
	ev << "[Host " << myIndex << "] - PhyLayer: " << msg << endl;
}

/**
 * @brief Creates and initializes a RandomFreqTimeModel with the passed
 * parameter values.
 */
AnalogueModel* SamplePhyLayer::createRandomFreqTimeModel(ParameterMap& params){

	//get the "seed"-parameter from the config
	ParameterMap::iterator it = params.find("seed");

	//create AnalogueModel with default seed if no seed parameter was defined
	if(it == params.end()){
		return new RandomFreqTimeModel();
	}

	long seed = it->second.longValue();
	return new RandomFreqTimeModel(seed);

}

/**
 * @brief Creates and initializes a RandomFreqOnlyModel with the passed
 * parameter values.
 */
AnalogueModel* SamplePhyLayer::createRandomFrequencyOnlyModel(ParameterMap& params){

	//get the "seed"-parameter from the config
	ParameterMap::iterator it = params.find("seed");

	//create AnalogueModel with default seed if no seed parameter was defined
	if(it == params.end()){
		return new RandomFrequencyOnlyModel();
	}

	long seed = it->second.longValue();
	return new RandomFrequencyOnlyModel(seed);

}

AnalogueModel* SamplePhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params) {

	if(name == "RandomFreqTimeModel")
		return createRandomFreqTimeModel(params);
	else if(name == "RandomFrequencyOnlyModel")
		return createRandomFrequencyOnlyModel(params);

	//If we couldn't create the passed analogue model, call the method
	//of our base class.
	//Note: even if all models defined in the xml-config can be handled
	//by this class method, there will be at least the call to create
	//the RadioStateAnalogueModel which in almost every case has to be done
	//by the PhyLayer.
	return PhyLayer::getAnalogueModelFromName(name, params);
}

Decider* SamplePhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {

	if(name == "ThresholdDecider"){
		ParameterMap::iterator it = params.find("threshold");
		if(it == params.end()){
			log("ERROR: No threshold parameter defined for ThresholdDecider!");
			return 0;
		}

		/*
		 * The value for the deciders threshold should be checked here against
		 * the value specified in ConnectionManager that stores the max/min valid value.
		 *
		 */

		return new ThresholdDecider(this, myIndex, it->second.doubleValue());
	}

	return 0;
}
