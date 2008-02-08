#include "BasePhyLayer.h"

#include "MacToPhyControlInfo.h"

//introduce BasePhyLayer as module to OMNet
Define_Module(BasePhyLayer);


//--Initialization----------------------------------

/**
 * Reads and returns the parameter with the passed named.
 * If the parameter couldn't be found the value of defaultValue
 * is returned.
 * 
 * parName 		- the name of the ned-parameter
 * defaultValue - the value to be returned if the parameter
 * 				  couldn't be found
 */
template<class T> T BasePhyLayer::readPar(const char* parName, const T defaultValue){
	if(hasPar(parName))
		return par(parName);
	else
		return defaultValue;
}

/** 
 * OMNeT++ initialization function. 
 * Read simple parameters.
 * Read and parse xml file for decider and anlogue models
 * configuration.
 */
void BasePhyLayer::initialize(int stage) {
	
	ChannelAccess::initialize(stage);
	
	if (stage == 0) {
		
		//TODO: stage 0 initialisation
		
		//get gate ids
		upperGateIn = findGate("uppergateIn");
        upperGateOut = findGate("uppergateOut");
        upperControlOut = findGate("upperControlOut");
        upperControlIn = findGate("upperControlIn");
		
		//read simple ned-parameters
		//	- initialize basic parameters
		
		usePropagationDelay = readPar("usePropagationDelay", false);				
		thermalNoise = readPar("thermalNoise", 0.0);
		sensitivity = readPar("sensitivity", 0.0);
		maxTXPower = readPar("maxTXPower", 1.0);		
		
		//	- initialize radio
		
		//		- switch times to TX
		simtime_t rxToTX = readPar("timeRXToTX", 0.0);
		simtime_t sleepToTX = readPar("timeSleepToTX", 0.0);
		
		//if no RX to TX defined asume same time as sleep to TX
		radio.setSwitchTime(Radio::RX, Radio::TX, readPar("timeRXToTX", sleepToTX)); 
		//if no sleep to TX defined asume same time as RX to TX
		radio.setSwitchTime(Radio::SLEEP, Radio::TX, readPar("timeSleepToTX", rxToTX));
		
		
		//		- switch times to RX
		simtime_t txToRX = readPar("timeTXToRX", 0.0);
		simtime_t sleepToRX = readPar("timeSleepToRX", 0.0);
		
		//if no TX to RX defined asume same time as sleep to RX
		radio.setSwitchTime(Radio::TX, Radio::RX, readPar("timeTXToRX", sleepToRX)); 
		//if no sleep to RX defined asume same time as TX to RX
		radio.setSwitchTime(Radio::SLEEP, Radio::RX, readPar("timeSleepToRX", txToRX));
		
		
		//		- switch times to sleep
		simtime_t txToSleep = readPar("timeTXToSleep", 0.0);
		simtime_t rxToSleep = readPar("timeRXToSleep", 0.0);
		
		//if no TX to sleep defined asume same time as RX to sleep
		radio.setSwitchTime(Radio::TX, Radio::SLEEP, readPar("timeTXToSleep", rxToSleep)); 
		//if no RX to sleep defined asume same time as TX to sleep
		radio.setSwitchTime(Radio::RX, Radio::SLEEP, readPar("timeRXToSleep", txToSleep));
		
		
		//read complex(xml) ned-parameters
		//	- analogue model parameters
		initializeAnalogueModels(readPar("analogueModels", (cXMLElement*)0));
		//	- decider parameters
		initializeDecider(readPar("decider", (cXMLElement*)0));
		
	} else if (stage == 1){
		
		//TODO: stage 1 initialisation
		
		//initialise timer messages
		radioSwitchingOverTimer = new cMessage(0, RADIO_SWITCHING_OVER);
		txOverTimer = new cMessage(0, TX_OVER);
	}
}

/**
 * Utility function. Reads the parameters of a XML element
 * and stores them in the passed ParameterMap reference.
 */
