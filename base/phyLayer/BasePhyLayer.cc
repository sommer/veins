#include "BasePhyLayer.h"
#include "BaseWorldUtility.h"

#include "MacToPhyControlInfo.h"
#include "PhyToMacControlInfo.h"



//introduce BasePhyLayer as module to OMNet
Define_Module(BasePhyLayer);


//--Initialization----------------------------------

template<class T> T BasePhyLayer::readPar(const char* parName, const T defaultValue){
	if(hasPar(parName))
		return par(parName);
	else
		return defaultValue;
}

void BasePhyLayer::initialize(int stage) {

	ChannelAccess::initialize(stage);

	if (stage == 0) {
		// if using sendDirect, make sure that messages arrive without delay
		gate("radioIn")->setDeliverOnReceptionStart(true);

		//get gate ids
		upperGateIn = findGate("upperGateIn");
        upperGateOut = findGate("upperGateOut");
        upperControlOut = findGate("upperControlOut");
        upperControlIn = findGate("upperControlIn");

		//read simple ned-parameters
		//	- initialize basic parameters
        if(readPar("useThermalNoise", false)) {
			double thermalNoiseVal = FWMath::dBm2mW(readPar("thermalNoise", -999.0));
			thermalNoise = new ConstantSimpleConstMapping(DimensionSet(Dimension::time),
														  thermalNoiseVal);
		} else {
			thermalNoise = 0;
		}
		sensitivity = readPar("sensitivity", 0.0);
		sensitivity = FWMath::dBm2mW(sensitivity);
		maxTXPower = readPar("maxTXPower", 1.0);

		//	- initialize radio
		int initialRadioState = readPar("initalRadioState", (int) Radio::RX);
		double radioMinAtt = readPar("radioMinAtt", 1.0);
		double radioMaxAtt = readPar("radioMaxAtt", 0.0);

		radio = new Radio(initialRadioState, radioMinAtt, radioMaxAtt);


		//	- switch times to TX
		simtime_t rxToTX = readPar("timeRXToTX", 0.0);
		simtime_t sleepToTX = readPar("timeSleepToTX", 0.0);

		//if no RX to TX defined asume same time as sleep to TX
		radio->setSwitchTime(Radio::RX, Radio::TX, readPar("timeRXToTX", sleepToTX));
		//if no sleep to TX defined asume same time as RX to TX
		radio->setSwitchTime(Radio::SLEEP, Radio::TX, readPar("timeSleepToTX", rxToTX));


		//		- switch times to RX
		simtime_t txToRX = readPar("timeTXToRX", 0.0);
		simtime_t sleepToRX = readPar("timeSleepToRX", 0.0);

		//if no TX to RX defined asume same time as sleep to RX
		radio->setSwitchTime(Radio::TX, Radio::RX, readPar("timeTXToRX", sleepToRX));
		//if no sleep to RX defined asume same time as TX to RX
		radio->setSwitchTime(Radio::SLEEP, Radio::RX, readPar("timeSleepToRX", txToRX));


		//		- switch times to sleep
		simtime_t txToSleep = readPar("timeTXToSleep", 0.0);
		simtime_t rxToSleep = readPar("timeRXToSleep", 0.0);

		//if no TX to sleep defined asume same time as RX to sleep
		radio->setSwitchTime(Radio::TX, Radio::SLEEP, readPar("timeTXToSleep", rxToSleep));
		//if no RX to sleep defined asume same time as TX to sleep
		radio->setSwitchTime(Radio::RX, Radio::SLEEP, readPar("timeRXToSleep", txToSleep));



		// get pointer to the world module
		world = FindModule<BaseWorldUtility*>::findGlobalModule();
        if (world == NULL)
            throw cRuntimeError("Could not find BaseWorldUtility module");

	} else if (stage == 1){
		//read complex(xml) ned-parameters
		//	- analogue model parameters
		initializeAnalogueModels(readPar("analogueModels", (cXMLElement*)0));
		//	- decider parameters
		initializeDecider(readPar("decider", (cXMLElement*)0));

		//initialise timer messages
		radioSwitchingOverTimer = new cMessage(0, RADIO_SWITCHING_OVER);
		txOverTimer = new cMessage(0, TX_OVER);

	}
}

