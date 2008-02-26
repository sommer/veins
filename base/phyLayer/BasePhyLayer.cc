#include "BasePhyLayer.h"
#include "BaseWorldUtility.h"

#include "MacToPhyControlInfo.h"
#include "PhyToMacControlInfo.h"

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
		
		//TODO: anything else to initialize?
		
		//get gate ids
		upperGateIn = findGate("upperGateIn");
        upperGateOut = findGate("upperGateOut");
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
		//TOTEST: check initialisation of timers
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
	
	//TODO: add default decider(s) here
	return 0;
}


//-----AnalogueModels initialization----------------

/**
 * Initializes the AnalogueModels with the data from the
 * passed XML-config data.
 */
void BasePhyLayer::initializeAnalogueModels(cXMLElement* xmlConfig) {
	
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
	
	//TODO: add default analogue mdoels here
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
	
	//self messages	
	//TOTEST: generell receiving and sending of self messages
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
	
	AirFrame* frame = static_cast<AirFrame*>(msg);
	
	//TOTEST: check states of recceived AirFrames (not just the kind)
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
		//TOTEST: check if correctly delayed (signal and scheduled message)
	} else {
		handleAirFrameStartReceive(frame);
	}
}

/**
 * Handles incoming AirFrames with the state START_RECEIVE.
 */
void BasePhyLayer::handleAirFrameStartReceive(AirFrame* frame) {
	//TOTEST: check channelinfo for new airframe
	channelInfo.addAirFrame(frame, simTime());
	
	filterSignal(frame->getSignal());
	
	if(decider) {
		frame->setState(RECEIVING);
		
		//pass the AirFrame the first time to the Decider
		//TOTEST: check first arrival at decider
		handleAirFrameReceiving(frame);
		
	//if no decider is defined we will schedule the message directly to its end
	} else {
		Signal& signal = frame->getSignal();
		
		simtime_t signalEndTime = signal.getSignalStart() + frame->getDuration();
		frame->setState(END_RECEIVE);
		
		sendSelfMessage(frame, signalEndTime);
	}
}

/**
 * Handles incoming AirFrames with the state RECEIVING.
 */
void BasePhyLayer::handleAirFrameReceiving(AirFrame* frame) {
	
	Signal& signal = frame->getSignal();
	//TOTEST: check arrival at decider
	simtime_t nextHandleTime = decider->processSignal(frame);
	
	simtime_t signalEndTime = signal.getSignalStart() + frame->getDuration();
	
	//TOTEST: check different possible return values of decider (x<0, x<simtime, x>end, x=end)
	
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
	//TOTEST: check removal of airframe from channelinfo
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
        delete msg;
        msg = 0;
		opp_error("Error: message for sending received, but radio not in state TX");
		// TODO: opp_error correct handling?
	}
	
	// check if not already sending
	if(txOverTimer->isScheduled())
	{
        delete msg;
        msg = 0;
		opp_error("Error: message for sending received, but radio already sending");
		// TODO: opp_error correct handling?
	}
	
	//TOTEST: check for correct encapsulation (data and encapsualted message)
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
 * The new AirFrame instance becomes owner of the Signal and MacPacket
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
	
	MacToPhyControlInfo* macToPhyCI = static_cast<MacToPhyControlInfo*>(ctrlInfo);
	
	// Retrieve the pointer to the Signal-instance from the ControlInfo-instance.
	// We are now the new owner of this instance.
	Signal* s = macToPhyCI->retrieveSignal();
	
	
	// delete the Control info
	delete macToPhyCI;
	macToPhyCI = 0;
	ctrlInfo = 0;
	
	// make sure we really obtained a pointer to an instance
	// TODO: figure out what to do if we actually have NO Signal here
	assert(s);
	
	// put host move pattern to Signal
	s->setMove(move);
	
	// create the new AirFrame
	AirFrame* frame = new AirFrame(0, AIR_FRAME);
	
	// set the members
	frame->setDuration(s->getSignalLength());
	// copy the signal into the AirFrame
	frame->setSignal(*s);
	
	// pointer and Signal not needed anymore
	delete s;
	s = 0;
	
	// TODO: where to get a unique id from?, set id properly
	//TOTEST: check if id is really unique
	frame->setId(0);
	frame->encapsulate(msg);
	
	// --- from here on, the AirFrame is the owner of the MacPacket ---
	msg = 0;
	
	
	return frame;
}

/**
 * Handles reception of a ChannelSenseRequest by forwarding it 
 * to the decider and scheduling it to the point in time 
 * returned by the decider.
 */
void BasePhyLayer::handleChannelSenseRequest(cMessage* msg) {
	//TOTEST: test ChannelSenseRequest handling
	ChannelSenseRequest* senseReq = static_cast<ChannelSenseRequest*>(msg);
			
	simtime_t nextHandleTime = decider->handleChannelSenseRequest(senseReq);
	
	//TOTEST: check returned times x<0, x<simtime
	if(nextHandleTime >= simTime()) { //schedule request for next handling
		sendSelfMessage(msg, nextHandleTime);
		
	} else if(nextHandleTime >= 0.0){
		opp_error("Next handle time of ChannelSenseRequest returned by the Decider is small er then current simulation time: %.2f",
								nextHandleTime);
	}
}

