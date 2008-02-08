#ifndef BASEPHYLAYER_
#define BASEPHYLAYER_

#include "ChannelAccess.h"
#include "DeciderToPhyInterface.h"
#include "MacToPhyInterface.h"

#include "AnalogueModel.h"
#include "Decider.h"
#include "ChannelInfo.h"

#include "MacPkt_m.h"

#include <cxmlelement.h>
#include <map>
#include <vector>
#include <iostream>


/**
 * The BasePhyLayer represents the physical layer of a nic.
 * The BasePhyLayer is directly connected to the mac layer via
 * OMNeT channels and is able to send messages to other physical
 * layers through subclassing from ChannelAcces.
 * 
 * The BasePhyLayer encapsulates two sub modules. 
 * The AnalogueModels, which are responsible for simulating
 * the attenuation of received signals and the Decider which
 * provides the main functionality of the physical layer like
 * signal classification (noise or not noise) and demodulation
 * (calculating transmission errors).
 * 
 * The BasePhyLayer itself is responsible for the OMNeT 
 * depended parts of the physical layer which are the following:
 * 
 * Module initialisation:
 * - read ned-parameters and initialice module, Decider and 
 *   AnalogueModels.
 * 
 * Message handling:
 * - receive messages from mac layer and hand them to the Decider
 *   or directly send them to the channel
 * - receive AirFrames from the channel, hand them to the 
 *   AnalogueModels for filtering, simulate delay and transmission 
 *   duration, hand it to the Decider for evaluation and send
 *   receifed packets to the mac layer
 * - keep track of currently active AirFrames on the channel
 *   (see ChannelInfo)
 * 
 * The actual evaluation of incoming signals is done by the 
 * Decider. 
 * 
 * In most cases it should be sufficient to only write your own 
 * Decider or AnalogueModel instead of subclassing from 
 * BasePhyLayer if you want to write your own physical layer. 
 * 
 * 
 * base class ChannelAccess:
 * - provides access to the channel via the ConnectionManager
 * 
 * base class DeciderToPhyInterface:
 * - interface for the Decider
 */