void BasePhyLayer::getParametersFromXML(cXMLElement* xmlData, ParameterMap& outputMap) {
	cXMLElementList parameters = xmlData->getElementsByTagName("Parameter");

	for(cXMLElementList::const_iterator it = parameters.begin();
		it != parameters.end(); it++) {

		const char* name = (*it)->getAttribute("name");
		const char* type = (*it)->getAttribute("type");
		const char* value = (*it)->getAttribute("value");
		if(name == 0 || type == 0 || value == 0) {
			ev << "Invalid parameter, could not find name, type or value." << endl;
			continue;
		}

		std::string sType = type; 	//needed for easier comparision
		std::string sValue = value;	//needed for easier comparision

		cMsgPar param(name);

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
		opp_error("Could not find a decider with the name \"%s\".", name);
		return;
	}

	coreEV << "Decider \"" << name << "\" loaded." << endl;
}

Decider* BasePhyLayer::getDeciderFromName(std::string name, ParameterMap& params) {

	return 0;
}


//-----AnalogueModels initialization----------------

void BasePhyLayer::initializeAnalogueModels(cXMLElement* xmlConfig) {

	/*
	* first of all, attach the AnalogueModel that represents the RadioState
	* to the AnalogueModelList as first element.
	*/
	std::string s("RadioStateAnalogueModel");
	ParameterMap p;

	AnalogueModel* newAnalogueModel = getAnalogueModelFromName(s, p);

	if(newAnalogueModel == 0)
	{
		ev << "Could not find an analogue model with the name \"" << s << "\"." << endl;
	}
	analogueModels.push_back(newAnalogueModel);


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

		coreEV << "AnalogueModel \"" << name << "\" loaded." << endl;

	} // end iterator loop


}

AnalogueModel* BasePhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params) {

	// add default analogue models here

	// case "RSAM", pointer is valid as long as the radio exists
	if (name == "RadioStateAnalogueModel")
	{
		return radio->getAnalogueModel();
	}

	return 0;
}



//--Message handling--------------------------------------

void BasePhyLayer::handleMessage(cMessage* msg) {

	//self messages
	if(msg->isSelfMessage()) {
		handleSelfMessage(msg);

	//MacPkts <- MacToPhyControlInfo
	} else if(msg->getArrivalGateId() == upperGateIn) {
		handleUpperMessage(msg);

	//controlmessages
	} else if(msg->getArrivalGateId() == upperControlIn) {
		handleUpperControlMessage(msg);

	//AirFrames
	} else if(msg->getKind() == AIR_FRAME){
		handleAirFrame(msg);

	//unknown message
	} else {
		ev << "Unknown message received." << endl;
		delete msg;
	}
}

void BasePhyLayer::handleAirFrame(cMessage* msg) {

	AirFrame* frame = static_cast<AirFrame*>(msg);

	switch(frame->getState()) {
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
		opp_error( "Unknown AirFrame state: %s", frame->getState());
		break;
	}
}

void BasePhyLayer::handleAirFrameStartReceive(AirFrame* frame) {
	coreEV << "Received new AirFrame with ID " << frame->getId() << " from channel" << endl;
	channelInfo.addAirFrame(frame, simTime());

	//TODO: consider updating signal start when propagation delay is used
	if(usePropagationDelay) {
		Signal& s = frame->getSignal();
		simtime_t delay = simTime() - s.getSignalStart();
		s.setPropagationDelay(delay);
	}
	assert(frame->getSignal().getSignalStart() == simTime());

	filterSignal(frame->getSignal());

	if(decider) {
		frame->setState(RECEIVING);

		//pass the AirFrame the first time to the Decider
		handleAirFrameReceiving(frame);

	//if no decider is defined we will schedule the message directly to its end
	} else {
		Signal& signal = frame->getSignal();

		simtime_t signalEndTime = signal.getSignalStart() + frame->getDuration();
		frame->setState(END_RECEIVE);

		sendSelfMessage(frame, signalEndTime);
	}
}

