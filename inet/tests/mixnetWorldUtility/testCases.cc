#include <MixnetWorldUtility.h>
#include <iostream>
#include <sstream>
#include <string>
#include <asserts.h>
#include <OmnetTestBase.h>
#include <FindModule.h>



class OmnetTest:public SimpleTest {
protected:

public:
	virtual int numInitStages() const {return 2;}
	virtual void initialize(int stage){
		if(stage == 1) {
			runTests();
		}
	}
protected:
	typedef std::pair<MACAddress, int> MACPair;


	void runTests() {
		displayPassed = true;

		MixnetWorldUtility* world =
				FindModule<MixnetWorldUtility*>::findGlobalModule();
		assertTrue("MixnetWorldUtility should be present in Test simulation.",
				   world != NULL);

		assertEqual("Getting mixim address from empty database (default).",
					MixnetWorldUtility::NoMacPairFound,
					world->getMiximMACAddr(MACAddress()));

		assertEqual("Getting mixim address from empty database (broadcast).",
					MixnetWorldUtility::NoMacPairFound,
					world->getMiximMACAddr(MACAddress::BROADCAST_ADDRESS));

		MACPair pair1(MACAddress::generateAutoAddress(), 1);
		MACPair pair2(MACAddress::generateAutoAddress(), 2);

		world->addMACAddrPair(pair1.first, pair1.second);
		assertEqual("Getting mixim address for first added pair.",
					pair1.second,
					world->getMiximMACAddr(pair1.first));
		assertEqual("Getting mixim address for not added pair.",
					MixnetWorldUtility::NoMacPairFound,
					world->getMiximMACAddr(pair2.first));

		world->addMACAddrPair(pair2.first, pair2.second);
		assertEqual("Getting mixim address for first added pair.",
					pair1.second,
					world->getMiximMACAddr(pair1.first));
		assertEqual("Getting mixim address for second added pair.",
					pair2.second,
					world->getMiximMACAddr(pair2.first));

		//add first pair again
		world->addMACAddrPair(pair1.first, pair1.second);
		assertEqual("Getting mixim address for first added pair.",
					pair1.second,
					world->getMiximMACAddr(pair1.first));
		assertEqual("Getting mixim address for second added pair.",
					pair2.second,
					world->getMiximMACAddr(pair2.first));

		//check for warnings (only possible by comparing the output with
		//expected output

		//change mixim address of first pair
		world->addMACAddrPair(pair1.first, 3);
		//one warning should occur
		assertEqual("Getting mixim address for mixim duplicate.",
					3,
					world->getMiximMACAddr(pair1.first));

		//change inet address of first pair
		MACAddress tmpAddr = MACAddress::generateAutoAddress();
		world->addMACAddrPair(tmpAddr, pair1.second);
		//one warning should occur
		assertEqual("Getting mixim address for mixim duplicate.",
					3,
					world->getMiximMACAddr(pair1.first));
		assertEqual("Getting mixim address for inet duplicate.",
					pair1.second,
					world->getMiximMACAddr(tmpAddr));

		//change both address of first pair
		world->addMACAddrPair(pair1.first, pair1.second);
		//two warnings should occur
		assertEqual("Getting mixim address for mixim duplicate.",
					pair1.second,
					world->getMiximMACAddr(pair1.first));
		assertEqual("Getting mixim address for inet duplicate.",
					pair1.second,
					world->getMiximMACAddr(tmpAddr));

		//add a pair with the MixnetWorldUtility::NoMacPairFound error value as
		//MAC address (should generate a warning)
		MACPair pair3(MACAddress::generateAutoAddress(), MixnetWorldUtility::NoMacPairFound);

		world->addMACAddrPair(pair3.first, pair3.second);
		assertEqual("Getting mixim address for error pair.",
					pair3.second,
					world->getMiximMACAddr(pair3.first));
		assertEqual("Getting mixim address for second pair.",
					pair2.second,
					world->getMiximMACAddr(pair2.first));

		testsExecuted = true;
	}
};

Define_Module(OmnetTest);
