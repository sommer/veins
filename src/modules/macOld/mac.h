#ifndef __MAC_H__
#define __MAC_H__

#include "message.h"

#include "BaseMacLayer.h"
#include "BasePhyLayer.h"
#include "Timer.h"
#include <assert.h>

#define BYTE_TIME		(8.0 / radio->par("bitrate").doubleValue())
#define PREAMBLE_TIME                (4.0 * BYTE_TIME)
#define EXTRA_TRANSMIT_TIME	(0.5 * BYTE_TIME)

/** Maximum number of MAC layer timers. */
#define TIMERS		5
#define OFF_TIME	9
#define ON_TIME		1

/* FIXME: do proper rng choosing */
#define RNG_MAC 0

/** Base class for MAC protocol implementations. */
class EyesMacLayer: public BaseMacLayer, Timer {
	friend class MacPacket;
	//Module_Class_Members(EyesMacLayer, BaseMacLayer, 0);
		void internal_printf(const char* fmt, va_list list, bool newline);

	protected:
		/** Print a message with time stamp.
		    @param fmt printf(3) format string.
	
		    The message is printed preceeded by the time and the module that
		    includes this function and followed by a newline.
			*/
		void printf(const char *fmt, ...);
		/** Print a message without a trailing newline.
		    @param fmt printf(3) format string.

		    Same as @b printf(), but with a trailing space instead of a newline.
		    This allows printinf of a message in several steps. See also @b
		    printf_clr().
		*/
		void printf_nr(const char *fmt, ...);
		/** Continue a message started with @b printf_nr().
		    @param fmt printf(3) format string.
		*/
		void printf_clr(const char *fmt, ...);
		/** Print a message without preceding time and node information. */
		void printfNoInfo(const char *fmt, ...);
		/** Print a message without preceding time and node information or trailing newline. */
		void printfNoInfo_nr(const char *fmt, ...);

		/** Get a string-type implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		std::string getStringParameter(const char *parameter, std::string defaultValue = "");
		/** Get a time-value implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		double getTimeParameter(const char *parameter, double defaultValue = 0.0);
		/** Get a double-type implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		double getDoubleParameter(const char *parameter, double defaultValue = 0.0);
		/** Get a long-type implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		long getLongParameter(const char *parameter, long defaultValue = 0);
		/** Get a bool-type implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		bool getBoolParameter(const char *parameter, bool defaultValue = false);

		/** Get the current time as indicated by the node's crystal clock.
		    @return The current time in clock ticks.

		    Most nodes feature a 32KHz crystal. This function will retrieve the
		    current value of the simulated clock.
		*/
		unsigned short getCurrentTime(void);

		
		/** Radio time spent for transmit overhead.*/
		double stat_time_tx_overhead;
		/** Radio time spent for reception overhead.*/
		double stat_time_rx_overhead;
		/** Radio time spent for overhearing.*/
		double stat_time_rx_overhear;
		/** Radio time spent for transmitting data.*/
		double stat_time_tx_data;
		/** Radio time spent for receiving data.*/
		double stat_time_rx_data;
	
		/** Set radio to listen state.
	
		    Funtion should be used by MAC layer implementations to set the
		    radio to receive.
		*/
		void setRadioListen();
		/** Set radio to transmit state.
	
		    Funtion should be used by MAC layer implementations to set the
		    radio to transmit.
		*/
		void setRadioTransmit();
		/** Set radio to sleep state.
	
		    Funtion should be used by MAC layer implementations to set the
		    radio to sleep.
		*/
		void setRadioSleep();

		/** Get the default header length for this MAC layer.
		    @return The default header length in bytes.

		    Should be overriden by implementations.
		*/
		virtual int headerLength();

	public:	
		virtual void initialize(int stage);
		virtual void initialize() {};
		virtual int numInitStages() const {
		  return 4;
		}
		virtual void finish();

	protected:
		/** Send a data packet arriving from the network layer.
		    @param msg The @b Packet to send.
			
		    This function gets called after this base class receives a @b
		    cMessage with kind TX. It should be implemented by the MAC layer to
			receive the packets from the network layer.
		*/
		virtual void txPacket(MacPacket* msg){assert(0);}
	
		/** Transmitted packets count. */
		unsigned stat_tx,
		/** Received packets count. */
			stat_rx,
		/** Count of packets that were to be sent, but were dropped. */
			stat_tx_drop;