void BasePhyLayer::handleAirFrameReceiving(AirFrame* frame) {

	Signal& signal = frame->getSignal();
	simtime_t nextHandleTime = decider->processSignal(frame);

	assert(signal.getSignalLength() == frame->getDuration());
	simtime_t signalEndTime = signal.getSignalStart() + frame->getDuration();

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
		opp_error("Invalid next handle time returned by Decider. Expected a value between current simulation time (%.2f) and end of signal (%.2f) but got %.2f",
								SIMTIME_DBL(simTime()), SIMTIME_DBL(signalEndTime), SIMTIME_DBL(nextHandleTime));
	}

	coreEV << "Handed AirFrame with ID " << frame->getId() << " to Decider. Next handling in " << nextHandleTime - simTime() << "s." << endl;

	sendSelfMessage(frame, nextHandleTime);
}

void BasePhyLayer::handleAirFrameEndReceive(AirFrame* frame) {
	coreEV << "End of Airframe with ID " << frame->getId() << "." << endl;

	channelInfo.removeAirFrame(frame);

	/* clean information in the radio until earliest time-point
	*  of information in the ChannelInfo,
	*  since this time-point might have changed due to removal of
	*  the AirFrame
	*/
	radio->cleanAnalogueModelUntil(channelInfo.getEarliestInfoPoint());
}

void BasePhyLayer::handleUpperMessage(cMessage* msg){

	// check if Radio is in TX state
	if (radio->getCurrentState() != Radio::TX)
	{
        delete msg;
        msg = 0;
		opp_error("Error: message for sending received, but radio not in state TX");
	}

	// check if not already sending
	if(txOverTimer->isScheduled())
	{
        delete msg;
        msg = 0;
		opp_error("Error: message for sending received, but radio already sending");
	}

	// build the AirFrame to send
	assert(dynamic_cast<cPacket*>(msg) != 0);

	AirFrame* frame = encapsMsg(static_cast<cPacket*>(msg));

	// make sure there is no self message of kind TX_OVER scheduled
	// and schedule the actual one
	assert (!txOverTimer->isScheduled());
	sendSelfMessage(txOverTimer, simTime() + frame->getDuration());

	sendMessageDown(frame);
}

AirFrame *BasePhyLayer::encapsMsg(cPacket *macPkt)
{
	// the cMessage passed must be a MacPacket... but no cast needed here
	// MacPkt* pkt = static_cast<MacPkt*>(msg);

	// ...and must always have a ControlInfo attached (contains Signal)
	cObject* ctrlInfo = macPkt->removeControlInfo();
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
	assert(s);

	// put host move pattern to Signal
	s->setMove(move);

	// create the new AirFrame
	AirFrame* frame = new AirFrame(0, AIR_FRAME);

	// set the members
	assert(s->getSignalLength() > 0);
	frame->setDuration(s->getSignalLength());
	// copy the signal into the AirFrame
	frame->setSignal(*s);

	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	frame->setId(world->getUniqueAirFrameId());
	frame->encapsulate(macPkt);

	// --- from here on, the AirFrame is the owner of the MacPacket ---
	macPkt = 0;


	return frame;
}

void BasePhyLayer::handleChannelSenseRequest(cMessage* msg) {
	ChannelSenseRequest* senseReq = static_cast<ChannelSenseRequest*>(msg);

	simtime_t nextHandleTime = decider->handleChannelSenseRequest(senseReq);

	if(nextHandleTime >= simTime()) { //schedule request for next handling
		sendSelfMessage(msg, nextHandleTime);

	} else if(nextHandleTime >= 0.0){
		opp_error("Next handle time of ChannelSenseRequest returned by the Decider is smaller then current simulation time: %.2f",
				SIMTIME_DBL(nextHandleTime));
	}

	// else, i.e. nextHandleTime < 0.0, the Decider doesn't want to handle
	// the request again
}

void BasePhyLayer::handleUpperControlMessage(cMessage* msg){

	switch(msg->getKind()) {
	case CHANNEL_SENSE_REQUEST:
		handleChannelSenseRequest(msg);
		break;
	default:
		ev << "Received unknown control message from upper layer!" << endl;
		break;
	}
}

