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


/**
 * @brief Executes test cases for class MixnetBridge
 *
 * - initialize:
 *   - test correct registration of NIC with InterfaceTable @DummyTable::addInterface
 *     - mac address
 *       - from parameter
 *         - invalid
 *         - valid
 *       - automatic
 *     - name
 *       - not empty
 *       - not NULL
 *     - can broadcast?
 *     - can multicast?
 *     - can point to point?
 *     - mtu
 *     - pointer to NIC module
 *   - pointer to NIC
 *     - no NIC @Config Test6
 *     - valid NIC @Config Test1 et al.
 *   - pointer to world utility
 *     - no world utility @Config Test2
 *     - no MixnetWorldUtility @Config Test3
 *     - valid MixnetWorldUtility @Config Test1 et al.
 *   - register MAC address pair with world utility
 *     - INET @TestWorldUtility::addMACAddressPair
 *     - MiXiM
 *       - address should be default addressing scheme (NIC modules id)
 *         @TestWorldUtility::addMACAddressPair
 *       - warning if addressing module found
 *         @Config Test4
 *     - registeration of address pair in worldutility
 *       @TestWorldUtility::addMACAddressPair
 *
 * - check forwarding of lower to upper messages @state MAC_TO_NETW
 *   - should not touch/change forwarded message
 * - check dump of lower to upper control messages @state BROADCAST_TO_MAC
 * - check forwarding of upper to lower message
 *   - convert Ieee802Ctrl to NetwToMacCtrlInfo @state NETW_TO_MAC
 *   - convert Ieee802Ctrl's destAddr from MACAddress to Mixim int
 *     - broadcast @state BROADCAST_TO_MAC
 *     - unicast @state NETW_TO_MAC
 *     - invalid address @Config Test5

 */
class OmnetTest:public SimpleTest {
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
		SimpleTest(),
		testPacket(NULL)
	{
		testsExecuted = true;
	}

	virtual ~OmnetTest() {
		if(testPacket && !testPacket->isScheduled() && run != -1)
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

class TestWorldUtility: public MixnetWorldUtility {
protected:
	bool addressRegistered;
public:
	TestWorldUtility():
		addressRegistered(false)
	{}

	virtual void finish() {
		MixnetWorldUtility::finish();

		int run = simulation.getSystemModule()->par("run").longValue();
		assertTrue("Address-pair should have been registered.",
				   (run > 0) || addressRegistered);
	}

	virtual void addMACAddrPair(const MACAddress& inetAddr, int miximAddr) {
		MixnetWorldUtility::addMACAddrPair(inetAddr, miximAddr);

		int run = simulation.getSystemModule()->par("run").longValue();
    	if(run > 0)
    		return;

    	OmnetTest* nic = FindModule<OmnetTest*>::findGlobalModule();

    	assertEqual("Mixim MAC address correctly set?",
					nic->getId(), miximAddr);

    	if(run == 0) {
			assertEqual("MACAddress correctly set?",
						MACAddress("11 11 11 11 11 11"), inetAddr);
    	}else if(run == -1) {
    		assertFalse("MACAddress specified?",
						inetAddr.isUnspecified());
    		assertFalse("MACAddress not broadcast?",
						inetAddr.isBroadcast());
    		assertFalse("MACAddress not multicast?",
						inetAddr.isMulticast());
    	}

    	addressRegistered = true;
	}

	//virtual int getMiximMACAddr(const MACAddress& inetAddr) const;
};
Define_Module(TestWorldUtility);

class DummyTable: public cSimpleModule,
				  public IInterfaceTable {
protected:
	bool interfaceAdded;
public:
	DummyTable():
		interfaceAdded(false)
	{}

	virtual void finish() {
		int run = simulation.getSystemModule()->par("run").longValue();
		assertTrue("Interface should have been registered.",
				   (run > 0) || interfaceAdded);
	}

	/**
	 * Checks if bridge registered NIC correctly:
	 *
	 * - mac address
	 *   - from parameter
	 *     - invalid
	 *     - valid
	 *   - automatic
	 * - name
	 *   - not empty
	 *   - not NULL
	 * - can broadcast?
	 * - can multicast?
	 * - can point to point?
	 * - is loopback?
	 * - mtu
	 * - pointer to interface module
	 *
	 */
    virtual void addInterface(InterfaceEntry *entry, cModule *ifmod)
    {
    	int run = simulation.getSystemModule()->par("run").longValue();
    	if(run > 0)
    		return;

    	MixnetBridge* bridge = FindModule<MixnetBridge*>::findGlobalModule();

    	assertTrue("OmnetTest module should be present.", bridge != NULL);
    	assertEqual("Pointer to interface module should be the OmnetTest module.",
					bridge, ifmod);

    	assertEqual("MTU correctly set?", 1500, entry->getMTU());
    	assertTrue("Broadcast capability set?", entry->isBroadcast());
    	assertTrue("Multicast capability set?", entry->isMulticast());
    	assertFalse("Point-to-point capability set?", entry->isPointToPoint());
    	assertFalse("Is not loopback device.", entry->isLoopback());
    	assertTrue("Name is not NULL.", entry->getName());
    	std::string name = entry->getName();
    	assertTrue("name is not empty string.", name != "");

    	if(run == 0) {
			assertEqual("MACAddress correctly set?",
						MACAddress("11 11 11 11 11 11"), entry->getMacAddress());
    	}else if(run == -1) {
    		assertFalse("MACAddress specified?",
						entry->getMacAddress().isUnspecified());
    		assertFalse("MACAddress not broadcast?",
						entry->getMacAddress().isBroadcast());
    		assertFalse("MACAddress not multicast?",
						entry->getMacAddress().isMulticast());
    	}

    	interfaceAdded = true;
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
