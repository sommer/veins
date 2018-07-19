#include "veins/base/phyLayer/BasePhyLayer.h"

#include <string>
#include <sstream>
#include <vector>
#include "veins/base/phyLayer/MacToPhyControlInfo.h"
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/utils/POA.h"
#include "veins/modules/phy/SampledAntenna1D.h"
#include "veins/base/phyLayer/AnalogueModel.h"
#include "veins/base/phyLayer/Decider.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"

#ifndef coreEV
#define coreEV_clear EV
#define coreEV EV << logName() << "::" << getClassName() << ": "
#endif

using namespace Veins;

using Veins::AirFrame;

Define_Module(Veins::BasePhyLayer);

Coord NoMobiltyPos = Coord::ZERO;

//--Initialization----------------------------------

BasePhyLayer::BasePhyLayer()
    : protocolId(GENERIC)
    , thermalNoiseValue(0)
    , radio(0)
    , decider(0)
    , radioSwitchingOverTimer(0)
    , txOverTimer(0)
    , headerLength(-1)
    , world(NULL)
{
}

template <class T>
T BasePhyLayer::readPar(const char* parName, const T defaultValue)
{
    if (hasPar(parName))
        return par(parName);
    else
        return defaultValue;
}

// the following line is needed to allow linking when compiled in RELEASE mode.
// Add a declaration for each parameterization of the template used in
// code to be linked, e.g. in modules or in examples, if it is not already
// used in base (double and simtime_t). Needed with (at least): gcc 4.4.1.
template int BasePhyLayer::readPar<int>(const char* parName, const int);
template double BasePhyLayer::readPar<double>(const char* parName, const double);
template simtime_t BasePhyLayer::readPar<simtime_t>(const char* parName, const simtime_t);
template bool BasePhyLayer::readPar<bool>(const char* parName, const bool);

void BasePhyLayer::initialize(int stage)
{

    ChannelAccess::initialize(stage);

    if (stage == 0) {
        // if using sendDirect, make sure that messages arrive without delay
        gate("radioIn")->setDeliverOnReceptionStart(true);

        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        upperControlOut = findGate("upperControlOut");
        upperControlIn = findGate("upperControlIn");

        if (par("useThermalNoise").boolValue()) {
            thermalNoiseValue = FWMath::dBm2mW(par("thermalNoise").doubleValue());
        }
        else {
            thermalNoiseValue = 0;
        }
        headerLength = par("headerLength");
        sensitivity = par("sensitivity").doubleValue();
        sensitivity = FWMath::dBm2mW(sensitivity);

        recordStats = par("recordStats").boolValue();

        radio = initializeRadio();

        world = FindModule<BaseWorldUtility*>::findGlobalModule();
        if (world == NULL) {
            throw cRuntimeError("Could not find BaseWorldUtility module");
        }

        if (cc->hasPar("sat") && (sensitivity - FWMath::dBm2mW(cc->par("sat").doubleValue())) < -0.000001) {
            throw cRuntimeError("Sensitivity can't be smaller than the signal attenuation threshold (sat) in ConnectionManager. Please adjust your omnetpp.ini file accordingly.");
        }

        initializeAnalogueModels(par("analogueModels").xmlValue());
        initializeDecider(par("decider").xmlValue());
        initializeAntenna(par("antenna").xmlValue());

        radioSwitchingOverTimer = new cMessage("radio switching over", RADIO_SWITCHING_OVER);
        txOverTimer = new cMessage("transmission over", TX_OVER);
    }
}

