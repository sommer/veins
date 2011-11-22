#include "SimpleMacLayer.h"

#include "Mapping.h"
#include "FindModule.h"
#include "MacToPhyInterface.h"
#include "MacToPhyControlInfo.h"
#include "MacPkt_m.h"

using std::endl;

Define_Module(SimpleMacLayer);

//---omnetpp part----------------------

//---intialisation---------------------
void SimpleMacLayer::initialize(int stage) {
	BaseModule::initialize(stage);

	if(stage == 0) {
		myIndex = findHost()->getIndex();

		dimensions.addDimension(Dimension::time);
		dimensions.addDimension(Dimension::frequency);

		dataOut = findGate("lowerLayerOut");
		dataIn = findGate("lowerLayerIn");

		phy = FindModule<MacToPhyInterface*>::findSubModule(this->getParentModule());

	} else if(stage == 1) {
		if(myIndex == 0){ //host with index 0 start with sending packets
			log("Switching radio to TX...");
			nextReceiver = 1;
			phy->setRadioState(Radio::TX); //to be able to send packets it has to switch to TX mode

		}else{ //every other host start in receiving mode
			log("Switching radio to RX...");
			phy->setRadioState(Radio::RX);
		}
		//switching the radio can take some time,
		//we will get a "RADIO_SWITCHING_OVER" message as soon as the radio has switched.
	}
}

void SimpleMacLayer::handleMessage(cMessage* msg) {

	//what to do if the radio has switched
	if(msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
		log("...switching radio done.");

		switch(phy->getRadioState()) {
		case Radio::TX: 		//if we switched to TX mode we probably wanted to send
			broadCastPacket();	//an answer packet
			break;
		default:
			break;
		}
		delete msg;

	//forward MacPackets and TX_OVER to their own handling routine
	} else if (msg->getKind() == TEST_MACPKT) {
		handleMacPkt(static_cast<MacPkt*>(msg));
	} else if(msg->getKind() == MacToPhyInterface::TX_OVER) {
		handleTXOver();
		delete msg;
	}

}

void SimpleMacLayer::handleTXOver() {
	//the TX_Over message is sent by the phy to us to indicate that
	//the sending process is over. SO we will switch back to receiving
	//mode after that.
	log("Transmission over signal from PhyLayer received. Changing back to RX");
	phy->setRadioState(Radio::RX);
}

void SimpleMacLayer::handleMacPkt(MacPkt* pkt) {

	//if we got a Mac packet check if it was for us or not.
	if(pkt->getDestAddr() == LAddress::L2Type(myIndex)){
		log("Received MacPkt for me - broadcasting answer (but first change to TX mode)");
		if(myIndex == 0)
			nextReceiver = 1;
		else
			nextReceiver = 0;

		//if the destination of the packet was us, send an answer packet->switch to sending mode
		phy->setRadioState(Radio::TX);
	}else
		log("Received MacPkt - but not for me.");

	delete pkt;

}

void SimpleMacLayer::log(std::string msg) {
	ev << "[Host " << myIndex << "] - MacLayer: " << msg << endl;
}

void SimpleMacLayer::broadCastPacket() {
	//create the answer packet of length 64kByte / 11kByte/sec
	MacPkt* pkt = createMacPkt(64.0 / 11000.0);

	log("Sending broadcast packet to phy layer.");
	//pass it to the phy layer
	sendDown(pkt);
}

void SimpleMacLayer::sendDown(MacPkt* pkt) {
	send(pkt, dataOut);
}

Mapping* SimpleMacLayer::createMapping(simtime_t_cref time, simtime_t_cref length, double freqFrom, double freqTo, double value){
	//create mapping for frequency and time
	Mapping* m = MappingUtils::createMapping(Argument::MappedZero, dimensions, Mapping::LINEAR);

	//set position Argument
	Argument pos(dimensions, time);
	pos.setArgValue(Dimension("frequency"), freqFrom);
	//set mapping at position
	m->setValue(pos, value);

	pos.setTime(time + length);
	//set mapping at position
	m->setValue(pos, value);

	pos.setArgValue(Dimension("frequency"), freqTo);
	//set mapping at position
	m->setValue(pos, value);

	pos.setTime(time);
	//set mapping at position
	m->setValue(pos, value);



	return m;
}

MacPkt* SimpleMacLayer::createMacPkt(simtime_t_cref length) {
	//create signal with start at current simtime and passed length
	Signal* s = new Signal(simTime(), length);

	//create and set tx power mapping
	Mapping* txPower = createMapping(simTime(), length, 2.412e9, 2.472e9, 50.0);
	s->setTransmissionPower(txPower);

	//create and set bitrate mapping
	Mapping* bitrate = createMapping(simTime(), length, 2.412e9, 2.472e9, 54.0);
	s->setBitrate(bitrate);

	//create and initialize control info
	MacPkt* res = new MacPkt();
	MacToPhyControlInfo::setControlInfo(res, s);
	res->setKind(TEST_MACPKT);
	res->setDestAddr(LAddress::L2Type(nextReceiver));
	return res;
}