/**
 * Handles messages received from the upper layer through the
 * control gate.
 */
void BasePhyLayer::handleUpperControlMessage(cMessage* msg){
	
	//TODO: we should propably process control messages independent from their 
	//		kind because they are just passed to the decider anyway
	switch(msg->kind()) {
	case CHANNEL_SENSE_REQUEST:
		handleChannelSenseRequest(msg);
		break;
	default:
		ev << "Received unknown control message from upper layer!" << endl;
		break;
	}	
}

/**
 * Handles self scheduled messages.
 */
void BasePhyLayer::handleSelfMessage(cMessage* msg) {	
	
	switch(msg->kind()) {
	//transmission over
	case TX_OVER:
		assert(msg == txOverTimer);
		sendControlMsg(new cMessage(0, TX_OVER));
		break;
		
	//radio switch over
	case RADIO_SWITCHING_OVER:
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
	//TOTEST: send a test control message up
	send(msg, upperControlOut);
}

/**
 * Sends the passed MacPkt to the upper layer.
 */
void BasePhyLayer::sendMacPktUp(cMessage* pkt) {
	//TOTEST: send a test MacPkt up
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
	//TODO: maybe delete this method because it doesn't makes much sense,
	//		or change it to "scheduleIn(msg, timeDelta)" which schedules
	//		a message to +timeDelta from current time
	scheduleAt(time, msg);
}


/**
 * Filters the passed Signal to every registered AnalogueModel.
 */ 
void BasePhyLayer::filterSignal(Signal& s) {
	for(AnalogueModelList::const_iterator it = analogueModels.begin();
		it != analogueModels.end(); it++) {
		
		AnalogueModel* tmp = *it;
		tmp->filterSignal(s);
	}
}

//--Destruction--------------------------------
/**
 * Free the pointer to the decider and the AnalogueModels.
 */
BasePhyLayer::~BasePhyLayer() {
	//free timer messages
	//TOPROFILE: check BasePhy destruction for memory leaks
	if(txOverTimer) {
        if(txOverTimer->isScheduled())
            cancelEvent(txOverTimer);
		delete txOverTimer;
	}
	if(radioSwitchingOverTimer) {
        if(radioSwitchingOverTimer->isScheduled())
            cancelEvent(radioSwitchingOverTimer);

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
	
	//TODO: implement correct calculation
	const Signal& s = frame->getSignal();
	Move senderPos = s.getMove();
	
	//very naiv and very wrong calculation, but for now sufficient
	double distance = senderPos.startPos.distance(move.startPos);
	
	double delay = distance / BaseWorldUtility::speedOfLight;
	return delay;
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
	
	//TODO: what to do if we are currently transmitting a signal?
	//TOTEST: check radio switching at another time then 0.0
	simtime_t switchTime = radio.switchTo(rs);
	
	if(switchTime < 0) //invalid switch time, we are propably already switching 
		return switchTime;
	
	sendSelfMessage(radioSwitchingOverTimer, simTime() + switchTime);
	
	return switchTime;
}

/**
 * Returns the current state of the channel. See ChannelState
 * for details.
 */
ChannelState BasePhyLayer::getChannelState() {
	//TOTEST: check correct passing of channelstate
	return decider->getChannelState();
}

//--DeciderToPhyInterface implementation------------

/**
 * @brief Fills the passed AirFrameVector with all AirFrames that intersect 
 * with the time interval [from, to]
 */
void BasePhyLayer::getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out) {
	//TOTEST: check correct passing of the airframevector
	channelInfo.getAirFrames(from, to, out);
}

/** 
 * @brief Called by the Decider to send a control message to the MACLayer
 * 
 * This function can be used to answer a ChannelSenseRequest to the MACLayer
 */
void BasePhyLayer::sendControlMsg(cMessage* msg) {
	//TOTEST: check correct passing of controlmessage to mac
	sendControlMessageUp(msg);
}

/** 
 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
 * 
 * When a packet is completely received and not noise, the Decider
 * calls this function to send the packet together with
 * the corresponding DeciderResult up to the MACLayer
 */
void BasePhyLayer::sendUp(AirFrame* frame, DeciderResult result) {
	
	//TODO: implement
	//TOTEST: check correct creation (decapsulation) of MacPkt
	
	cMessage* packet = frame->decapsulate();
	
	assert(packet);
	
	PhyToMacControlInfo* ctrlInfo = new PhyToMacControlInfo(result);
	
	packet->setControlInfo(ctrlInfo);
	
	sendMacPktUp(packet);	
}

/**
 * @brief Returns the current simulation time
 */
simtime_t BasePhyLayer::getSimTime() {
	
	return simTime();
}
