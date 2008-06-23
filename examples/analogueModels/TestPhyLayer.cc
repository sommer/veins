#include "TestPhyLayer.h"

#include "ThresholdDecider.h"
#include "RandomFreqTimeModel.h"
#include "RandomFrequencyOnlyModel.h"
#include "PathlossModel.h"

Define_Module(TestPhyLayer);


void TestPhyLayer::initialize(int stage) {
	//call BasePhy's initialize
	BasePhyLayer::initialize(stage);
	
	if(stage == 0) {
		myIndex = findHost()->index();
		DEFAULT_ALPHA = 3.5;
		DEFAULT_CARRIER_FREQ = 2.412e+9;
		
	} else if(stage == 1) {
		//Decider and AnalogueModels are created by the BasePhyLayer in this stage
	}
}

void TestPhyLayer::handleMessage(cMessage* msg) {	
	if(msg->kind() == AIR_FRAME) {
		AirFrame* frame = static_cast<AirFrame*>(msg);
		
		//normally a subclassed phylayer doesn't has to care about these
		//events, we only catch them to display some messages telling the
		//current state of the receiving process
		switch(frame->getState()) {
		case FIRST_RECEIVE:
			log("Received new AirFrame (state=FIRST_RECEIVE)");
			if(!usePropagationDelay) {
				log("Since simualtion of propagation delay is disabled we proceed the AirFrame directly to state START_RECEIVE.");
			}
			break;
			
		case START_RECEIVE:
			log("Received delayed AirFrame (state=START_RECEIVE). Proceeding it directly to RECEIVING state");
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
	
	//IF a subclass of BasePhyLayer overrides the handleMessage method it should
	//make sure to call the base method.
	BasePhyLayer::handleMessage(msg);	
}

void TestPhyLayer::log(std::string msg) {
	ev << "[Host " << myIndex << "] - PhyLayer: " << msg << endl;
}


/**
 * @brief Creates and initializes a PathlossModel with the
 * passed parameter values.
 */
AnalogueModel* TestPhyLayer::createPathLossModel(ParameterMap& params){
	
	// init with default value
	double alpha;
	double carrierFrequency;
	bool useTorus = world->useTorus();
	const Coord& playgroundSize = *(world->getPgs());
	
	// get alpha-coefficient from config
	ParameterMap::iterator it = params.find("alpha");
	
	if ( it != params.end() ) // parameter alpha has been specified in config.xml
	{
		// set alpha 
		alpha = it->second.doubleValue();
		log("createPathLossModel(): alpha set from config.xml to " + toString(alpha));
		
		
		// check whether alpha is not smaller than specified in ConnectionManager
		if(cc->hasPar("alpha") && alpha < cc->par("alpha").doubleValue()) 
		{
	        // throw error
			opp_error("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
		}
		
		
	} else // alpha has not been specified in config.xml
	{
		
		if (cc->hasPar("alpha")) // parameter alpha has been specified in ConnectionManager
		{
			// set alpha according to ConnectionManager
			alpha = cc->par("alpha").doubleValue();
			log("createPathLossModel(): alpha set from ConnectionManager to " + toString(alpha));
		} else // alpha has not been specified in ConnectionManager
		
		{
			// set alpha to default value
			alpha = DEFAULT_ALPHA;
			log("createPathLossModel(): alpha set from default value to " + toString(alpha));
		}
	}
	
	
	
	// get carrierFrequency from config
	it = params.find("carrierFrequency");
	
	if ( it != params.end() ) // parameter carrierFrequency has been specified in config.xml
	{
		// set carrierFrequency 
		carrierFrequency = it->second.doubleValue();
		log("createPathLossModel(): carrierFrequency set from config.xml to " + toString(carrierFrequency));
		
		
		// check whether carrierFrequency is not smaller than specified in ConnectionManager
		if(cc->hasPar("carrierFrequency") && carrierFrequency < cc->par("carrierFrequency").doubleValue()) 
		{
			// throw error
			opp_error("TestPhyLayer::createPathLossModel(): carrierFrequency can't be smaller than specified in \
	               ConnectionManager. Please adjust your config.xml file accordingly");
		}
		
		
	} else // carrierFrequency has not been specified in config.xml
	{
		
		if (cc->hasPar("carrierFrequency")) // parameter carrierFrequency has been specified in ConnectionManager
		{
			// set carrierFrequency according to ConnectionManager
			carrierFrequency = cc->par("carrierFrequency").doubleValue();
			log("createPathLossModel(): carrierFrequency set from ConnectionManager to " + toString(carrierFrequency));
		} else // carrierFrequency has not been specified in ConnectionManager
		
		{
			// set carrierFrequency to default value
			carrierFrequency  = DEFAULT_CARRIER_FREQ;
			log("createPathLossModel(): carrierFrequency set from default value to " + toString(carrierFrequency));
		}
	}
	
	return new PathlossModel(alpha, carrierFrequency, &move, useTorus, playgroundSize);
	
}

/**
 * @brief Creates and initializes a RandomFreqTimeModel with the passed
 * parameter values.
 */
AnalogueModel* TestPhyLayer::createRandomFreqTimeModel(ParameterMap& params){
	
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
AnalogueModel* TestPhyLayer::createRandomFrequencyOnlyModel(ParameterMap& params){
	
	//get the "seed"-parameter from the config
	ParameterMap::iterator it = params.find("seed");
	
	//create AnalogueModel with default seed if no seed parameter was defined
	if(it == params.end()){
		return new RandomFrequencyOnlyModel();
	}
	
	long seed = it->second.longValue();
	return new RandomFrequencyOnlyModel(seed);
	
}

AnalogueModel* TestPhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params) {
	
	if(name == "PathlossModel")
		return createPathLossModel(params);
	else if(name == "RandomFreqTimeModel")
		return createRandomFreqTimeModel(params);
	else if(name == "RandomFrequencyOnlyModel")
		return createRandomFrequencyOnlyModel(params);
	
	//If we couldn't create the passed analogue model, call the method
	//of our base class.
	//Note: even if all models defined in the xml-config can be handled
	//by this class method, there will be at least the call to create
	//the RadioStateAnalogueModel which in almost every case has to be done
	//by the BasePhyLayer.
	return BasePhyLayer::getAnalogueModelFromName(name, params);
}
		
Decider* TestPhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {
		
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
