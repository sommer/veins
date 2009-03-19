#ifndef BASEPHYLAYER_
#define BASEPHYLAYER_

#include "ChannelAccess.h"
#include "DeciderToPhyInterface.h"
#include "MacToPhyInterface.h"

#include "AnalogueModel.h"

#include "Decider.h"
#include "ChannelInfo.h"
#include "BaseWorldUtility.h"

#include "MacPkt_m.h"

#include <cxmlelement.h>
#include <map>
#include <vector>
#include <iostream>


/**
 * @brief The BasePhyLayer represents the physical layer of a nic.
 *
 * The BasePhyLayer is directly connected to the mac layer via
 * OMNeT channels and is able to send messages to other physical
 * layers through sub-classing from ChannelAcces.
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
 * Module initialization:
 * - read ned-parameters and initialize module, Decider and
 *   AnalogueModels.
 *
 * Message handling:
 * - receive messages from mac layer and hand them to the Decider
 *   or directly send them to the channel
 * - receive AirFrames from the channel, hand them to the
 *   AnalogueModels for filtering, simulate delay and transmission
 *   duration, hand it to the Decider for evaluation and send
 *   received packets to the mac layer
 * - keep track of currently active AirFrames on the channel
 *   (see ChannelInfo)
 *
 * The actual evaluation of incoming signals is done by the
 * Decider.
 *
 * base class ChannelAccess:
 * - provides access to the channel via the ConnectionManager
 *
 * base class DeciderToPhyInterface:
 * - interface for the Decider
 *
 *  base class MacToPhyInterface:
 * - interface for the Mac
 *
 * @defgroup phyLayer Physical layer
 * @{
 * 		@defgroup analogueModels Analogue models
 * 		@defgroup decider Decider
 * @}
 * @ingroup phyLayer
 */
