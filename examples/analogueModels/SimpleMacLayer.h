#ifndef SIMPLEMACLAYER_H_
#define SIMPLEMACLAYER_H_

#include <omnetpp.h>

#include "Signal_.h"
#include "BaseModule.h"

class MacPkt;
class MacToPhyInterface;

/**
 * @brief A simple Mac layer implementation which only shows how to
 * create Signals, send them to Phy and receive the from the Phy layer.
 *
 * This Mac layer just sends an answer packet every time it receives a packet.
 * Since both hosts do that in this simulation, it will only terminate if one
 * packet gets lost (couldn't be received by the phy layer correctly).
 *
 * @ingroup exampleAM
 */
class SimpleMacLayer:public BaseModule {
protected:
	/** @brief Pointer to the phy module of this host.*/
	MacToPhyInterface* phy;

	/** @brief Omnet gates.*/
	int  dataOut;
	int  dataIn;

	long myIndex;

	/** @brief The index of the host the next packet should be send to. */
	long nextReceiver;

	enum {
		/** @brief Used as Message kind to identify MacPackets. */
		TEST_MACPKT = 12121
	};

	/** @brief The dimensions for the signal this mac layer works with (frequency and time) */
	DimensionSet dimensions;

protected:

	/**
	 * @brief Creates a Mapping with the passed value at the passed position in freqency
	 * and time.
	 * */
	Mapping* createMapping(simtime_t_cref time, simtime_t_cref length, double freqFrom, double freqTo, double value);

	void handleMacPkt(MacPkt* pkt);
	void handleTXOver();

	/**
	 * @brief Sends the answer packet.
	 */
	void broadCastPacket();

	void sendDown(MacPkt* pkt);

	/**
	 * @brief Creates the answer packet.
	 */
	MacPkt* createMacPkt(simtime_t_cref length);

	void log(std::string msg);

public:
	//---Omnetpp parts-------------------------------
	virtual void initialize(int stage);

	virtual void handleMessage(cMessage* msg);
};

#endif /*TESTMACLAYER_H_*/
