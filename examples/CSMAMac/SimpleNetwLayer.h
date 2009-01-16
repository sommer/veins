/*
 * SimpleNetwLayer.h
 *
 *  Created on: 29.08.2008
 *      Author: Karl Wessel
 */

#ifndef SIMPLENETWLAYER_H_
#define SIMPLENETWLAYER_H_

#include <omnetpp.h>
#include <cassert>
#include <BaseModule.h>
#include <NetwPkt_m.h>
#include <SimpleAddress.h>
#include <MacControlInfo.h>

/**
 * @brief This is an implementation of a simple network layer
 * which provides only some simple routing (next hop) and
 * message forwarding (only switches).
 *
 * This network layer isn't a real network layer its only purpose
 * is to provide some message sending for the CSMA-Mac layer.
 *
 * At first there is a small "saying hello"-phase where the
 * several network layers saying hello to each other and creating
 * their routing table.
 *
 * After that phase the non-switch instances start to babbling randomly
 * to other instances without caring if they here them.
 * The switch instances does only provide packet forwarding from there on.
 *
 * The several instanceses can bable directly to each other or over
 * a switch instance.
 */
class SimpleNetwLayer : public BaseModule{
//--------members----------
protected:
	bool isSwitch;
	/** @brief time after when the module gets bored and starts jabbering. */
	simtime_t boredTime;

	int maxTtl;

	int ip;

	int dataIn;
	int dataOut;

	cMessage* startJabberTimer;

	unsigned long runningSeqNumber;

	typedef std::map<int, int> RoutingTable;

	RoutingTable routingTable;

	enum NetwPktKind{
		HELLO_WORLD = 4200,
		JABBER,
		START_TO_JABBER
	};

//--------methods----------
protected:
	void scheduleJabbering(){
		if(startJabberTimer->isScheduled()){
			cancelEvent(startJabberTimer);
		}

		scheduleAt(simTime() + boredTime, startJabberTimer);
	}

	void broadcastHelloWorld() {
		assert(!ev.isDisabled());
		ev << "Broadcasting hello world.\n";
		NetwPkt* helloWorld = new NetwPkt("helloWorld", HELLO_WORLD);

		helloWorld->setDestAddr(L3BROADCAST);
		helloWorld->setSrcAddr(ip);
		helloWorld->setSeqNum(runningSeqNumber++);
		helloWorld->setTtl(maxTtl);

		MacControlInfo* cInfo = new MacControlInfo(L2BROADCAST);

		helloWorld->setControlInfo(cInfo);

		getNode()->bubble("Hello World!");

		sendDown(helloWorld);
	}

	void sendDown(cPacket* pkt) {
		send(pkt, dataOut);
	}

	void forwardPacket(NetwPkt* pkt, int nextHop){
		NetwPkt* fwd = new NetwPkt(pkt->getName(), pkt->getKind());

		fwd->setDestAddr(pkt->getDestAddr());
		fwd->setSrcAddr(pkt->getSrcAddr());
		fwd->setSeqNum(pkt->getSeqNum());
		fwd->setTtl(pkt->getTtl() - 1);

		MacControlInfo* cInfo = new MacControlInfo(nextHop);

		fwd->setControlInfo(cInfo);

		sendDown(fwd);
	}

	void handleHelloWorld(NetwPkt* pkt){

		//who said hello?
		int srcIP = pkt->getSrcAddr();

		//we already know ourself...
		if (srcIP == ip){
			delete pkt;
			return;
		}

		//do we already know him?
		if(routingTable.count(srcIP) == 0){
			//if not add him with the mac address of the previous hop
			MacControlInfo* cInfo = static_cast<MacControlInfo*>(pkt->getControlInfo());

			int prevHop = cInfo->getNextHopMac();

			routingTable[srcIP] = prevHop;

			char buff[255];
			sprintf(buff, "Got hello from %d", srcIP);
			getNode()->bubble(buff);
			ev << buff << endl;
		}

		//if we are a switch and the time to live of the packet
		//hasn't exceeded yet forward it
		if(isSwitch){
			if(pkt->getTtl() > 0) {
				forwardPacket(pkt, L2BROADCAST);
				char buff[255];
				sprintf(buff, "%d said hello!", srcIP);
				getNode()->bubble(buff);
				ev << buff << endl;
			}
		} else {
			//otherwise reset the bored timer after when we will start jabbering
			scheduleJabbering();
		}

		delete pkt;
	}

	void jabberToSomeone(){
		assert(!isSwitch);
		assert(routingTable.size() > 0);
		int target = intrand(routingTable.size());


		RoutingTable::const_iterator it = routingTable.begin();
		for(int i = 0; i < target; ++i)
			++it;

		ev << "Jabbering - Routingtablesize:" << routingTable.size() << "  target:" << target << "  dest:" << it->first << endl;

		NetwPkt* jabber = new NetwPkt("jabber", JABBER);

		jabber->setDestAddr(it->first);
		jabber->setSrcAddr(ip);
		jabber->setSeqNum(runningSeqNumber++);
		jabber->setTtl(maxTtl);

		MacControlInfo* cInfo = new MacControlInfo(it->second);

		jabber->setControlInfo(cInfo);

		char buff[255];
		sprintf(buff, "Babbling with %d", it->first);
		getNode()->bubble(buff);
		ev << buff << endl;
		sendDown(jabber);

		scheduleJabbering();
	}

	void handleIncomingJabber(NetwPkt* pkt){
		if(isSwitch) {
			if(pkt->getDestAddr() != ip){
				assert(pkt->getTtl() > 0);
				assert(routingTable.count(pkt->getDestAddr()) > 0);

				int nextHop = routingTable[pkt->getDestAddr()];

				char buff[255];
				sprintf(buff, "%d babbles with %d", pkt->getSrcAddr(), pkt->getDestAddr());
				getNode()->bubble(buff);
				ev << buff << endl;

				forwardPacket(pkt, nextHop);
			} else {
				char buff[255];
				sprintf(buff, "%d babbles with me. But I'm a serious switch, I do not babble...", pkt->getSrcAddr());
				getNode()->bubble(buff);
				ev << buff << endl;
			}
		} else {
			assert(pkt->getDestAddr() == ip);

			char buff[255];
			sprintf(buff, "Got babbling from %d", pkt->getSrcAddr());
			getNode()->bubble(buff);
			ev << buff << endl;
		}

		delete pkt;
	}

public:
	virtual void initialize(int stage){

		if(stage == 0){
			dataOut = findGate("lowerGateOut");
			dataIn = findGate("lowerGateIn");

			isSwitch = par("isSwitch").boolValue();

			if(isSwitch)
				getParentModule()->getParentModule()->getDisplayString().setTagArg("i",0,"device/accesspoint");

			ip = par("ip").longValue();
			maxTtl = par("maxTtl").longValue();
			boredTime = par("boredTime").doubleValue();
		} else if(stage == 1) {
			startJabberTimer = new cMessage("jabber!", START_TO_JABBER);
			broadcastHelloWorld();
		}
	}

	virtual void handleMessage(cMessage* msg){

		switch(msg->getKind()){
		case HELLO_WORLD:
			handleHelloWorld(static_cast<NetwPkt*>(msg));
			break;
		case START_TO_JABBER:
			jabberToSomeone();
			break;
		case JABBER:
			handleIncomingJabber(static_cast<NetwPkt*>(msg));
			break;
		default:
			error("unknown packet type");
			break;
		}
	}


};

#endif /* SIMPLENETWLAYER_H_ */