		/** Pointer to the enclosing @b Node. */
		BaseModule* node;
		/** Pointer to the @b Radio in this @b Node container. */
		//Radio* radio;
		BasePhyLayer* radio;
	
		/** Get the current RSSI value.
		    @return The current RSSI value, in the range 0.0 to 1.0.
		*/
		double getRssi();
		/** Get current receiving state.
		    @return A boolean indication whether the radio is currently
		        receving data over the radio (@c true).
		*/
		bool isReceiving();
		/** Set a timer.
		    @param ticks The number of ticks until the timer should fire.
			@param which Which of the @b TIMERS timers should be set
		        (timer 0 by default).
		*/
		void setTimeout(int ticks, int which=0);
		/** Cancel a timer.
		    @param which Which of the @b TIMERS to cancel (0 by default).
		*/
		void cancelTimeout(int which = 0) {cancelTimer(which);}
		/** Register a @b MacPacket as reception overhead.
		    @param p The @b MacPacket to register.
		*/
		void reg_rx_overhead(MacPacket * p);
		/** Register a @b MacPacket as transmission overhead.
		    @param p The @b MacPacket to register.
		*/
		void reg_tx_overhead(MacPacket * p);
		/** Register a @b MacPacket as received data.
		    @param p The @b MacPacket to register.
		*/
		void reg_rx_data(MacPacket * p);
		/** Register a @b MacPacket as transmitted data.
		    @param p The @b MacPacket to register.
		*/
		void reg_tx_data(MacPacket * p);
		/** Register a @b MacPacket as overheared data.
		    @param p The @b MacPacket to register.
		*/
		void reg_rx_overhear(MacPacket * p);
		/** Send a @b MacPacket to the radio.
		    @param msg The @b MacPacket to be sent.
		
		    Note that the radio should have already been set to transmit mode.
		*/
		void startTransmit(MacPacket * msg);
		/** Send a @b MacPacket to the routing layer.
		    @param msg The @b MacPacket to send to the routing layer.
		*/
		void rxPacket(MacPacket * msg);
		/** Notify the routing layer that a @b MacPacket is sent.
		    @param msg The @b MacPacket to notify the routing layer about.
		*/
		void txPacketDone(MacPacket *msg);
		void txPacketFail(MacPacket *msg);
	
		/** Timer fire.
		
		    This function is called when timer 0 fires, and the
			@b timeout(int which) function is NOT overriden.
		*/
		virtual void timeout();
		/** Timer fire.
		    @param which The timer that fired.
		
		    When the timeout set for a timer has passed, this function is
		    called. MAC implementations should override this (or the
			@b timeout() function) to receive notification of timer fire events.
		*/
		virtual void timeout(int which);
		/** Notification of reception of the end of the header of a packet.
		    @param msg The @b MacPacket that is currently being received.
		
		    If the MAC implementation needs to perform an action based soley on
		    the information contained in the packet header, this function can
		    be overridden to get notification of the reception of the header.
		*/
		virtual void rxHeader(MacPacket * msg) {};
		/** Notification of receive failure.
		
		    Called when reception of a MacPacket failed. This means that there was
			some sort of collision. @b rxStarted() will have been called
		    previously.
		*/
		virtual void rxFailed(){assert(0);}
		/** Notification of reception start.
		
		    Called when the radio has picked up a preamble.
		*/
		virtual void rxStarted(){assert(0);}
		/** Notification of transmition completion.
		
			Called when the radio has finished transmitting a packet.
		*/
		virtual void transmitDone(){assert(0);}
		/** Notification of packet reception.
		
		    Called when the radio has received a @b MacPacket.
		*/
		virtual void rxFrame(MacPacket * msg){assert(0);}

		/** Simulate processor cycles use.
		    @param cycles The number of cycles to be used in simulation.

		    This routine can be used to simulate that code needs cpu-cycles to
		    execute. The cycles used will then be taken into account in
		    calculating the amount of power used by the processor.
	
		    Note that this simply calls the eatCycles routine in @b Node.
		*/
		//virtual void eatCycles(unsigned cycles);