class BasePhyLayer: public ChannelAccess,
					public DeciderToPhyInterface,
					public MacToPhyInterface {
	
protected:
	
	/** Defines if the physical layer should simulate propagation delay.*/
	bool usePropagationDelay;
	
	/** Defines the strength of the thermal noise.*/
	double thermalNoise;
	
	/** The maximum transmission power a message can be send with */
	double maxTXPower;
	
	/** The sensitivity describes the minimum strength a signal must have to be received.*/
	double sensitivity;
	
	/** 
	 * Channel info keeps track of received AirFrames and provides information about
	 * currently active AirFrames at the channel.
	 */
	ChannelInfo channelInfo;
	
	/** The statemachine storing the current radio state (TX, RX, SLEEP).*/
	Radio radio;
	
	/** Pointer to the decider module. */
	Decider* decider;
	
	/** AnalogueModelList is used to store the AnalogueModels to be used as filters.*/
	typedef std::vector<AnalogueModel*> AnalogueModelList;
	
	/** A list of the analogue models to use.*/
	AnalogueModelList analogueModels;
	
	/** 
	 * ParameterMap is used at initialisation to pass the parameters
	 * to the AnalogueModel and Decider
	 */
	typedef std::map<std::string, cPar> ParameterMap;
	
	/** The id of the in-data gate from the Mac layer */
	int upperGateIn;
	/** The id of the out-data gate to the Mac layer */
	int upperGateOut;
	/** The id of the out-control gate to the Mac layer */
	int upperControlOut;
	/** The id of the in-control gate from the Mac layer */
	int upperControlIn;
	
	/** 
	 * Self message scheduled to the point in time when the 
	 * switching process of the radio is over.
	 */
	cMessage* radioSwitchingOverTimer;
	
	/** 
	 * Self message scheduled to the point in time when the
	 * transmission of an AirFrame is over.
	 */
	cMessage* txOverTimer;
	
	enum AirFrameStates {
		/** 
		 * First receive of AirFrame in simulation, not 
		 * necessary real receiving start.
		 */
		FIRST_RECEIVE = 0,
		/** Start of actual receiving process of the AirFrame. */
		START_RECEIVE,
		/** AirFrame is being received. */
		RECEIVING,
		/** Receiving process over */
		END_RECEIVE
	};
	
public:
	enum BasePhyMessageKinds {
		/** Indicates the end of a send transmission. */
		TX_OVER = 22000,
		/** Indicates the end of a radio switch. */ 
		RADIO_SWITCHING_OVER,
		/** Channel sense control message between Mac and Phy.*/
		CHANNEL_SENSE_REQUEST,
		/** AirFrame kind */
		AIR_FRAME
	};
	
private:
	
	/**
	 * Reads and returns the parameter with the passed named.
	 * If the parameter couldn't be found the value of defaultValue
	 * is returned.
	 * 
	 * parName 		- the name of the ned-parameter
	 * defaultValue - the value to be returned if the parameter
	 * 				  couldn't be found
	 */
	template<class T> T readPar(const char* parName, const T defaultValue);
	
	/**
	 * Utility function. Reads the parameters of a XML element
	 * and stores them in the passed ParameterMap reference.
	 */
	void getParametersFromXML(cXMLElement* xmlData, ParameterMap& outputMap);	
	
	/**
	 * Initializes the AnalogueModels with the data from the
	 * passed XML-config data.
	 */
	void initializeAnalogueModels(cXMLElement* xmlConfig);
	
	/**
	 * Initializes the Decider with the data from the
	 * passed XML-config data.
	 */
	void initializeDecider(cXMLElement* xmlConfig);
	
	/**
	 * Calculates the propagation delay for the passed AirFrame.
	 */
	simtime_t calculatePropagationDelay(AirFrame* frame);
	
protected:
	
	/** 
	 * OMNeT++ initialization function. 
	 * Read simple parameters.
	 * Read and parse xml file for decider and anlogue models
	 * configuration.
	 */
	virtual void initialize(int stage);
	
	/**
	 * OMNeT++ handle message function.
	 * Classify and forward message to subroutines.
	 * - AirFrames from channel
	 * - self scheduled AirFrames
	 * - MacPackets from MAC layer
	 * - ControllMesasges from MAC layer
	 * - self messages like TX_OVER and RADIO_SWITCHED
	 */
	virtual void handleMessage(cMessage* msg);
	
	/**
	 * Returns an instance of the AnalogueModel with the 
	 * specified name. The returned AnalogueModel has to be
	 * generated with the "new" command. The BasePhyLayer
	 * keeps the ownership of the returned AnalogueModel.
	 * 
	 * This method is used by the BasePhyLayer during 
	 * initialisation to load the AnalogueModels which
	 * has been specified in the ned file.
	 * 
	 * This method has to be overriden if you want to be
	 * able to load your own AnalogueModels.
	 */
	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);
	
	/**
	 * Returns an instance of the Decider with the specified 
	 * name. The returned Decider has to be generated with 
	 * the "new" command. The BasePhyLayer keeps the ownership
	 * of the returned Decider.
	 * 
	 * This method is used by the BasePhyLayer during 
	 * initialisation to load the Decider which has been 
	 * specified in the ned file.
	 * 
	 * This method has to be overriden if you want to be
	 * able to load your own Decider.
	 */
	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);
	
	//TODO: Check which handlers should be made virtual
	/**
	 * Handles messages received from the channel (propably AirFrames).
	 */
	void handleAirFrame(cMessage* msg);

	/**
	 * Handles messages received from the upper layer through the
	 * data gate.
	 */
	void handleUpperMessage(cMessage* msg);

	/**
	 * Handles messages received from the upper layer through the
	 * control gate.
	 */
	void handleUpperControlMessage(cMessage* msg);

	/**
	 * Handles self scheduled messages.
	 */
	void handleSelfMessage(cMessage* msg);
	
	/**
	 * Handles reception of a ChannelSenseRequest by forwarding it 
	 * to the decider and scheduling it to the point in time 
	 * returned by the decider.
	 */
	void handleChannelSenseRequest(cMessage* msg);
	
	/**
	 * Handles incoming AirFrames with the state FIRST_RECEIVE.
	 */
	void handleAirFrameFirstReceive(AirFrame* msg);
	
	/**
	 * Handles incoming AirFrames with the state START_RECEIVE.
	 */
	void handleAirFrameStartReceive(AirFrame* msg);
	
	/**
	 * Handles incoming AirFrames with the state RECEIVING.
	 */
	void handleAirFrameReceiving(AirFrame* msg);	
	
	/**
	 * Handles incoming AirFrames with the state END_RECEIVE.
	 */
	void handleAirFrameEndReceive(AirFrame* msg);

	//--Send messages------------------------------

	/**
	 * Sends the passed control message to the upper layer.
	 */
	void sendControlMessageUp(cMessage* msg);

	/**
	 * Sends the passed MacPkt to the upper layer.
	 */
	void sendMacPktUp(MacPkt* pkt);

	/**
	 * Sends the passed AirFrame to the channel
	 */
	void sendMessageDown(AirFrame* pkt);

	/**
	 * Schedule self message to passed point in time.
	 */
	void sendSelfMessage(cMessage* msg, simtime_t time);
	
	/**
	 * This function encapsulates messages from the upper layer into an
	 * AirFrame and sets all necessary attributes.
	 */
	AirFrame *encapsMsg(cMessage *msg);
	
