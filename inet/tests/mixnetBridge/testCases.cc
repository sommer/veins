#include <MixnetWorldUtility.h>
#include <iostream>
#include <sstream>
#include <string>
#include <asserts.h>
#include <OmnetTestBase.h>
#include <FindModule.h>
#include <IInterfaceTable.h>
#include <MixnetBridge.h>
#include <Ieee802Ctrl_m.h>
#include <NetwToMacControlInfo.h>
#include <SimpleAddress.h>



class OmnetTest:public OmnetTestBase {
protected:
	cPacket* testPacket;
	enum {
		NETW_TO_MAC = 0,
		MAC_TO_NETW,
		BROADCAST_TO_MAC,
		NO_MORE_MESSAGES
	};
	int state;

	int run;
public:
	OmnetTest():
		OmnetTestBase()
	{
		testsExecuted = true;
	}

	virtual void finish() {
		delete testPacket;
	}

	virtual int numInitStages() const {return 2;}
	virtual void initialize(int stage){
		if(stage == 0) {
			state = NO_MORE_MESSAGES;
			testPacket = new cPacket("test packet");
			run = simulation.getSystemModule()->par("run").longValue();
			if(run < 1) {
				testsExecuted = false;
			}

		}
		else if(stage == 1) {
			if(run == 0) {
				state = NETW_TO_MAC;
				Ieee802Ctrl* ctrl = new Ieee802Ctrl();
				ctrl->setDest(MACAddress("11 11 11 11 11 11"));
				testPacket->setControlInfo(ctrl);

				send(testPacket, "toUpperBridge");
			} else if(run == -1) {
				Ieee802Ctrl* ctrl = new Ieee802Ctrl();
				ctrl->setDest(MACAddress("11 11 11 11 11 12"));
				testPacket->setControlInfo(ctrl);

				send(testPacket, "toUpperBridge");
				testsExecuted = true;
			}
		}
	}

	virtual void handleMessage(cMessage* msg) {
		if(msg->arrivedOn("fromUpperBridge") || msg->arrivedOn("fromLowerBridge")) {
			switch(state) {
			case NETW_TO_MAC:
			{
				assertEqual("Netw to mac: Packet should have stayed the same.",
							testPacket, msg);
				NetwToMacControlInfo* ctrl = dynamic_cast<NetwToMacControlInfo*>(testPacket->removeControlInfo());
				assertTrue("Netw to mac: Control info should've been converted to "
							"Mixims type.", ctrl != NULL);
				assertEqual("Netw to mac: Dest MAC should be id of this module "
							"(because it serves as a NIC here).",
							getId(), ctrl->getNextHopMac());

				send(testPacket, "toLowerBridge");
				state = MAC_TO_NETW;
				break;
			}
			case MAC_TO_NETW:
			{
				assertEqual("Mac to netw: Packet should have stayed the same.",
							testPacket, msg);

				state = BROADCAST_TO_MAC;
				Ieee802Ctrl* ctrl = new Ieee802Ctrl();
				ctrl->setDest(MACAddress::BROADCAST_ADDRESS);
				testPacket->setControlInfo(ctrl);

				send(testPacket, "toUpperBridge");

				break;
			}
			case BROADCAST_TO_MAC:
			{
				assertEqual("BC to mac: Packet should have stayed the same.",
							testPacket, msg);
				NetwToMacControlInfo* ctrl = dynamic_cast<NetwToMacControlInfo*>(testPacket->removeControlInfo());
				assertTrue("BC to mac: Control info should've been converted to "
							"Mixims type.", ctrl != NULL);
				assertEqual("BC to mac: Dest MAC should be L2BROADCAST.",
							(int)L2BROADCAST, ctrl->getNextHopMac());

				send(new cMessage("dumpCtrl"), "toCtrlLowerBridge");
				testsExecuted = true;
				state = NO_MORE_MESSAGES;
				break;
			}
			case NO_MORE_MESSAGES:
				assertFalse("Got unexpected message!", true);
				break;

			default:
				assertFalse("Undefined state!", true);
				break;
			}
		} else {
			assertFalse("Unexpected message at unexpected gate!", true);
		}
	}
protected:

	void runTests() {
	}
};

Define_Module(OmnetTest);


class DummyTable: public cSimpleModule,
				  public IInterfaceTable {
protected:

public:

	/**
     * Adds an interface. The second argument should be a module which belongs
     * to the physical interface (e.g. PPP or EtherMac) -- it will be used
     * to discover and fill in getNetworkLayerGateIndex(), getNodeOutputGateId(),
     * and getNodeInputGateId() in InterfaceEntry. It should be NULL if this is
     * a virtual interface (e.g. loopback).
     */
    virtual void addInterface(InterfaceEntry *entry, cModule *ifmod)
    {
    	if(simulation.getSystemModule()->par("run").longValue() > 0)
    		return;

    	MixnetBridge* bridge = FindModule<MixnetBridge*>::findGlobalModule();

    	assertTrue("OmnetTest module should be present.", bridge != NULL);
    	assertEqual("Pointer to interface module should be the OmnetTest module.",
					bridge, ifmod);

    	assertEqual("MTU correctly set?", 1500, entry->getMTU());
    	assertEqual("MACAddress correctly set?",
					MACAddress("11 11 11 11 11 11"), entry->getMacAddress());
    	assertTrue("Broadcast capability set?", entry->isBroadcast());
    	assertTrue("Multicast capability set?", entry->isMulticast());
    	assertFalse("Point-to-point capability set?", entry->isPointToPoint());
    	assertFalse("Is not loopback device.", entry->isLoopback());
    	assertTrue("Name is not NULL.", entry->getName());
    	std::string name = entry->getName();
    	assertTrue("name is not empty string.", name != "");
    }


    virtual std::string getFullPath() const { return ""; }
    virtual void deleteInterface(InterfaceEntry *entry) {}
    virtual int getNumInterfaces() { return 0; }
    virtual InterfaceEntry *getInterface(int pos) { return NULL; }
    virtual InterfaceEntry *getInterfaceById(int id) { return NULL; }
    virtual InterfaceEntry *getInterfaceByNodeOutputGateId(int id) { return NULL; }
    virtual InterfaceEntry *getInterfaceByNodeInputGateId(int id) { return NULL; }
    virtual InterfaceEntry *getInterfaceByNetworkLayerGateIndex(int index) { return NULL; }
    virtual InterfaceEntry *getInterfaceByName(const char *name) { return NULL; }
    virtual InterfaceEntry *getFirstLoopbackInterface() { return NULL; }

protected:
    virtual void interfaceChanged(InterfaceEntry *entry, int category) {}
};
Define_Module(DummyTable);