		/** Calculate the time required to send a full packet.
		    @param x The size of the packet in bytes.
		    @return The time in seconds required to send an @a x byte packet.
		
		    Sums the @b preamble_time, the time for the start byte, the CRC and
		    the data bytes in the packet to calculate the total time required
		    to send a packet of @a x bytes.
		*/
		double frameTotalTime(double x) const {
			return preamble_time+ x/radio->par("bitrate").doubleValue();
		}
		
		/** Get a number from the sift distribution.
		    @param low The lowest number in the sift distribution.
		    @param high The highest number in the sift distribution.
		*/
		static int siftSlot(int low, int high);

		typedef std::pair<const char*, std::string> ParameterPair;
		static std::vector<ParameterPair> parameterList;
		virtual std::string getParameter(const char *parameter);

		virtual ~EyesMacLayer();

		/* FIXME: This assumes that parent is a nic */
		int macid() {return getParentModule()->getId();}

		/** Current RSSI measurement. */
		double rssi;
		/** Time in seconds required for sending the preamble. */
		double preamble_time;
		/** Boolean indicating whether to Low Power Listening.
	
		    Low Power Listening is impelemented below the actual MAC
		    implementation.
		*/
		bool lpl;
		/** Sleep time for Low Power Listening. */
		int off_time;
		/** Wake time for Low Power Listening. */
		int on_time;
		/** Force Low Power Listening to sleep.
	
		    After receiving a packet, the radio of the transmitter will still
		    be in transmit mode for a short time. Therefore, we have to force
		    the Low Power Listening sampling to sleep for a short amount of
		    time, to prevent unnecessary listening triggered by this last part
		    of the transmission. */
		bool force_sleep;
		/** Boolean to keep track of whether the radio is sending. */
		bool send_state;
	
		/** Indicates whether parameters have been read from ini-file. */
		static bool parametersInitialised, parametersPrinted;
		/** The number of node to tune the sift distribution for. */
		static int siftNodes;

		/** Sift distribution data. */
		struct SiftData {
			int slots;
			double *chances;
		};
		static SiftData *siftDistribution;
		static int maxSiftDistributions;
		static int siftDistributions;

		enum {SENSE=1, FRAME, SLEEP, PREAMBLE_DETECT}
		/** Current receiving state of the radio. */
			recv_state;
		
		/** Timeout function of the Low Power Listening timer. */
		void internalTimeout();
		/** Set the Low Power Listening timer.
		    @param ticks The number of clock ticks before the timer should
		        fire.
		*/
		void setInternalTimeout(int ticks);
		/** Cancel the Low Power Listening timer. */
		void cancelInternalTimeout();
		/** Handle reception of a message.
		    @param msg The @b MacPacket that was received.
		
		    This function calls @b rxFrame to notify a MAC implementation
		    that a @b MacPacket has been received.
		*/
		void rx(MacPacket * msg);
		/** Handle the end of transmission notification from the radio.
		
		    This function calls @b transmitDone to notify a MAC implementation
			that the transmission has finished.
		*/
		void txDone();
		/** Handle the start of reception notification from the radio.
		
		    This function calls @b rxStarted to notify a MAC implementation
			that reception of a packet has started.
		*/
		void rxStart();
		/** Handle the reception failed notification from the radio.
		
		    This function calls @b rxFailed to notify a MAC implementation
			that reception of a packet has failed.
		*/
		void rxFail();
		/** Tell the radio to go to sleep.

		    The Low Power Listening needs to be able to tell the radio to go
		    to sleep without setting the radio state kept in the MAC layer to
		    @c SLEEP. Therefore, this internal function actually puts the
		    radio to sleep, while the @b setRadioSleep function doesn't
		    actually put the radio to sleep itself, but calls this function
		    to do it for it.
		*/
		void setRadioSleepInternal();
		/** Fires on expiration of a timer.
		    Fires after a call to @b setTimer().
			@param index Timer number that fired. Will be between 0 and the value given to @b initTimers()
		*/	

		virtual void handleTimer(unsigned int count);
		/** Fires on incoming data messages.
			@param msg Incoming message.
		*/	

		void handleUpperMsg(cMessage * msg);
		void handleLowerMsg(cMessage * msg);
		void handleLowerControl(cMessage * msg);

		double getThenTime(unsigned short ticks);

		/* timer stuff */

		/** Initialise a set of timers for this protocol layer
		    @param count Number of timers used by this layer
		 */	
		void initTimers(unsigned int count){Timer::init(this);allocateTimers(count);}
};

#endif