class BasePhyLayer: public ChannelAccess,
					public DeciderToPhyInterface,
					public MacToPhyInterface {

protected:

	/** @brief Defines the strength of the thermal noise.*/
	ConstantSimpleConstMapping* thermalNoise;

	/** @brief The maximum transmission power a message can be send with */
	double maxTXPower;

	/** @brief The sensitivity describes the minimum strength a signal must have to be received.*/
	double sensitivity;

	/**
	 * @brief Channel info keeps track of received AirFrames and provides information about
	 * currently active AirFrames at the channel.
	 */
	ChannelInfo channelInfo;

	/** @brief The state machine storing the current radio state (TX, RX, SLEEP).*/
	Radio* radio;

	/** @brief Pointer to the decider module. */
	Decider* decider;

	/** @brief Used to store the AnalogueModels to be used as filters.*/
	typedef std::vector<AnalogueModel*> AnalogueModelList;

	/** @brief List of the analogue models to use.*/
	AnalogueModelList analogueModels;

	/**
	 * @brief Used at initialisation to pass the parameters
	 * to the AnalogueModel and Decider
	 */
	typedef std::map<std::string, cMsgPar> ParameterMap;

	/** @brief The id of the in-data gate from the Mac layer */
	int upperGateIn;
	/** @brief The id of the out-data gate to the Mac layer */
	int upperGateOut;
	/** @brief The id of the out-control gate to the Mac layer */
	int upperControlOut;
	/** @brief The id of the in-control gate from the Mac layer */
	int upperControlIn;

	/**
	 * @brief Self message scheduled to the point in time when the
	 * switching process of the radio is over.
	 */
	cMessage* radioSwitchingOverTimer;

	/**
	 * @brief Self message scheduled to the point in time when the
	 * transmission of an AirFrame is over.
	 */
	cMessage* txOverTimer;

	enum AirFrameStates {
		/** @brief Start of actual receiving process of the AirFrame. */
		START_RECEIVE = 1,
		/** @brief AirFrame is being received. */
		RECEIVING,
		/** @brief Receiving process over */
		END_RECEIVE
	};

	/* @brief Pointer to the World Utility, to obtain some global information*/
	BaseWorldUtility* world;

public:


private:

	/**
	 * @brief Reads and returns the parameter with the passed name.
	 *
	 * If the parameter couldn't be found the value of defaultValue
	 * is returned.
	 *
	 * @param parName 		- the name of the ned-parameter
	 * @param defaultValue 	- the value to be returned if the parameter
	 * 				  		  couldn't be found
	 */
	template<class T> T readPar(const char* parName, const T defaultValue);

	/**
	 * @brief Utility function. Reads the parameters of a XML element
	 * and stores them in the passed ParameterMap reference.
	 */
	void getParametersFromXML(cXMLElement* xmlData, ParameterMap& outputMap);

	/**
	 * @brief Initializes the AnalogueModels with the data from the
	 * passed XML-config data.
	 */
	void initializeAnalogueModels(cXMLElement* xmlConfig);

	/**
	 * @brief Initializes the Decider with the data from the
	 * passed XML-config data.
	 */
	void initializeDecider(cXMLElement* xmlConfig);

protected:

	/**
	 * @brief OMNeT++ initialization function.
	 * Read simple parameters.
	 * Read and parse xml file for decider and analogue models
	 * configuration.
	 */
	virtual void initialize(int stage);

	/**
	 * @brief OMNeT++ handle message function.
	 *
	 * Classify and forward message to subroutines.
	 * - AirFrames from channel
	 * - self scheduled AirFrames
	 * - MacPackets from MAC layer
	 * - ControllMesasges from MAC layer
	 * - self messages like TX_OVER and RADIO_SWITCHED
	 */
	virtual void handleMessage(cMessage* msg);

	/**
	 * @brief Creates and returns an instance of the AnalogueModel with the
	 * specified name.
	 *
	 * The returned AnalogueModel has to be
	 * generated with the "new" command. The BasePhyLayer
	 * keeps the ownership of the returned AnalogueModel.
	 *
	 * This method is used by the BasePhyLayer during
	 * initialisation to load the AnalogueModels which
	 * has been specified in the ned file.
	 *
	 * This method has to be overridden if you want to be
	 * able to load your own AnalogueModels.
	 */
	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

	/**
	 * @brief Creates and returns an instance of the Decider with the specified
	 * name.
	 *
	 * The returned Decider has to be generated with
	 * the "new" command. The BasePhyLayer keeps the ownership
	 * of the returned Decider.
	 *
	 * This method is used by the BasePhyLayer during
	 * initialisation to load the Decider which has been
	 * specified in the ned file.
	 *
	 * This method has to be overridden if you want to be
	 * able to load your own Decider.
	 */
	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

	//TODO: Check which handlers should be made virtual
	/**
	 * @brief Handles messages received from the channel (probably AirFrames).
	 */
	void handleAirFrame(cMessage* msg);

	/**
	 * @brief Handles messages received from the upper layer through the
	 * data gate.
	 */
	void handleUpperMessage(cMessage* msg);

	/**
	 * @brief Handles messages received from the upper layer through the
	 * control gate.
	 */
	void handleUpperControlMessage(cMessage* msg);

	/**
	 * @brief Handles self scheduled messages.
	 */
	void handleSelfMessage(cMessage* msg);

	/**
	 * @brief Handles reception of a ChannelSenseRequest by forwarding it
	 * to the decider and scheduling it to the point in time
	 * returned by the decider.
	 */
	void handleChannelSenseRequest(cMessage* msg);

	/**
	 * @brief Handles incoming AirFrames with the state FIRST_RECEIVE.
	 */
	void handleAirFrameFirstReceive(AirFrame* msg);

	/**
	 * @brief Handles incoming AirFrames with the state START_RECEIVE.
	 */
	void handleAirFrameStartReceive(AirFrame* msg);

	/**
	 * @brief Handles incoming AirFrames with the state RECEIVING.
	 */
	void handleAirFrameReceiving(AirFrame* msg);

	/**
	 * @brief Handles incoming AirFrames with the state END_RECEIVE.
	 */
	void handleAirFrameEndReceive(AirFrame* msg);

	//--Send messages------------------------------

	/**
	 * @brief Sends the passed control message to the upper layer.
	 */
	void sendControlMessageUp(cMessage* msg);

	/**
	 * @brief Sends the passed MacPkt to the upper layer.
	 */
	void sendMacPktUp(cMessage* pkt);

	/**
	 * @brief Sends the passed AirFrame to the channel
	 */
	void sendMessageDown(AirFrame* pkt);

	/**
	 * @brief Schedule self message to passed point in time.
	 */
	void sendSelfMessage(cMessage* msg, simtime_t time);

	/**
	 * @brief This function encapsulates messages from the upper layer into an
	 * AirFrame and sets all necessary attributes.
	 */
	AirFrame *encapsMsg(cPacket *msg);

	/**
	 * @brief Filters the passed Signal by every registered AnalogueModel.
	 */
	void filterSignal(Signal& s);

	/**
	 * @brief Called the moment the simulated switching process of the Radio is finished.
	 *
	 * The Radio is set the new RadioState and the MAC Layer is sent
	 * a confirmation message.
	 */
	void finishRadioSwitching();

public:

	/**
	 * Free the pointer to the decider and the AnalogueModels and the Radio.
	 */
	virtual ~BasePhyLayer();

	//---------MacToPhyInterface implementation-----------

	/**
	 * @brief Returns the current state the radio is in.
	 *
	 * See RadioState for possible values.
	 *
	 * This method is mainly used by the mac layer.
	 */
	virtual int getRadioState();

	/**
	 * @brief Tells the BasePhyLayer to switch to the specified
	 * radio state.
	 *
	 * The switching process can take some time depending on the
	 * specified switching times in the ned file.
	 *
	 * @return	-1: Error code if the Radio is currently switching
	 *			else: switching time from the current RadioState to the new RadioState
	 */
	virtual simtime_t setRadioState(int rs);

	/**
	 * @brief Returns the current state of the channel.
	 *
	 * See ChannelState for details.
	 */
	virtual ChannelState getChannelState();

	//---------DeciderToPhyInterface implementation-----------

	/**
	 * @brief Fills the passed AirFrameVector with all AirFrames that intersect
	 * with the time interval [from, to]
	 */
	virtual void getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out);

	/**
	 * @brief Returns a Mapping which defines the thermal noise in
	 * the passed time frame (in mW).
	 *
	 * The implementing class of this method keeps ownership of the
	 * Mapping.
	 *
	 * This implementation returns a constant mapping with the value
	 * of the "thermalNoise" module parameter
	 *
	 * Override this method if you want to define a more complex
	 * thermal noise.
	 */
	virtual ConstMapping* getThermalNoise(simtime_t from, simtime_t to);

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
	virtual void sendUp(AirFrame* packet, DeciderResult* result);

	/**
	 * @brief Returns the current simulation time
	 */
	virtual simtime_t getSimTime();

	/**
	 * @brief Tells the PhyLayer to cancel a scheduled message (AirFrame or
	 * ControlMessage).
	 *
	 * Used by the Decider if it has to handle an AirFrame or an control message
	 * earlier than it has returned to the PhyLayer the last time the Decider
	 * handled that message.
	 */
	virtual void cancelScheduledMessage(cMessage* msg);

};

#endif /*BASEPHYLAYER_*/