Radio* BasePhyLayer::initializeRadio()
{
    int initialRadioState = par("initialRadioState");
    int nbRadioChannels = readPar("nbRadioChannels", 1);
    int initialRadioChannel = readPar("initialRadioChannel", 0);

    Radio* radio = Radio::createNewRadio(recordStats, initialRadioState, initialRadioChannel, nbRadioChannels);

    //    - switch times to TX
    // if no RX to TX defined asume same time as sleep to TX
    radio->setSwitchTime(Radio::RX, Radio::TX, (hasPar("timeRXToTX") ? par("timeRXToTX") : par("timeSleepToTX")).doubleValue());
    // if no sleep to TX defined asume same time as RX to TX
    radio->setSwitchTime(Radio::SLEEP, Radio::TX, (hasPar("timeSleepToTX") ? par("timeSleepToTX") : par("timeRXToTX")).doubleValue());

    //    - switch times to RX
    // if no TX to RX defined asume same time as sleep to RX
    radio->setSwitchTime(Radio::TX, Radio::RX, (hasPar("timeTXToRX") ? par("timeTXToRX") : par("timeSleepToRX")).doubleValue());
    // if no sleep to RX defined asume same time as TX to RX
    radio->setSwitchTime(Radio::SLEEP, Radio::RX, (hasPar("timeSleepToRX") ? par("timeSleepToRX") : par("timeTXToRX")).doubleValue());

    //    - switch times to sleep
    // if no TX to sleep defined asume same time as RX to sleep
    radio->setSwitchTime(Radio::TX, Radio::SLEEP, (hasPar("timeTXToSleep") ? par("timeTXToSleep") : par("timeRXToSleep")).doubleValue());
    // if no RX to sleep defined asume same time as TX to sleep
    radio->setSwitchTime(Radio::RX, Radio::SLEEP, (hasPar("timeRXToSleep") ? par("timeRXToSleep") : par("timeTXToSleep")).doubleValue());

    return radio;
}

void BasePhyLayer::getParametersFromXML(cXMLElement* xmlData, ParameterMap& outputMap)
{
    cXMLElementList parameters = xmlData->getElementsByTagName("Parameter");

    for (cXMLElementList::const_iterator it = parameters.begin(); it != parameters.end(); it++) {

        const char* name = (*it)->getAttribute("name");
        const char* type = (*it)->getAttribute("type");
        const char* value = (*it)->getAttribute("value");
        if (name == 0 || type == 0 || value == 0) throw cRuntimeError("Invalid parameter, could not find name, type or value");

        std::string sType = type; // needed for easier comparision
        std::string sValue = value; // needed for easier comparision

        cMsgPar param(name);

        // parse type of parameter and set value
        if (sType == "bool") {
            param.setBoolValue(sValue == "true" || sValue == "1");
        }
        else if (sType == "double") {
            param.setDoubleValue(strtod(value, 0));
        }
        else if (sType == "string") {
            param.setStringValue(value);
        }
        else if (sType == "long") {
            param.setLongValue(strtol(value, 0, 0));
        }
        else {
            throw cRuntimeError("Unknown parameter type: '%s'", sType.c_str());
        }

        // add parameter to output map
        outputMap[name] = param;
    }
}

void BasePhyLayer::finish()
{
    // give decider the chance to do something
    decider->finish();
}

//-----Decider initialization----------------------

void BasePhyLayer::initializeDecider(cXMLElement* xmlConfig)
{

    decider = 0;

    if (xmlConfig == 0) {
        throw cRuntimeError("No decider configuration file specified.");
    }

    cXMLElementList deciderList = xmlConfig->getElementsByTagName("Decider");

    if (deciderList.empty()) {
        throw cRuntimeError("No decider configuration found in configuration file.");
    }

    if (deciderList.size() > 1) {
        throw cRuntimeError("More than one decider configuration found in configuration file.");
    }

    cXMLElement* deciderData = deciderList.front();

    const char* name = deciderData->getAttribute("type");

    if (name == 0) {
        throw cRuntimeError("Could not read type of decider from configuration file.");
    }

    ParameterMap params;
    getParametersFromXML(deciderData, params);

    decider = getDeciderFromName(name, params);

    if (decider == 0) {
        throw cRuntimeError("Could not find a decider with the name \"%s\".", name);
    }

    coreEV << "Decider \"" << name << "\" loaded." << endl;
}

Decider* BasePhyLayer::getDeciderFromName(std::string name, ParameterMap& params)
{
    return 0;
}

//-----Antenna initialization----------------------

