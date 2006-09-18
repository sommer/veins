#ifndef __MAC_H__
#define __MAC_H__

#include "software.h"
#include "node.h"
#include "radio.h"
#include "message.h"

/** Maximum number of MAC layer timers. */
#define TIMERS		5
#define OFF_TIME	9
#define ON_TIME		1

/** MAC layer forcing states for Guesswork routing protocol. */
enum ForceState {FORCE_NORMAL, FORCE_NOSLEEP, FORCE_SLEEP_WANTED};

class Radio;

/** Base class for MAC protocol implementations. */
class Mac: public MiximSoftwareModule {
	Module_Class_Members(Mac, MiximSoftwareModule, 0);

	public:
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
	
		virtual void initialize();
		virtual void finish();
		
		/** Get the default header length for this MAC layer.
		    @return The default header length in bytes.

		    Should be overriden by implementations.
		*/
		virtual int headerLength();

		/** Send a data packet arriving from the network layer.
		    @param msg The @b Packet to send.
			
		    This function gets called after this base class receives a @b
		    cMessage with kind TX. It should be implemented by the MAC layer to
			receive the packets from the network layer.
		*/
		virtual void txPacket(Packet* msg) = 0;
	
	protected:
		/** Transmitted packets count. */
		unsigned stat_tx,
		/** Received packets count. */
			stat_rx,
		/** Count of packets that were to be sent, but were dropped. */
			stat_tx_drop;
		/** Pointer to the enclosing @b Node. */
		Node* node;
		/** Pointer to the @b Radio in this @b Node container. */
		Radio* radio;
	
		/** Get the current RSSI value.
		    @return The current RSSI value, in the range 0.0 to 1.0.
		*/
		double getRssi();
		/** Get current receiving state.
		    @return A boolean indication whether the radio is currently
		        receving data over the radio (@c true).
		*/
		bool isReceiving();
		/** Get transmit preference.
		    @return A boolean indicating whether we prefer transmitting over
			    receiving if we have the choice.
		*/
		inline bool txPreferred() { return tx_preferred; }
		/** Get the current force state.
			@return The current force state.
		*/
		const ForceState getForce();
		/** Set a timer.
		    @param ticks The number of ticks until the timer should fire.
			@param which Which of the @b TIMERS timers should be set
		        (timer 0 by default).
		*/
		void setTimeout(int ticks, int which=0);
		/** Cancel a timer.
		    @param which Which of the @b TIMERS to cancel (0 by default).
		*/
		void cancelTimeout(int which = 0);
		/** Register a @b Packet as reception overhead.
		    @param p The @b Packet to register.
		*/
		void reg_rx_overhead(Packet * p);
		/** Register a @b Packet as transmission overhead.
		    @param p The @b Packet to register.
		*/
		void reg_tx_overhead(Packet * p);
		/** Register a @b Packet as received data.
		    @param p The @b Packet to register.
		*/
		void reg_rx_data(Packet * p);
		/** Register a @b Packet as transmitted data.
		    @param p The @b Packet to register.
		*/
		void reg_tx_data(Packet * p);
		/** Register a @b Packet as overheared data.
		    @param p The @b Packet to register.
		*/
		void reg_rx_overhear(Packet * p);
		/** Send a @b Packet to the radio.
		    @param msg The @b Packet to be sent.
		
		    Note that the radio should have already been set to transmit mode.
		*/
		void startTransmit(Packet * msg);
		/** Send a @b Packet to the routing layer.
		    @param msg The @b Packet to send to the routing layer.
		*/
		void rxPacket(Packet * msg);
		/** Notify the routing layer that a @b Packet is sent.
		    @param msg The @b Packet to notify the routing layer about.
			
		    The @b Packet should have its @a kind field set to indicate success
		    (@c TX_DONE) or failure (@c TX_FAILED).
		*/
		void txPacketDone(Packet *msg);
	
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
		    @param msg The @b Packet that is currently being received.
		
		    If the MAC implementation needs to perform an action based soley on
		    the information contained in the packet header, this function can
		    be overridden to get notification of the reception of the header.
		*/
		virtual void rxHeader(Packet * msg) {};
		/** End the forcing of the MAC layer.
		
		    This function can be overridden to get notification of the end of
			forcing of the radio.
		*/
		virtual void endForce(){};
		/** Notification of receive failure.
		
		    Called when reception of a packet failed. This means that there was
			some sort of collision. @b rxStarted() will have been called
		    previously.
		*/
		virtual void rxFailed() = 0;
		/** Notification of reception start.
		
		    Called when the radio has picked up a preamble.
		*/
		virtual void rxStarted() = 0;
		/** Notification of transmition completion.
		
			Called when the radio has finished transmitting a packet.
		*/
		virtual void transmitDone() = 0;
		/** Notification of packet reception.
		
		    Called when the radio has received a @b Packet.
		*/
		virtual void rxFrame(Packet * msg)=0;

		/** Notification of routing layer request for a forceTime period
		 */
		virtual void forceRequest();

		/** grants a force request to the routing layer */
		
		void forceGrant(bool success);

		/** Simulate processor cycles use.
		    @param cycles The number of cycles to be used in simulation.

		    This routine can be used to simulate that code needs cpu-cycles to
		    execute. The cycles used will then be taken into account in
		    calculating the amount of power used by the processor.
	
		    Note that this simply calls the eatCycles routine in @b Node.
		*/
		virtual void eatCycles(unsigned cycles);

		/** Calculate the time required to send a full packet.
		    @param x The size of the packet in bytes.
		    @return The time in seconds required to send an @a x byte packet.
		
		    Sums the @b preamble_time, the time for the start byte, the CRC and
		    the data bytes in the packet to calculate the total time required
		    to send a packet of @a x bytes.
		*/
		double frameTotalTime(double x) const {
			return preamble_time+FRAME_DATA_TIME(x);
		}

		
		/** Get a number from the sift distribution.
		    @param low The lowest number in the sift distribution.
		    @param high The highest number in the sift distribution.
		*/
		static int siftSlot(int low, int high);

		typedef pair<const char*, std::string> ParameterPair;
		static vector<ParameterPair> parameterList;
		virtual std::string getParameter(const char *parameter);

		virtual ~Mac();

	private:
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
		/** Boolean indicating whether we currently prefer sending over
		    receiving. */
		bool tx_preferred;
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


		enum {SENSE, FRAME, SLEEP, PREAMBLE_DETECT}
		/** Current receiving state of the radio. */
			recv_state;
		
		/** Force state for the Gueswork protocol. */
		ForceState force;
	
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
		    @param msg The @b Packet that was received.
		
		    This function calls @b rxFrame to notify a MAC implementation
		    that a @b Packet has been received.
		*/
		void rx(Packet * msg);
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
		virtual void handleEvent(cMessage *msg);
};

#endif