void BasePhyLayer::getParametersFromXML(cXMLElement* xmlData, ParameterMap& outputMap) {
	cXMLElementList parameters = xmlData->getElementsByTagName("Parameter");
	
	for(cXMLElementList::const_iterator it = parameters.begin();
		it != parameters.end(); it++) {
		
		const char* name = (*it)->getAttribute("name");
		const char* type = (*it)->getAttribute("type");
		const char* value = (*it)->getAttribute("value");
		if(name == 0 || type == 0 || value == 0) {
			ev << "Invalid parameter, could net find name, type or value." << endl;
			continue;
		}
		
		std::string sType = type; 	//needed for easier comparision
		std::string sValue = value;	//needed for easier comparision
		
		cPar param(name);
		
		//parse type of parameter and set value
		if (sType == "bool") {
			param.setBoolValue(sValue == "true" || sValue == "1");
			
		} else if (sType == "double") {
			param.setDoubleValue(strtod(value, 0));
			
		} else if (sType == "string") {
			param.setStringValue(value);
			
		} else if (sType == "long") {
			param.setLongValue(strtol(value, 0, 0));
			
		} else {
			ev << "Unknown parameter type: \"" << sType << "\"" << endl;
			continue;
		}
		
		//add parameter to output map
		outputMap[name] = param;
	}
}

//-----Decider initialization----------------------


/**
 * Initializes the Decider with the data from the
 * passed XML-config data.
 */
void BasePhyLayer::initializeDecider(cXMLElement* xmlConfig) {
	
	//TODO: implement
	
	decider = 0;
	
	if(xmlConfig == 0) {
		ev << "No decider configuration file specified." << endl;
		return;
	}
	
	cXMLElementList deciderList = xmlConfig->getElementsByTagName("Decider");
	
	if(deciderList.empty()) {
		ev << "No decider configuration found in configuration file." << endl;
		return;
	}
	
	if(deciderList.size() > 1) {
		ev << "More than one decider configuration found in configuration file." << endl;
		return;
	}
	
	cXMLElement* deciderData = deciderList.front();
	
	const char* name = deciderData->getAttribute("type");
	
	if(name == 0) {
		ev << "Could not read type of decider from configuration file." << endl;
		return;
	}
	
	ParameterMap params;
	getParametersFromXML(deciderData, params);
	
	decider = getDeciderFromName(name, params);	
	
	if(decider == 0) {
		ev << "Could not find a decider with the name \"" << name << "\"." << endl;
		return;
	}
}

/**
 * Returns an instance of the Decider with the specified 
 * name.
 * 
 * This method is used by the BasePhyLayer during 
 * initialisation to load the Decider which has been 
 * specified in the ned file.
 * 
 * This method has to be overriden if you want to be
 * able to load your own Decider.
 */
Decider* BasePhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {
	
	//TODO: implement
	return 0;
}


//-----AnalogueModels initialization----------------

/**
 * Initializes the AnalogueModels with the data from the
 * passed XML-config data.
 */
void BasePhyLayer::initializeAnalogueModels(cXMLElement* xmlConfig) {
	
	//TODO: implement
	
	
	if(xmlConfig == 0) {
		ev << "No analogue models configuration file specified." << endl;
		return;
	}
	
	cXMLElementList analogueModelList = xmlConfig->getElementsByTagName("AnalogueModel");
	
	if(analogueModelList.empty()) {
		ev << "No analogue models configuration found in configuration file." << endl;
		return;
	}
	
	// iterate over all AnalogueModel-entries, get a new AnalogueModel instance and add
	// it to analogueModels
	for(cXMLElementList::const_iterator it = analogueModelList.begin();
		it != analogueModelList.end(); it++) {
	
	
		cXMLElement* analogueModelData = *it;
			
		const char* name = analogueModelData->getAttribute("type");
		
		if(name == 0) {
			ev << "Could not read name of analogue model." << endl;
			continue;
		}
		
		ParameterMap params;
		getParametersFromXML(analogueModelData, params);
		
		AnalogueModel* newAnalogueModel = getAnalogueModelFromName(name, params);	
		
		if(newAnalogueModel == 0) {
			ev << "Could not find an analogue model with the name \"" << name << "\"." << endl;
			continue;
		} 
				
		// attach the new AnalogueModel to the AnalogueModelList
		analogueModels.push_back(newAnalogueModel);
		
	
	} // end iterator loop
	
	
}

/**
 * Returns an instance of the AnalogueModel with the 
 * specified name.
 * 
 * This method is used by the BasePhyLayer during 
 * initialisation to load the AnalogueModels which
 * has been specified in the ned file.
 * 
 * This method has to be overriden if you want to be
 * able to load your own AnalogueModels.
 */