void BasePhyLayer::initializeAntenna(cXMLElement* xmlConfig)
{
    antenna = 0;

    if (xmlConfig == 0) {
        throw cRuntimeError("No antenna configuration file specified.");
    }

    cXMLElementList antennaList = xmlConfig->getElementsByTagName("Antenna");

    if (antennaList.empty()) {
        throw cRuntimeError("No antenna configuration found in configuration file.");
    }

    cXMLElement* antennaData;
    if (antennaList.size() > 1) {
        int num = intuniform(0, antennaList.size() - 1);
        antennaData = antennaList[num];
    }
    else {
        antennaData = antennaList.front();
    }

    const char* name = antennaData->getAttribute("type");

    if (name == 0) {
        throw cRuntimeError("Could not read type of antenna from configuration file.");
    }

    ParameterMap params;
    getParametersFromXML(antennaData, params);

    antenna = getAntennaFromName(name, params);

    if (antenna == 0) {
        throw cRuntimeError("Could not find an antenna with the name \"%s\".", name);
    }

    const char* id = antennaData->getAttribute("id");
    coreEV << "Antenna \"" << name << "\" with ID \"" << id << "\" loaded." << endl;
}

std::shared_ptr<Antenna> BasePhyLayer::getAntennaFromName(std::string name, ParameterMap& params)
{
    if (name == "SampledAntenna1D") {
        return initializeSampledAntenna1D(params);
    }

    return std::make_shared<Antenna>();
}

std::shared_ptr<Antenna> BasePhyLayer::initializeSampledAntenna1D(ParameterMap& params)
{
    // get samples of the modeled antenna and put them in a vector
    ParameterMap::iterator it = params.find("samples");
    std::vector<double> values;
    if (it != params.end()) {
        std::string buf;
        std::stringstream samplesStream(it->second.stringValue());
        while (samplesStream >> buf) {
            values.push_back(stod(buf));
        }
    }
    else {
        throw cRuntimeError("BasePhyLayer::initializeSampledAntenna1D(): No samples specified for this antenna. \
                Please adjust your xml file accordingly.");
    }

    // get optional random offsets for the antenna's samples
    it = params.find("random-offsets");
    std::string offsetType = "";
    std::vector<double> offsetParams;
    if (it != params.end()) {
        std::string buf;
        std::stringstream offsetStream(it->second.stringValue());
        offsetStream >> offsetType;
        while (offsetStream >> buf) {
            offsetParams.push_back(stod(buf));
        }
    }

    // get optional random rotation of the whole pattern
    it = params.find("random-rotation");
    std::string rotationType = "";
    std::vector<double> rotationParams;
    if (it != params.end()) {
        std::string buf;
        std::stringstream rotationStream(it->second.stringValue());
        rotationStream >> rotationType;
        while (rotationStream >> buf) {
            rotationParams.push_back(stod(buf));
        }
    }

    return std::make_shared<SampledAntenna1D>(values, offsetType, offsetParams, rotationType, rotationParams, this->getRNG(0));
}

//-----AnalogueModels initialization----------------

void BasePhyLayer::initializeAnalogueModels(cXMLElement* xmlConfig)
{

    if (xmlConfig == 0) {
        throw cRuntimeError("No analogue models configuration file specified.");
    }

    cXMLElementList analogueModelList = xmlConfig->getElementsByTagName("AnalogueModel");

    if (analogueModelList.empty()) {
        throw cRuntimeError("No analogue models configuration found in configuration file.");
    }

    // iterate over all AnalogueModel-entries, get a new AnalogueModel instance and add
    // it to analogueModels
    for (cXMLElementList::const_iterator it = analogueModelList.begin(); it != analogueModelList.end(); it++) {

        cXMLElement* analogueModelData = *it;

        const char* name = analogueModelData->getAttribute("type");
        const char* thresholdingFlag = analogueModelData->getAttribute("thresholding");

        if (name == 0) {
            throw cRuntimeError("Could not read name of analogue model.");
        }

        ParameterMap params;
        getParametersFromXML(analogueModelData, params);

        AnalogueModel* newAnalogueModel = getAnalogueModelFromName(name, params);

        if (newAnalogueModel == 0) {
            throw cRuntimeError("Could not find an analogue model with the name \"%s\".", name);
        }

        // attach the new AnalogueModel to the AnalogueModelList
        if (thresholdingFlag && std::string(thresholdingFlag) == "true") {
            analogueModelsThresholding.push_back(newAnalogueModel);
        }
        else {
            analogueModels.push_back(newAnalogueModel);
        }

        coreEV << "AnalogueModel \"" << name << "\" loaded." << endl;

    } // end iterator loop
}

