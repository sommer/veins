/* -*- mode:c++ -*- ********************************************************
 * file:        BasicSinkRouting.cc
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     wsn-specific modules
 * description: network layer: basic source-to-sink routing
 ***************************************************************************/


#include "BasicSinkRouting.h"
#include "NetwControlInfo.h"
#include "MacControlInfo.h"
#include "NicControlType.h"

Define_Module(BasicSinkRouting);

void BasicSinkRouting::toNetwork(NetwPkt * out)
{
	if (msgBusy)
	{
		msgQueue->push(out);
		EV << "Local link is busy, queuing for future send" << endl;
	}
	else if (out->getDestAddr() == SINK_ADDRESS && !setNextHop(out))
	{
		msgQueue->push(out);
		EV << "Msg for sink and we don't have any!" << endl;
	}
	else
	{
		msgBusy = true;
		EV << "Pushing over local link" << endl;
		sendDown(out);
	}
}

NetwPkt *BasicSinkRouting::buildSink(SinkInfo * sink, int from)
{
	NetwPkt *pkt = buildPkt(SINK_BCAST, L3BROADCAST, "sink");
	if (sink == NULL)
	{
		sink = new SinkInfo();
		sink->setSinkId(myNetwAddr);
		sink->setParent(-1);
		sink->setCost(0);
	}
	else
	{
		sink = check_and_cast < SinkInfo * >(sink->dup());
		sink->setCost(sink->getCost() + 1);
		sink->setParent(from);
	}
	pkt->encapsulate(sink);
	return pkt;
}

void BasicSinkRouting::initialize(int stage)
{
	BaseNetwLayer::initialize(stage);
	Timer::init(this);

	/*if (stage == 0)
	{
		hasPar("debug") ? debug = par("debug").boolValue() : debug = false;
	}
	else */if (stage == 1)
	{
		headerLength = par("headerLength");
		arp = BaseArpAccess().get();
		myNetwAddr = id();
		EV << " myNetwAddr " << myNetwAddr << endl;
		msgQueue = new std::queue < NetwPkt * >();
		sinks = new std::map < int, SinkInfo * >();
		msgBusy = false;
	}
	else if (stage == 2)
	{
		cModule *node = getNode();
		if (node->hasPar("isSink") && node->par("isSink").boolValue())
		{
			isSink = true;
			EV << "Sink node, broadcasting\n";
			NetwPkt *out = buildSink();
			(*sinks)[myNetwAddr] = check_and_cast < SinkInfo * >(out->encapsulatedMsg()->dup());
			msgQueue->push(out);
			setTimer(0, 0.001);
		}
		else
			isSink = false;
	}
}

BasicSinkRouting::~BasicSinkRouting()
{
	delete msgQueue;
	for (std::map < int, SinkInfo * >::iterator si = sinks->begin(); si != sinks->end(); si++)
	{
		delete(*si).second;
	}
	delete sinks;
}

void BasicSinkRouting::finish()
{
	BaseNetwLayer::finish();
	printSinks();
}

void BasicSinkRouting::printSinks()
{
	for (std::map < int, SinkInfo * >::iterator si = sinks->begin(); si != sinks->end(); si++)
	{
		SinkInfo *s = (*si).second;
		EV << "Sink " << s->getSinkId() << " is findable via parent " << s->getParent() << " (macAddr = " << (s->getParent() == -1 ? -1 : arp->getMacAddr(s->getParent())) << ") with cost " << s->getCost() << endl;
	}
}

/**
 * Decapsulates the packet from the received Network packet 
 **/
cMessage *BasicSinkRouting::decapsMsg(NetwPkt * msg)
{
	cMessage *m = msg->decapsulate();
	m->setControlInfo(new NetwControlInfo(msg->getSrcAddr()));
	// delete the netw packet
	delete msg;
	return m;
}

bool BasicSinkRouting::setNextHop(NetwPkt * pkt)
{
	int macAddr;
	delete pkt->removeControlInfo();
	if (pkt->getDestAddr() != SINK_ADDRESS)
		error("non-sink packet!");
	if (sinks->size() > 0)
	{
		printSinks();
		macAddr = arp->getMacAddr(sinks->begin()->second->getParent());
		EV << "toNetwork: nHop=SINK_ADDRESS -> sending to sink via parent " << macAddr << endl;
	}
	else
	{
		// don't know where to send this yet
		EV << "toNetwork: nHop=SINK_ADDRESS -> need to find a sink to send this to" << endl;
		macAddr = -2;
	}
	pkt->setControlInfo(new MacControlInfo(macAddr));
	return macAddr != -2;
}

NetwPkt *BasicSinkRouting::buildPkt(int kind, int netwAddr, const char *name)
{
	int macAddr;
	NetwPkt *pkt = new NetwPkt(name, kind);
	pkt->setLength(headerLength);
	pkt->setSrcAddr(myNetwAddr);
	pkt->setDestAddr(netwAddr);
	EV << " netw " << myNetwAddr << " sending packet" << endl;
	if (netwAddr == L3BROADCAST)
	{
		EV << "toNetwork: nHop=L3BROADCAST -> message has to be broadcasted" << " -> set destMac=L2BROADCAST\n";
		macAddr = L2BROADCAST;
	}
	else if (netwAddr == SINK_ADDRESS)
	{
		//setNextHop(pkt);
		return pkt;
	}
	else
	{
		EV << "toNetwork: get the MAC address\n";
		macAddr = arp->getMacAddr(netwAddr);
	}

	pkt->setControlInfo(new MacControlInfo(macAddr));

	return pkt;
}