void BasePhyLayer::handleSelfMessage(cMessage* msg) {

	switch(msg->getKind()) {
	//transmission over
	case TX_OVER:
		assert(msg == txOverTimer);
		sendControlMsg(new cMessage(0, TX_OVER));
		break;

	//radio switch over
	case RADIO_SWITCHING_OVER:
		assert(msg == radioSwitchingOverTimer);
		finishRadioSwitching();
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

void BasePhyLayer::sendControlMessageUp(cMessage* msg) {
	send(msg, upperControlOut);
}

void BasePhyLayer::sendMacPktUp(cMessage* pkt) {
	send(pkt, upperGateOut);
}

void BasePhyLayer::sendMessageDown(AirFrame* msg) {

	sendToChannel(msg);
}

void BasePhyLayer::sendSelfMessage(cMessage* msg, simtime_t time) {
	//TODO: maybe delete this method because it doesn't makes much sense,
	//		or change it to "scheduleIn(msg, timeDelta)" which schedules
	//		a message to +timeDelta from current time
	scheduleAt(time, msg);
}


void BasePhyLayer::filterSignal(Signal& s) {
	for(AnalogueModelList::const_iterator it = analogueModels.begin();
		it != analogueModels.end(); it++) {

		AnalogueModel* tmp = *it;
		tmp->filterSignal(s);
	}
}

//--Destruction--------------------------------

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

	//free thermal noise mapping
	if(thermalNoise) {
		delete thermalNoise;
	}

	//free Decider
	if(decider != 0) {
		delete decider;
	}

	/*
	 * get a pointer to the radios RSAM again to avoid deleting it,
	 * it is not created by calling new (BasePhyLayer is not the owner)!
	 */
	AnalogueModel* rsamPointer = radio->getAnalogueModel();

	//free AnalogueModels
	for(AnalogueModelList::iterator it = analogueModels.begin();
		it != analogueModels.end(); it++) {

		AnalogueModel* tmp = *it;

		// do not delete the RSAM, it's not allocated by new!
		if (tmp == rsamPointer)
		{
			rsamPointer = 0;
			continue;
		}

		if(tmp != 0) {
			delete tmp;
		}
	}


	// free radio
	if(radio != 0)
	{
		delete radio;
	}
}

//--MacToPhyInterface implementation-----------------------

int BasePhyLayer::getRadioState() {
	Enter_Method_Silent();
	return radio->getCurrentState();
}

void BasePhyLayer::finishRadioSwitching()
{
	radio->endSwitch(simTime());
	sendControlMsg(new cMessage(0, RADIO_SWITCHING_OVER));
}

simtime_t BasePhyLayer::setRadioState(int rs) {
	Enter_Method_Silent();

	//TODO: check if we are currently transmitting, if so throw a warning or error
	simtime_t switchTime = radio->switchTo(rs, simTime());

	//invalid switch time, we are probably already switching
	if(switchTime < 0)
		return switchTime;

	// if switching is done in exactly zero-time no extra self-message is scheduled
	if (switchTime == 0.0)
	{
		// TODO: in case of zero-time-switch, send no control-message to mac!
		// maybe call a method finishRadioSwitchingSilent()
		finishRadioSwitching();
	} else
	{
		sendSelfMessage(radioSwitchingOverTimer, simTime() + switchTime);
	}

	return switchTime;
}

ChannelState BasePhyLayer::getChannelState() {
	Enter_Method_Silent();

	return decider->getChannelState();
}

//--DeciderToPhyInterface implementation------------

void BasePhyLayer::getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out) {
	channelInfo.getAirFrames(from, to, out);
}

ConstMapping* BasePhyLayer::getThermalNoise(simtime_t from, simtime_t to) {
	if(thermalNoise)
		thermalNoise->initializeArguments(Argument(from));

	return thermalNoise;
}

void BasePhyLayer::sendControlMsg(cMessage* msg) {
	sendControlMessageUp(msg);
}

void BasePhyLayer::sendUp(AirFrame* frame, DeciderResult* result) {

	coreEV << "Decapsulating MacPacket from Airframe with ID " << frame->getId() << " and sending it up to MAC." << endl;

	cMessage* packet = frame->decapsulate();

	assert(packet);

	PhyToMacControlInfo* ctrlInfo = new PhyToMacControlInfo(result);

	packet->setControlInfo(ctrlInfo);

	sendMacPktUp(packet);
}

simtime_t BasePhyLayer::getSimTime() {

	return simTime();
}

void BasePhyLayer::cancelScheduledMessage(cMessage* msg) {
	if(msg->isScheduled()){
		cancelEvent(msg);
	} else {
		EV << "Warning: Decider wanted to cancel a scheduled message but message"
		   << " wasn't actually scheduled. Message is: " << msg << endl;
	}
}