AnalogueModel* BasePhyLayer::getAnalogueModelFromName(std::string name, ParameterMap& params)
{
    // add default analogue models here
    return 0;
}

//--Message handling--------------------------------------

void BasePhyLayer::handleMessage(cMessage* msg)
{

    // self messages
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);

        // MacPkts <- MacToPhyControlInfo
    }
    else if (msg->getArrivalGateId() == upperLayerIn) {
        handleUpperMessage(msg);

        // controlmessages
    }
    else if (msg->getArrivalGateId() == upperControlIn) {
        handleUpperControlMessage(msg);

        // AirFrames
    }
    else if (msg->getKind() == AIR_FRAME) {
        handleAirFrame(static_cast<AirFrame*>(msg));

        // unknown message
    }
    else {
        EV << "Unknown message received." << endl;
        delete msg;
    }
}

void BasePhyLayer::handleAirFrame(AirFrame* frame)
{
    // TODO: ask jerome to set air frame priority in his UWBIRPhy
    // assert(frame->getSchedulingPriority() == airFramePriority());

    switch (frame->getState()) {
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
        throw cRuntimeError("Unknown AirFrame state: %s", frame->getState());
    }
}

void BasePhyLayer::handleAirFrameStartReceive(AirFrame* frame)
{
    coreEV << "Received new AirFrame " << frame << " from channel." << endl;

    channelInfo.addAirFrame(frame, simTime());
    assert(!channelInfo.isChannelEmpty());

    if (usePropagationDelay) {
        Signal& s = frame->getSignal();
        simtime_t delay = simTime() - s.getSendingStart();
        s.setPropagationDelay(delay);
    }
    assert(frame->getSignal().getReceptionStart() == simTime());

    frame->getSignal().setReceptionSenderInfo(frame);
    filterSignal(frame);

    if (decider && isKnownProtocolId(frame->getProtocolId())) {
        frame->setState(RECEIVING);

        // pass the AirFrame the first time to the Decider
        handleAirFrameReceiving(frame);

        // if no decider is defined we will schedule the message directly to its end
    }
    else {
        Signal& signal = frame->getSignal();

        simtime_t signalEndTime = signal.getReceptionStart() + frame->getDuration();
        frame->setState(END_RECEIVE);

        sendSelfMessage(frame, signalEndTime);
    }
}

void BasePhyLayer::handleAirFrameReceiving(AirFrame* frame)
{

    Signal& signal = frame->getSignal();
    simtime_t nextHandleTime = decider->processSignal(frame);

    assert(signal.getDuration() == frame->getDuration());
    simtime_t signalEndTime = signal.getReceptionStart() + frame->getDuration();

    // check if this is the end of the receiving process
    if (simTime() >= signalEndTime) {
        frame->setState(END_RECEIVE);
        handleAirFrameEndReceive(frame);
        return;
    }

    // smaller zero means don't give it to me again
    if (nextHandleTime < 0) {
        nextHandleTime = signalEndTime;
        frame->setState(END_RECEIVE);

        // invalid point in time
    }
    else if (nextHandleTime < simTime() || nextHandleTime > signalEndTime) {
        throw cRuntimeError("Invalid next handle time returned by Decider. Expected a value between current simulation time (%.2f) and end of signal (%.2f) but got %.2f", SIMTIME_DBL(simTime()), SIMTIME_DBL(signalEndTime), SIMTIME_DBL(nextHandleTime));
    }

    coreEV << "Handed AirFrame with ID " << frame->getId() << " to Decider. Next handling in " << nextHandleTime - simTime() << "s." << endl;

    sendSelfMessage(frame, nextHandleTime);
}

void BasePhyLayer::handleAirFrameEndReceive(AirFrame* frame)
{
    coreEV << "End of Airframe with ID " << frame->getId() << "." << endl;

    simtime_t earliestInfoPoint = channelInfo.removeAirFrame(frame);

    /* clean information in the radio until earliest time-point
     * of information in the ChannelInfo,
     * since this time-point might have changed due to removal of
     * the AirFrame
     */
    if (channelInfo.isChannelEmpty()) {
        earliestInfoPoint = simTime();
    }
}