public:
	
	/**
	 * Free the pointer to the decider and the AnalogueModels.
	 */
	virtual ~BasePhyLayer();
	
	//---------MacToPhyInterface implementation-----------
	
	/**
	 * Returns the current state the radio is in. See RadioState
	 * for possible values.
	 * 
	 * This method is mainly used by the mac layer.
	 */
	virtual Radio::RadioState getRadioState();
	
	/**
	 * Tells the BasePhyLayer to switch to the specified
	 * radio state. The switching process can take some time
	 * depending on the specified switching times in the
	 * ned file.
	 */
	virtual simtime_t setRadioState(Radio::RadioState rs);
	
	/**
	 * Returns the current state of the channel. See ChannelState
	 * for details.
	 */
	virtual ChannelState getChannelState();
	
	/**
	 * Service method for the mac layer which creates and initializes
	 * an appropriate Signal with the specified values.
	 * 
	 * Used by the mac layer before sending a mac packet to the phy layer 
	 * to create an initial signal with some predefined values.
	 * 
	 * TODO: write more detailed axplanation as soon as modelation of
	 * 		 Signal is final.
	 */
	virtual Signal createSignal(double txPower,
								double headerBitrate,
								double payloadBitrate,
								simtime_t duration);
	
	//---------DeciderToPhyInterface implementation-----------
	
	/**
	 * @brief Fills the passed AirFrameVector with all AirFrames that intersect 
	 * with the time interval [from, to]
	 */
	virtual void getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out);
	
	/** 
	 * @brief Called by the Decider to send a control message to the MACLayer
	 * 
	 * This function can be used to answer a ChannelSenseRequest to the MACLayer
	 * 
	 */
	virtual void sendControlMsg(cMessage* msg);
	
	/** 
	 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
	 * 
	 * When a packet is completely received and not noise, the Decider
	 * call this function to send the packet together with
	 * the corresponding DeciderResult up to MACLayer
	 * 
	 */
	virtual void sendUp(AirFrame* packet, DeciderResult result);
	
	/**
	 * @brief Returns the current simulation time
	 * 
	 */
	virtual simtime_t getSimTime();

};

#endif /*BASEPHYLAYER_*/