/**
 * Encapsulates the received ApplPkt into a NetwPkt and set all needed
 * header fields.
 **/
NetwPkt *BasicSinkRouting::encapsMsg(cMessage * msg)
{
	EV << "in encaps...\n";
	int netwAddr;

	NetwControlInfo *cInfo = dynamic_cast < NetwControlInfo * >(msg->removeControlInfo());

	if (cInfo == NULL)
	{
		error("Application layer did not specify a destination L3 address");
		netwAddr = L3BROADCAST;
	}
	else
	{
		EV << "CInfo removed, netw addr=" << cInfo->getNetwAddr() << endl;
		netwAddr = cInfo->getNetwAddr();
		delete cInfo;
	}

	NetwPkt *pkt = buildPkt(UPPER_TYPE, netwAddr, msg->name());

	//encapsulate the application packet
	pkt->encapsulate(msg);
	EV << " pkt encapsulated\n";
	return pkt;
}

/**
 * Redefine this function if you want to process messages from lower
 * layers before they are forwarded to upper layers
 *
 *
 * If you want to forward the message to upper layers please use
 * @ref sendUp which will take care of decapsulation and thelike
 **/
void BasicSinkRouting::handleLowerMsg(cMessage * msg)
{
	NetwPkt *m = check_and_cast < NetwPkt * >(msg);
	EV << "handling packet from " << m->getSrcAddr() << endl;
	EV << "Incoming type is " << m->kind() << endl;
	switch (m->kind())
	{
		case UPPER_TYPE:
			{
				if (m->getDestAddr() == SINK_ADDRESS && !isSink && (!hasPar("autoForward") || par("autoForward").boolValue()))
				{
					//printSinks();
					EV << "Sink packet going through" << endl;
					toNetwork(m);
				}
				else
					sendUp(decapsMsg(m));
				break;
			}
		case SINK_BCAST:
			{
				int srcAddr = m->getSrcAddr();
				SinkInfo *si = check_and_cast < SinkInfo * >(decapsMsg(m));
				EV << "got new sink info " << si->getSinkId() << " from node " << srcAddr << " with cost " << si->getCost() << endl;
				std::map < int, SinkInfo * >::iterator i = sinks->find(si->getSinkId());
				if (i != sinks->end())
				{
					SinkInfo *old = (*i).second;
					if (old->getCost() <= si->getCost() + 1)
					{
						EV << "new sink is too expensive. Have " << old->getParent() << " with cost " << old->getCost() << endl;
						delete si;
						break;
					}
					else
					{
						EV << "new sink is cheap! Have " << old->getParent() << " with cost " << old->getCost() << endl;
						sinks->erase(i);
						delete old;
					}
				}
				else
					EV << "Brand new sink!" << endl;
				NetwPkt *out = buildSink(si, srcAddr);
				SinkInfo *ns = check_and_cast < SinkInfo * >(si->dup());
				ns->setParent(srcAddr);
				ns->setCost(ns->getCost() + 1);
				(*sinks)[ns->getSinkId()] = ns;
				EV << "got new sink " << ns->getSinkId() << " and my parent node is " << ns->getParent() << endl;

				if (sinks->size() > 1)
					error("Panic! Got more than 1 sinks... we can't handle that yet");

				toNetwork(out);
				delete si;
				break;
			}

		default:
			error("one of mine, but not handling");
			break;
	}
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * For the BasicSinkRouting we just use the destAddr of the network
 * message as a nextHop
 *
 * To forward the message to lower layers after processing it please
 * use @ref toNetwork. It will take care of anything needed
 **/

void BasicSinkRouting::handleUpperMsg(cMessage * msg)
{
	if (isSink)
	{
		EV << "D'oh. I'm the sink" << endl;
		sendUp(msg);
	}
	else
	{
		EV << "Sending upper layer packet to sink" << endl;
		toNetwork(encapsMsg(msg));
	}
}


void BasicSinkRouting::handleLowerControl(cMessage * msg)
{
	switch (msg->kind())
	{
		case NicControlType::TRANSMISSION_OVER:
			EV << "Transmission complete" << endl;
			msgBusy = false;
			sendQueued();
			delete msg;
			break;
		default:
			EV << "BaseSinkRouting does not handle control messages of this type (name was " << msg->name() << " kind was " << msg->kind() << ")" << endl;
			delete msg;
			break;
	}
}

void BasicSinkRouting::sendQueued()
{
	if (!msgQueue->empty())
	{
		EV << "Sending queued msg" << endl;
		NetwPkt *send = msgQueue->front();
		msgQueue->pop();		// trash message
		toNetwork(send);
	}
}

void BasicSinkRouting::handleTimer(unsigned int count)
{
	sendQueued();
}