void BasePhyLayer::handleUpperMessage(cMessage* msg)
{

    // check if Radio is in TX state
    if (radio->getCurrentState() != Radio::TX) {
        delete msg;
        msg = 0;
        throw cRuntimeError("Error: message for sending received, but radio not in state TX");
    }

    // check if not already sending
    if (txOverTimer->isScheduled()) {
        delete msg;
        msg = 0;
        throw cRuntimeError("Error: message for sending received, but radio already sending");
    }

    // build the AirFrame to send
    assert(dynamic_cast<cPacket*>(msg) != 0);

    AirFrame* frame = encapsMsg(static_cast<cPacket*>(msg));

    // Prepare a POA object and attach it to the created Airframe
    BaseMobility* sendersMobility = ChannelMobilityAccessType::get(this->getParentModule());
    assert(sendersMobility);
    Coord pos = sendersMobility->getCurrentPosition();
    Coord orient = sendersMobility->getCurrentOrientation();
    POA* poa = new POA(pos, orient, antenna);
    frame->setPoa(*poa);
    // the frame is now owner of the POA object
    delete poa;
    poa = 0;

    // make sure there is no self message of kind TX_OVER scheduled
    // and schedule the actual one
    assert(!txOverTimer->isScheduled());
    sendSelfMessage(txOverTimer, simTime() + frame->getDuration());

    sendMessageDown(frame);
}

AirFrame* BasePhyLayer::encapsMsg(cPacket* macPkt)
{
    // the cMessage passed must be a MacPacket... but no cast needed here
    // MacPkt* pkt = static_cast<MacPkt*>(msg);

    // ...and must always have a ControlInfo attached (contains Signal)
    cObject* ctrlInfo = macPkt->removeControlInfo();
    assert(ctrlInfo);

    // create the new AirFrame
    AirFrame* frame = new AirFrame(macPkt->getName(), AIR_FRAME);

    // Retrieve the pointer to the Signal-instance from the ControlInfo-instance.
    // We are now the new owner of this instance.
    Signal* s = MacToPhyControlInfo::getSignalFromControlInfo(ctrlInfo);
    // make sure we really obtained a pointer to an instance
    assert(s);

    // set the members
    assert(s->getDuration() > 0);
    frame->setDuration(s->getDuration());
    // copy the signal into the AirFrame
    frame->setSignal(*s);
    // set priority of AirFrames above the normal priority to ensure
    // channel consistency (before any thing else happens at a time
    // point t make sure that the channel has removed every AirFrame
    // ended at t and added every AirFrame started at t)
    frame->setSchedulingPriority(airFramePriority());
    frame->setProtocolId(myProtocolId());
    frame->setBitLength(headerLength);
    frame->setId(world->getUniqueAirFrameId());
    frame->setChannel(radio->getCurrentChannel());

    // pointer and Signal not needed anymore
    delete s;
    s = 0;

    // delete the Control info
    delete ctrlInfo;
    ctrlInfo = 0;

    frame->encapsulate(macPkt);

    // --- from here on, the AirFrame is the owner of the MacPacket ---
    macPkt = 0;
    coreEV << "AirFrame encapsulated, length: " << frame->getBitLength() << "\n";

    return frame;
}

void BasePhyLayer::handleUpperControlMessage(cMessage* msg)
{

    throw cRuntimeError("Received unknown control message from upper layer!");
}

void BasePhyLayer::handleSelfMessage(cMessage* msg)
{

    switch (msg->getKind()) {
    // transmission over
    case TX_OVER:
        assert(msg == txOverTimer);
        sendControlMsgToMac(new cMessage("Transmission over", TX_OVER));
        break;

    // radio switch over
    case RADIO_SWITCHING_OVER:
        assert(msg == radioSwitchingOverTimer);
        finishRadioSwitching();
        break;

    // AirFrame
    case AIR_FRAME:
        handleAirFrame(static_cast<AirFrame*>(msg));
        break;

    default:
        break;
    }
}

//--Send messages------------------------------

void BasePhyLayer::sendControlMessageUp(cMessage* msg)
{
    send(msg, upperControlOut);
}

void BasePhyLayer::sendMacPktUp(cMessage* pkt)
{
    send(pkt, upperLayerOut);
}

