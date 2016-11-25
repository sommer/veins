#ifndef BASEPHYLAYER_
#define BASEPHYLAYER_

#include <map>
#include <vector>
#include <string>
#include <memory>

#include "veins/base/utils/MiXiMDefs.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/base/phyLayer/DeciderToPhyInterface.h"
#include "veins/base/phyLayer/MacToPhyInterface.h"
#include "veins/base/phyLayer/Antenna.h"

#include "veins/base/phyLayer/ChannelInfo.h"

class AnalogueModel;
class Decider;
class BaseWorldUtility;
//class omnetpp::cXMLElement;

using Veins::AirFrame;
using Veins::ChannelAccess;
using Veins::Radio;

/**
 * @brief The BasePhyLayer represents the physical layer of a nic.
 *
 * The BasePhyLayer is directly connected to the mac layer via
 * OMNeT channels and is able to send messages to other physical
 * layers through sub-classing from ChannelAcces.
 *
 * The BasePhyLayer encapsulates three sub modules.
 * The AnalogueModels, which are responsible for simulating
 * the attenuation of received signals and the Decider which
 * provides the main functionality of the physical layer like
 * signal classification (noise or not noise) and demodulation
 * (calculating transmission errors). Furthermore, the Antenna
 * used for gain calculation is managed by the BasePhyLayer.
 *
 * The BasePhyLayer itself is responsible for the OMNeT
 * depended parts of the physical layer which are the following:
 *
 * Module initialization:
 * - read ned-parameters and initialize module, Decider,
 * AnalogueModels and Antenna.
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
 * base class MacToPhyInterface:
 * - interface for the Mac
 *
 * @ingroup phyLayer
 * @ingroup baseModules
 */