AnalogueModel* BasePhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params) {
	
	//TODO: implement
	return 0;
}



//--Message handling--------------------------------------

/**
 * OMNeT++ handle message function.
 * Classify and forward message to subroutines.
 * - AirFrames from channel
 * - self scheduled AirFrames
 * - MacPackets from MAC layer
 * - ControllMesasges from MAC layer
 * - self messages like TX_OVER and RADIO_SWITCHED
 */
void BasePhyLayer::handleMessage(cMessage* msg) {
	//TODO: check implementation
	
	//self messages	
	if(msg->isSelfMessage()) {
		handleSelfMessage(msg);
	
	//MacPkts <- MacToPhyControlInfo
	} else if(msg->arrivalGateId() == upperGateIn) {
		handleUpperMessage(msg);
		
	//controlmessages 
	} else if(msg->arrivalGateId() == upperControlIn) {
		handleUpperControlMessage(msg);
		
	//AirFrames
	} else if(msg->kind() == AIR_FRAME){
		handleAirFrame(msg);
		
	//unknown message
	} else {
		ev << "Unknown message received." << endl;
		delete msg;
	}
}

/**
 * Handles messages received from the channel (propably AirFrames).
 */
void BasePhyLayer::handleAirFrame(cMessage* msg) {
	
	//TODO: check implementation
	AirFrame* frame = static_cast<AirFrame*>(msg);
	
	switch(frame->getState()) {
	case FIRST_RECEIVE:
		handleAirFrameFirstReceive(frame);
		break;
		
	case START_RECEIVE:
		handleAirFrameStartReceive(frame);
		break;
		
	case RECEIVING: {
		handleAirFrameReceiving(frame);
		break;
	}
	case END_RECEIVE:
		handleAirFrameEndReceive(frame);
		break;
		
	default:
		ev << "Unknown AirFrame state: " << frame->getState() << endl;
		//TODO: this is a serious error -> end simulation
		break;
	}
}

/**
 * Handles incoming AirFrames with the state FIRST_RECEIVE.
 */
void BasePhyLayer::handleAirFrameFirstReceive(AirFrame* frame) {
	frame->setState(START_RECEIVE);
			
	if(usePropagationDelay) {
		//calculate delayed signal start
		simtime_t delayedStart = simTime() + calculatePropagationDelay(frame);
		//update signal start in signal
		frame->getSignal().setSignalStart(delayedStart);
		//schedule delayed AirFrame
		sendSelfMessage(frame, delayedStart);
		
	} else {
		handleAirFrameStartReceive(frame);
	}
}

/**
 * Handles incoming AirFrames with the state START_RECEIVE.
 */
void BasePhyLayer::handleAirFrameStartReceive(AirFrame* frame) {
	channelInfo.addAirFrame(frame, simTime());
	frame->setState(RECEIVING);
	
	//pass the AirFrame the first time to the Decider
	handleAirFrameReceiving(frame);
}

/**
 * Handles incoming AirFrames with the state RECEIVING.
 */
void BasePhyLayer::handleAirFrameReceiving(AirFrame* frame) {
	
	//TODO: check implementation
	Signal* signal = frame->getSignalPointer();
	simtime_t nextHandleTime = decider->processSignal(signal);
	
	simtime_t signalEndTime = signal->getSignalStart() + frame->getDuration();
	
	//check if this is the end of the receiving process
	if(simTime() >= signalEndTime) {
		frame->setState(END_RECEIVE);
		handleAirFrameEndReceive(frame);
		return;
	}
	
	//smaller zero means don't give it to me again
	if(nextHandleTime < 0) { 
		nextHandleTime = signalEndTime;
		frame->setState(END_RECEIVE);
		
	//invalid point in time
	} else if(nextHandleTime < simTime() || nextHandleTime > signalEndTime) {
		
		throw new cRuntimeError("Invalid next handle time returned by Decider. Expected a value between current simulation time (%.2f) and end of signal (%.2f) but got %.2f",
								simTime(), signalEndTime, nextHandleTime);
	}
	
	sendSelfMessage(frame, nextHandleTime);
}

/**
 * Handles incoming AirFrames with the state END_RECEIVE.
 */
void BasePhyLayer::handleAirFrameEndReceive(AirFrame* frame) {
	channelInfo.removeAirFrame(frame);
	//TODO: any other things to do?
}