void BasePhyLayer::sendMessageDown(AirFrame* msg)
{

    sendToChannel(msg);
}

void BasePhyLayer::sendSelfMessage(cMessage* msg, simtime_t_cref time)
{
    // TODO: maybe delete this method because it doesn't makes much sense,
    //        or change it to "scheduleIn(msg, timeDelta)" which schedules
    //        a message to +timeDelta from current time
    scheduleAt(time, msg);
}

void BasePhyLayer::filterSignal(AirFrame* frame)
{
    // determine antenna gains first
    // get POA from frame with the sender's position, orientation and antenna
    POA& senderPOA = frame->getPoa();
    // get own mobility module
    BaseMobility* ownMobility = ChannelMobilityAccessType::get(this->getParentModule());
    assert(ownMobility);
    Coord ownPos = ownMobility->getCurrentPosition();
    Coord ownOrient = ownMobility->getCurrentOrientation();
    // compute gains at sender and receiver antenna
    double ownGain = antenna->getGain(ownPos, ownOrient, senderPOA.pos);
    double otherGain = senderPOA.antenna->getGain(senderPOA.pos, senderPOA.orientation, ownPos);

    coreEV << "Sender's antenna gain: " << otherGain << endl;
    coreEV << "Own (receiver's) antenna gain: " << ownGain << endl;

    // add the resulting total gain to the attenuations list
    frame->getSignal().addUniformAttenuation(ownGain * otherGain);

    // go on with AnalogueModels
    if (analogueModels.empty() && analogueModelsThresholding.empty()) return;

    ChannelAccess* const senderModule = dynamic_cast<ChannelAccess* const>(frame->getSenderModule());
    ChannelAccess* const receiverModule = dynamic_cast<ChannelAccess* const>(frame->getArrivalModule());

    assert(senderModule);
    assert(receiverModule);

    /** claim the Move pattern of the sender from the Signal */
    ChannelMobilityPtrType sendersMobility = senderModule ? senderModule->getMobilityModule() : NULL;
    ChannelMobilityPtrType receiverMobility = receiverModule ? receiverModule->getMobilityModule() : NULL;

    const Coord sendersPos = sendersMobility ? sendersMobility->getCurrentPosition(/*sStart*/) : NoMobiltyPos;
    const Coord receiverPos = receiverMobility ? receiverMobility->getCurrentPosition(/*sStart*/) : NoMobiltyPos;

    frame->getSignal().setSenderPos(sendersPos);
    frame->getSignal().setReceiverPos(receiverPos);
    frame->getSignal().setAnalogueModelList(&analogueModelsThresholding);
    // frame->getSignal().applyAllAnalogueModels();

    for (AnalogueModelList::const_iterator it = analogueModels.begin(); it != analogueModels.end(); it++) (*it)->filterSignal(&frame->getSignal(), sendersPos, receiverPos);
}

//--Destruction--------------------------------

BasePhyLayer::~BasePhyLayer()
{
    // get AirFrames from ChannelInfo and delete
    //(although ChannelInfo normally owns the AirFrames it
    // is not able to cancel and delete them itself
    AirFrameVector channel;
    channelInfo.getAirFrames(0, simTime(), channel);

    for (AirFrameVector::iterator it = channel.begin(); it != channel.end(); ++it) {
        cancelAndDelete(*it);
    }

    // free timer messages
    if (txOverTimer) {
        cancelAndDelete(txOverTimer);
    }
    if (radioSwitchingOverTimer) {
        cancelAndDelete(radioSwitchingOverTimer);
    }

    // free Decider
    if (decider != 0) {
        delete decider;
    }

    // free AnalogueModels (RSAM cannot be part of the thresholding-list)
    for (AnalogueModelList::iterator it = analogueModels.begin(); it != analogueModels.end(); it++) {

        AnalogueModel* tmp = *it;

        if (tmp != 0) {
            delete tmp;
        }
    }

    for (AnalogueModelList::iterator it = analogueModelsThresholding.begin(); it != analogueModelsThresholding.end(); it++) {
        AnalogueModel* tmp = *it;

        if (tmp != 0) {
            delete tmp;
        }
    }

    // free radio
    if (radio != 0) {
        delete radio;
    }
}