class MIXIM_API BasePhyLayer: public ChannelAccess,
                              public DeciderToPhyInterface,
                              public MacToPhyInterface {

protected:

	enum ProtocolIds {
		GENERIC = 0,
	};

	int protocolId;

	/** @brief Defines the scheduling priority of AirFrames.
	 *
	 * AirFrames use a slightly higher priority than normal to ensure
	 * channel consistency. This means that before anything else happens
	 * at a time point t every AirFrame which ended at t has been removed and
	 * every AirFrame started at t has been added to the channel.
	 *
	 * An example where this matters is a ChannelSenseRequest which ends at
	 * the same time as an AirFrame starts (or ends). Depending on which message
	 * is handled first the result of ChannelSenseRequest would differ.
	 */
	static short airFramePriority() {
		return 10;
	}

	/** @brief Defines the strength of the thermal noise.*/
	ConstantSimpleConstMapping* thermalNoise;

	/** @brief The sensitivity describes the minimum strength a signal must have to be received.*/
	double sensitivity;

	/** @brief Stores if tracking of statistics (esp. cOutvectors) is enabled.*/
	bool recordStats;

	/**
	 * @brief Channel info keeps track of received AirFrames and provides information about
	 * currently active AirFrames at the channel.
	 */
	ChannelInfo channelInfo;

	/** @brief The state machine storing the current radio state (TX, RX, SLEEP).*/
	Radio* radio;

	/**
	 * @brief Shared pointer to the Antenna used for this node. It needs to be a shared
	 * pointer as a possible receiver could still need the antenna for gain calculation
	 * even if this node has already disappeared. The antenna will be deleted when nothing
	 * is pointing to it any more.
	 */
	std::shared_ptr<Antenna> antenna;

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
	int upperLayerIn;
	/** @brief The id of the out-data gate to the Mac layer */
	int upperLayerOut;
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

	/** @brief The states of the receiving process for AirFrames.*/
	enum AirFrameStates {
		/** @brief Start of actual receiving process of the AirFrame. */
		START_RECEIVE = 1,
		/** @brief AirFrame is being received. */
		RECEIVING,
		/** @brief Receiving process over */
		END_RECEIVE
	};

	/** @brief Stores the length of the phy header in bits. */
	int headerLength;

	/** @brief Pointer to the World Utility, to obtain some global information*/
	BaseWorldUtility* world;

public:


private:

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

	/**
	 * @brief Initializes the Antenna with the data from the
	 * passed XML-config data.
	 */
	void initializeAntenna(cXMLElement* xmlConfig);

protected:

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
	 * @brief OMNeT++ initialization function.
	 *
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
	 * @brief Initializes and returns the radio class to use.
	 *
	 * Can be overridden by sub-classing phy layers to use their
	 * own Radio implementations.
	 */
	virtual Radio* initializeRadio();

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

	/**
	 * @brief Creates and returns an instance of the Antenna with the specified name
	 * as a shared pointer.
	 *
	 * This method is called during initialization to load the Antenna specified. If no
	 * special antenna has been specified, an object of the base Antenna class is
	 * instantiated, representing an isotropic antenna.
	 */
	virtual std::shared_ptr<Antenna> getAntennaFromName(std::string name, ParameterMap& params);

	/**
     * @brief Creates and returns an instance of the SampledAntenna1D class as a
     * shared pointer.
     *
     * The given parameters (i.e. samples and optional randomness parameters) are
     * evaluated and given to the antenna's constructor.
     */
	virtual std::shared_ptr<Antenna> initializeSampledAntenna1D(ParameterMap& params);


	/**
	 * @name Handle Messages
	 **/
	/*@{ */
	/**
	 * @brief Handles messages received from the channel (probably AirFrames).
	 */
	virtual void handleAirFrame(AirFrame* frame);

	/**
	 * @brief Handles messages received from the upper layer through the
	 * data gate. Also, attaches a POA object to the created Airframe containing
	 * information needed by the receiver for antenna gain calculation.
	 */
	virtual void handleUpperMessage(cMessage* msg);

	/**
	 * @brief Handles messages received from the upper layer through the
	 * control gate.
	 */
	virtual void handleUpperControlMessage(cMessage* msg);

	/**
	 * @brief Handles self scheduled messages.
	 */
	virtual void handleSelfMessage(cMessage* msg);

	/**
	 * @brief Handles reception of a ChannelSenseRequest by forwarding it
	 * to the decider and scheduling it to the point in time
	 * returned by the decider.
	 */
	virtual void handleChannelSenseRequest(cMessage* msg);

	/**
	 * @brief Handles incoming AirFrames with the state FIRST_RECEIVE.
	 */
	void handleAirFrameFirstReceive(AirFrame* msg);

	/**
	 * @brief Handles incoming AirFrames with the state START_RECEIVE.
	 */
	virtual void handleAirFrameStartReceive(AirFrame* msg);

	/**
	 * @brief Handles incoming AirFrames with the state RECEIVING.
	 */
	virtual void handleAirFrameReceiving(AirFrame* msg);

	/**
	 * @brief Handles incoming AirFrames with the state END_RECEIVE.
	 */
	virtual void handleAirFrameEndReceive(AirFrame* msg);

	/*@}*/

	/**
	 * @name Send Messages
	 **/
	/*@{ */

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
	void sendSelfMessage(cMessage* msg, simtime_t_cref time);

	/*@}*/

	/**
	 * @brief This function encapsulates messages from the upper layer into an
	 * AirFrame and sets all necessary attributes.
	 */
	virtual AirFrame *encapsMsg(cPacket *msg);

	/**
	 * @brief Filters the passed AirFrame's Signal by every registered AnalogueModel.
	 * Moreover, the antenna gains are calculated and added to the signal.
	 */
	virtual void filterSignal(AirFrame *frame);

	/**
	 * @brief Called the moment the simulated switching process of the Radio is finished.
	 *
	 * The Radio is set the new RadioState and the MAC Layer is sent
	 * a confirmation message.
	 */
	virtual void finishRadioSwitching();

	/**
	 * @brief Returns the identifier of the protocol this phy uses to send
	 * messages.
	 *
	 * @return An integer representing the identifier of the used protocol.
	 */
	virtual int myProtocolId() { return protocolId; }

	/**
	 * @brief Returns true if the protocol with the passed identifier is
	 * decodeable by the decider.
	 *
	 * If the protocol with the passed id is not understood by this phy layers
	 * decider the according AirFrame is not passed to the it but only is added
	 * to channel info to be available as interference to the decider.
	 *
	 * Default implementation checks only if the passed id is the same as the
	 * one returned by "myProtocolId()".
	 *
	 * @param id The identifier of the protocol of an AirFrame.
	 * @return Returns true if the passed protocol id is supported by this phy-
	 * layer.
	 */
	virtual bool isKnownProtocolId(int id) { return id == myProtocolId(); }

public:
	BasePhyLayer();

	/**
	 * Free the pointer to the decider and the AnalogueModels and the Radio.
	 */
	virtual ~BasePhyLayer();

	/** @brief Only calls the deciders finish method.*/
	virtual void finish();

	//---------MacToPhyInterface implementation-----------
	/**
	 * @name MacToPhyInterface implementation
	 * @brief These methods implement the MacToPhyInterface.
	 **/
	/*@{ */

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

	/**
	 * @brief Returns the length of the phy header in bits.
	 *
	 * Since the MAC layer has to create the signal for
	 * a transmission it has to know the total length of
	 * the packet and therefore needs the length of the
	 * phy header.
	 */
	virtual int getPhyHeaderLength();

	/** @brief Sets the channel currently used by the radio. */
	virtual void setCurrentRadioChannel(int newRadioChannel);

	/** @brief Returns the channel currently used by the radio. */
	virtual int getCurrentRadioChannel();

	/** @brief Returns the number of channels available on this radio. */
	virtual int getNbRadioChannels();

	/*@}*/

	//---------DeciderToPhyInterface implementation-----------
	/**
	 * @name DeciderToPhyInterface implementation
	 * @brief These methods implement the DeciderToPhyInterface.
	 **/
	/*@{ */

	/**
	 * @brief Fills the passed AirFrameVector with all AirFrames that intersect
	 * with the time interval [from, to]
	 */
	virtual void getChannelInfo(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out);

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
	virtual ConstMapping* getThermalNoise(simtime_t_cref from, simtime_t_cref to);

	/**
	 * @brief Called by the Decider to send a control message to the MACLayer
	 *
	 * This function can be used to answer a ChannelSenseRequest to the MACLayer
	 *
	 */
	virtual void sendControlMsgToMac(cMessage* msg);

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
	 * Used by the Decider if it doesn't need to handle an AirFrame or
	 * ControlMessage again anymore.
	 */
	virtual void cancelScheduledMessage(cMessage* msg);

	/**
	 * @brief Tells the PhyLayer to reschedule a message (AirFrame or
	 * ControlMessage).
	 *
	 * Used by the Decider if it has to handle an AirFrame or an control message
	 * earlier than it has returned to the PhyLayer the last time the Decider
	 * handled that message.
	 */
	virtual void rescheduleMessage(cMessage* msg, simtime_t_cref t);

	/**
	 * @brief Does nothing. For an actual power supporting
	 * phy see "PhyLayerBattery".
	 */
	virtual void drawCurrent(double amount, int activity);

	/**
	 * @brief Returns a pointer to the simulations world-utility-module.
	 */
	virtual BaseWorldUtility* getWorldUtility();

	/**
	 * @brief Records a double into the scalar result file.
	 *
	 * Implements the method from DeciderToPhyInterface, method-calls are forwarded
	 * to OMNeT-method 'recordScalar'.
	 */
	void recordScalar(const char *name, double value, const char *unit=NULL);

	/*@}*/

	/**
	 * @brief Attaches a "control info" (PhyToMac) structure (object) to the message pMsg.
	 *
	 * This is most useful when passing packets between protocol layers
	 * of a protocol stack, the control info will contain the decider result.
	 *
	 * The "control info" object will be deleted when the message is deleted.
	 * Only one "control info" structure can be attached (the second
	 * setL3ToL2ControlInfo() call throws an error).
	 *
	 * @param pMsg		The message where the "control info" shall be attached.
	 * @param pSrcAddr	The MAC address of the message receiver.
	 */
	 virtual cObject *const setUpControlInfo(cMessage *const pMsg, DeciderResult *const pDeciderResult);
};

#endif /*BASEPHYLAYER_*/