/**
 * Handles messages received from the upper layer through the
 * data gate.
 */
void BasePhyLayer::handleUpperMessage(cMessage* msg){
	
	//TODO: check and test implementation
	
	// check if Radio is in TX state
	if (radio.getCurrentState() != Radio::TX)
	{
		ev << "Error: message for sending received, but radio not in state TX" << endl;
		// TODO: what to do here when this error occured?
	}
	
	// check if not already sending
	if(txOverTimer->isScheduled())
	{
		ev << "Error: message for sending received, but radio already sending" << endl;
		// TODO: what to do here when this error occured?
	}
	
	
	// build the AirFrame to send
	AirFrame* frame = encapsMsg(msg);
	
	// make sure there is no self message of kind TX_OVER scheduled 
	// and schedule the actual one
	assert (!txOverTimer->isScheduled());
	sendSelfMessage(txOverTimer, simTime() + frame->getDuration());
	
		
	sendMessageDown(frame);
}

/**
 * This function encapsulates messages from the upper layer into an
 * AirFrame and sets all necessary attributes.
 * 
 */
AirFrame *BasePhyLayer::encapsMsg(cMessage *msg)
{
 //TODO: move method to the right position in the source code
 //TODO: check and test implementation
	
	// the cMessage passed must be a MacPacket... but no cast needed here
	// MacPkt* pkt = static_cast<MacPkt*>(msg);
	
	// ...and must always have a ControlInfo attached (contains Signal)
	cPolymorphic* ctrlInfo = msg->removeControlInfo();
	assert(ctrlInfo);
	
	// TODO: delete ControlInfo object in the end
	MacToPhyControlInfo* macToPhyCI = static_cast<MacToPhyControlInfo*>(ctrlInfo);
	
	Signal* s = macToPhyCI->retrieveSignal();
	
	// put host move pattern to Signal
	s->setMove(move);
	
	// create the new AirFrame
	AirFrame* frame = new AirFrame(0, AIR_FRAME);
	
	// set the members
	frame->setSignal(s);
	frame->setDuration(s->getSignalLength());
	
	// TODO: wie und woher bekommt man die Id, Id richtig setzen
	frame->setId(0);
	frame->encapsulate(msg);
	
	return frame;
}

/**
 * Handles reception of a ChannelSenseRequest by forwarding it 
 * to the decider and scheduling it to the point in time 
 * returned by the decider.
 */
void BasePhyLayer::handleChannelSenseRequest(cMessage* msg) {
	ChannelSenseRequest* senseReq = static_cast<ChannelSenseRequest*>(msg);
			
	simtime_t nextHandleTime = decider->handleChannelSenseRequest(senseReq);
	
	if(nextHandleTime >= simTime()) { //schedule request for next handling
		sendSelfMessage(msg, nextHandleTime);
		
	} else if(nextHandleTime >= 0.0){
		throw new cRuntimeError("Next handle time of ChannelSenseRequest returned by the Decider is small er then current simulation time: %.2f",
								nextHandleTime);
	}
}

/**
 * Handles messages received from the upper layer through the
 * control gate.
 */
void BasePhyLayer::handleUpperControlMessage(cMessage* msg){
	
	//TODO: check implementation
	switch(msg->kind()) {
	case CHANNEL_SENSE_REQUEST:
		handleChannelSenseRequest(msg);
	}
	
}

/**
 * Handles self scheduled messages.
 */
void BasePhyLayer::handleSelfMessage(cMessage* msg) {	
	
	switch(msg->kind()) {
	//transmission over
	case TX_OVER:
		//TODO: check implementation
		assert(msg == txOverTimer);
		sendControlMsg(new cMessage(0, TX_OVER));
		break;
		
	//radio switch over
	case RADIO_SWITCHING_OVER:
		//TODO: check implementation
		assert(msg == radioSwitchingOverTimer);
		radio.endSwitch();
		sendControlMsg(new cMessage(0, RADIO_SWITCHING_OVER));
		break;		
		
	//AirFrame
	case AIR_FRAME:
		handleAirFrame(msg);
		break;
		
	//ChannelSenseRequest
	case CHANNEL_SENSE_REQUEST:
		handleChannelSenseRequest(msg);
		break;
		
	default:
		break;
	}
}