//--MacToPhyInterface implementation-----------------------

int BasePhyLayer::getRadioState()
{
    Enter_Method_Silent();
    assert(radio);
    return radio->getCurrentState();
}

void BasePhyLayer::finishRadioSwitching()
{
    radio->endSwitch(simTime());
    sendControlMsgToMac(new cMessage("Radio switching over", RADIO_SWITCHING_OVER));
}

simtime_t BasePhyLayer::setRadioState(int rs)
{
    Enter_Method_Silent();
    assert(radio);

    if (txOverTimer && txOverTimer->isScheduled()) {
        EV_WARN << "Switched radio while sending an AirFrame. The effects this would have on the transmission are not simulated by the BasePhyLayer!";
    }

    simtime_t switchTime = radio->switchTo(rs, simTime());

    // invalid switch time, we are probably already switching
    if (switchTime < 0) return switchTime;

    // if switching is done in exactly zero-time no extra self-message is scheduled
    if (switchTime == 0.0) {
        // TODO: in case of zero-time-switch, send no control-message to mac!
        // maybe call a method finishRadioSwitchingSilent()
        finishRadioSwitching();
    }
    else {
        sendSelfMessage(radioSwitchingOverTimer, simTime() + switchTime);
    }

    return switchTime;
}

int BasePhyLayer::getPhyHeaderLength()
{
    Enter_Method_Silent();
    if (headerLength < 0) return par("headerLength");
    return headerLength;
}

void BasePhyLayer::setCurrentRadioChannel(int newRadioChannel)
{
    if (txOverTimer && txOverTimer->isScheduled()) {
        EV_WARN << "Switched channel while sending an AirFrame. The effects this would have on the transmission are not simulated by the BasePhyLayer!";
    }

    radio->setCurrentChannel(newRadioChannel);
    decider->channelChanged(newRadioChannel);
    coreEV << "Switched radio to channel " << newRadioChannel << endl;
}

int BasePhyLayer::getCurrentRadioChannel()
{
    return radio->getCurrentChannel();
}

int BasePhyLayer::getNbRadioChannels()
{
    return par("nbRadioChannels");
}

//--DeciderToPhyInterface implementation------------

void BasePhyLayer::getChannelInfo(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out)
{
    channelInfo.getAirFrames(from, to, out);
}

double BasePhyLayer::getThermalNoiseValue()
{
    return thermalNoiseValue;
}

void BasePhyLayer::sendControlMsgToMac(cMessage* msg)
{
    sendControlMessageUp(msg);
}

void BasePhyLayer::sendUp(AirFrame* frame, DeciderResult* result)
{

    coreEV << "Decapsulating MacPacket from Airframe with ID " << frame->getId() << " and sending it up to MAC." << endl;

    cMessage* packet = frame->decapsulate();

    assert(packet);

    setUpControlInfo(packet, result);

    sendMacPktUp(packet);
}

simtime_t BasePhyLayer::getSimTime()
{

    return simTime();
}

void BasePhyLayer::cancelScheduledMessage(cMessage* msg)
{
    if (msg->isScheduled()) {
        cancelEvent(msg);
    }
    else {
        EV << "Warning: Decider wanted to cancel a scheduled message but message"
           << " wasn't actually scheduled. Message is: " << msg << endl;
    }
}

void BasePhyLayer::rescheduleMessage(cMessage* msg, simtime_t_cref t)
{
    cancelScheduledMessage(msg);
    scheduleAt(t, msg);
}

void BasePhyLayer::drawCurrent(double amount, int activity)
{
    BatteryAccess::drawCurrent(amount, activity);
}

BaseWorldUtility* BasePhyLayer::getWorldUtility()
{
    return world;
}

void BasePhyLayer::recordScalar(const char* name, double value, const char* unit)
{
    ChannelAccess::recordScalar(name, value, unit);
}

/**
 * Attaches a "control info" (PhyToMac) structure (object) to the message pMsg.
 */
cObject* const BasePhyLayer::setUpControlInfo(cMessage* const pMsg, DeciderResult* const pDeciderResult)
{
    return PhyToMacControlInfo::setControlInfo(pMsg, pDeciderResult);
}
