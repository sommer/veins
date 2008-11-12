#include "BasePhyLayer.h"
#include "BaseWorldUtility.h"

#include "MacToPhyControlInfo.h"
#include "PhyToMacControlInfo.h"

#include "BaseDecider.h"

#include "analogueModel/SimplePathlossModel.h"

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
		// if using sendDirect, make sure that messages arrive without delay
		gate("radioIn")->setDeliverOnReceptionStart(true);

		//get gate ids
		upperGateIn = findGate("upperGateIn");
        upperGateOut = findGate("upperGateOut");
        upperControlOut = findGate("upperControlOut");
        upperControlIn = findGate("upperControlIn");

		//read simple ned-parameters
		//	- initialize basic parameters

		thermalNoise = readPar("thermalNoise", 0.0);
		sensitivity = readPar("sensitivity", 0.0);
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

	coreEV << "Decider \"" << name << "\" loaded." << endl;
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

	if(name == "BaseDecider"){
		double threshold = params["threshold"];
		return new BaseDecider(this, threshold, sensitivity, 0, coreDebug); //TODO: set a valid index
	}
	return 0;
}


//-----AnalogueModels initialization----------------

/**
 * Initializes the AnalogueModels with the data from the
 * passed XML-config data.
 */
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

	// add default analogue models here

	// case "RSAM", pointer is valid as long as the radio exists
	if (name == "RadioStateAnalogueModel")
	{
		return radio->getAnalogueModel();
	}
	else if (name == "SimplePathlossModel")
	{
		return createSimplePathlossModel(params);
	}

	return 0;
}

AnalogueModel* BasePhyLayer::createSimplePathlossModel(ParameterMap& params){

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
		coreEV << "createPathLossModel(): alpha set from config.xml to " << alpha << endl;


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
			coreEV << "createPathLossModel(): alpha set from ConnectionManager to " << alpha << endl;
		} else // alpha has not been specified in ConnectionManager

		{
			// set alpha to default value
			alpha = 3.5;
			coreEV << "createPathLossModel(): alpha set from default value to " << alpha << endl;
		}
	}



	// get carrierFrequency from config
	it = params.find("carrierFrequency");

	if ( it != params.end() ) // parameter carrierFrequency has been specified in config.xml
	{
		// set carrierFrequency
		carrierFrequency = it->second.doubleValue();
		coreEV << "createPathLossModel(): carrierFrequency set from config.xml to " << carrierFrequency << endl;


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
			coreEV << "createPathLossModel(): carrierFrequency set from ConnectionManager to " << carrierFrequency << endl;
		} else // carrierFrequency has not been specified in ConnectionManager

		{
			// set carrierFrequency to default value
			carrierFrequency  = 2.412e+9;
			coreEV << "createPathLossModel(): carrierFrequency set from default value to " << carrierFrequency << endl;
		}
	}

	return new SimplePathlossModel(alpha, carrierFrequency, &move, useTorus, playgroundSize, coreDebug);

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

/**
 * Handles messages received from the channel (propably AirFrames).
 */
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

/**
 * Handles incoming AirFrames with the state START_RECEIVE.
 */
void BasePhyLayer::handleAirFrameStartReceive(AirFrame* frame) {
	coreEV << "Received new AirFrame with ID " << frame->getId() << " from channel" << endl;
	channelInfo.addAirFrame(frame, simTime());

	//TODO: consider updating signal start when propagation delay is used
	assert(usePropagationDelay or frame->getSignal().getSignalStart() == simTime());

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

/**
 * Handles incoming AirFrames with the state RECEIVING.
 */
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

/**
 * Handles incoming AirFrames with the state END_RECEIVE.
 */
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

/**
 * Handles messages received from the upper layer through the
 * data gate.
 */
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

/**
 * This function encapsulates messages from the upper layer into an
 * AirFrame and sets all necessary attributes.
 *
 * The new AirFrame instance becomes owner of the Signal and MacPacket
 */
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
	frame->setDuration(s->getSignalLength());
	// copy the signal into the AirFrame
	frame->setSignal(*s);

	// pointer and Signal not needed anymore
	delete s;
	s = 0;

	// TODO TEST: check if id is really unique
	frame->setId(world->getUniqueAirFrameId());
	frame->encapsulate(macPkt);

	// --- from here on, the AirFrame is the owner of the MacPacket ---
	macPkt = 0;


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
		opp_error("Next handle time of ChannelSenseRequest returned by the Decider is smaller then current simulation time: %.2f",
				SIMTIME_DBL(nextHandleTime));
	}

	// else, i.e. nextHandleTime < 0.0, the Decider doesn't want to handle
	// the request again
}

/**
 * Handles messages received from the upper layer through the
 * control gate.
 */
void BasePhyLayer::handleUpperControlMessage(cMessage* msg){

	//TODO: we should propably process control messages independent from their
	//		kind because they are just passed to the decider anyway
	switch(msg->getKind()) {
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

/**
 * Sends the passed control message to the upper layer.
 */
void BasePhyLayer::sendControlMessageUp(cMessage* msg) {
	send(msg, upperControlOut);
}

/**
 * Sends the passed MacPkt to the upper layer.
 */
void BasePhyLayer::sendMacPktUp(cMessage* pkt) {
	send(pkt, upperGateOut);
}

/**
 * Sends the passed AirFrame to the channel
 */
void BasePhyLayer::sendMessageDown(AirFrame* msg) {

	sendToChannel(msg);
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

/**
 * Returns the current state the radio is in. See RadioState
 * for possible values.
 *
 * This method is mainly used by the mac layer.
 */
int BasePhyLayer::getRadioState() {
	Enter_Method_Silent();
	return radio->getCurrentState();
}

/**
 * @brief Called the moment the simulated switching process of the Radio is finished.
 *
 * The Radio is set the new RadioState and the MAC Layer is sent
 * a confirmation message.
 */
void BasePhyLayer::finishRadioSwitching()
{
	radio->endSwitch(simTime());
	sendControlMsg(new cMessage(0, RADIO_SWITCHING_OVER));
}

/**
 * Tells the BasePhyLayer to switch to the specified
 * radio state. The switching process can take some time
 * depending on the specified switching times in the
 * ned file.
 *
 * @return	-1: Error code if the Radio is currently switching
 *
 * 			else: switching time from the current RadioState to the new RadioState
 */
simtime_t BasePhyLayer::setRadioState(int rs) {
	Enter_Method_Silent();

	//TODO: what to do if we are currently transmitting a signal?
	simtime_t switchTime = radio->switchTo(rs, simTime());

	//invalid switch time, we are propably already switching
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

/**
 * Returns the current state of the channel. See ChannelState
 * for details.
 */
ChannelState BasePhyLayer::getChannelState() {
	Enter_Method_Silent();

	return decider->getChannelState();
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
 */
void BasePhyLayer::sendControlMsg(cMessage* msg) {
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

	coreEV << "Decapsulating MacPacket from Airframe with ID " << frame->getId() << " and sending it up to MAC." << endl;

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