//--Send messages------------------------------

/**
 * Sends the passed control message to the upper layer.
 */
void BasePhyLayer::sendControlMessageUp(cMessage* msg) {
	
	send(msg, upperControlOut);
}

/**
 * Sends the passed MacPkt to the upper layer.
 */
void BasePhyLayer::sendMacPktUp(MacPkt* pkt) {
	
	send(pkt, upperGateOut);
}

/**
 * Sends the passed AirFrame to the channel
 */
void BasePhyLayer::sendMessageDown(AirFrame* msg) {
	
	//TODO: check if delay is needed
	sendToChannel(msg, 0);
}

/**
 * Schedule self message to passed point in time.
 */
void BasePhyLayer::sendSelfMessage(cMessage* msg, simtime_t time) {
	
	scheduleAt(time, msg);
}


//--Destruction--------------------------------
/**
 * Free the pointer to the decider and the AnalogueModels.
 */
BasePhyLayer::~BasePhyLayer() {
	//free timer messages
	if(txOverTimer) {
		delete txOverTimer;
	}
	if(radioSwitchingOverTimer) {
		delete radioSwitchingOverTimer;
	}
	
	//free Decider
	if(decider != 0) {
		delete decider;
	}
	
	//free AnalogueModels
	for(AnalogueModelList::iterator it = analogueModels.begin();
		it != analogueModels.end(); it++) {
		
		AnalogueModel* tmp = *it;
		if(tmp != 0) {
			delete tmp;
		}
	}
}

//--calculations------------------------------------------

/**
 * Calculates the propagation delay for the passed AirFrame.
 */
simtime_t BasePhyLayer::calculatePropagationDelay(AirFrame* frame) {
	
	//TODO: implement
	return 0;
}


//--MacToPhyInterface implementation-----------------------

/**
 * Returns the current state the radio is in. See RadioState
 * for possible values.
 * 
 * This method is mainly used by the mac layer.
 */
Radio::RadioState BasePhyLayer::getRadioState() {

	return radio.getCurrentState();
}

/**
 * Tells the BasePhyLayer to switch to the specified
 * radio state. The switching process can take some time
 * depending on the specified switching times in the
 * ned file.
 */
simtime_t BasePhyLayer::setRadioState(Radio::RadioState rs) {
	
	//TODO: check implementation
	simtime_t switchTime = radio.switchTo(rs);
	
	if(switchTime < 0) //invalid switch time, we are propably already switching 
		return switchTime;
	
	sendSelfMessage(radioSwitchingOverTimer, switchTime);
	
	return switchTime;
}

/**
 * Returns the current state of the channel. See ChannelState
 * for details.
 */
ChannelState BasePhyLayer::getChannelState() {
	return decider->getChannelState();
}

/**
 * Service method for the mac layer which creates and initializes
 * an appropriate Signal with the specified values.
 * 
 * Used by the mac layer before sending a mac packet to the phy layer 
 * to create an initial signal with some predefined values.
 * 
 * TODO: write more detailed axplanation as soon as modelation of
 * 		 Signal is final.
 */
Signal BasePhyLayer::createSignal(	double txPower, 
									double headerBitrate, 
									double payloadBitrate, 
									simtime_t duration) {
	
	//TODO: implement
	Signal s;
	return s;
	
}


//--DeciderToPhyInterface implementation------------

/**
 * @brief Fills the passed AirFrameVector with all AirFrames that intersect 
 * with the time interval [from, to]
 */
void BasePhyLayer::getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out) {
	
	channelInfo.getAirFrames(from, to, out);
}

/** 
 * @brief Called by the Decider to send a control message to the MACLayer
 * 
 * This function can be used to answer a ChannelSenseRequest to the MACLayer
 * 
 */
void BasePhyLayer::sendControlMsg(cMessage* msg) {
	
	sendControlMessageUp(msg);
}

/** 
 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
 * 
 * When a packet is completely received and not noise, the Decider
 * call this function to send the packet together with
 * the corresponding DeciderResult up to MACLayer
 * 
 */
void BasePhyLayer::sendUp(AirFrame* packet, DeciderResult result) {
	
	//TODO: implement
	
}

/**
 * @brief Returns the current simulation time
 * 
 */
simtime_t BasePhyLayer::getSimTime() {
	
	return simTime();
}
