#ifndef __RADIO_H__
#define __RADIO_H__

#include "mixim.h"
#include "mac.h"

/** List of possible radio states. */
enum RadioState {
	RADIO_LISTEN = 1,
	RADIO_TRANSMIT,
	RADIO_SLEEP
};

/** List of possible radio listen states. */
enum RxState {
	RX_IDLE,
	RX_IN_FRAME
};

/** Base class for radio implemenations. */
class Radio : public MiximBaseModule {
	Module_Class_Members(Radio, MiximBaseModule, 0);

	public:
		virtual void initialize();
		virtual void finish();
		virtual void handleMessage(cMessage *msg);
		virtual ~Radio();

		double getRXThreshold() { return rx_threshold; }
		double getMaxTXPower() { return max_tx_power; }

	private:
		struct NeighbourInfo {
			double power;
			int id;
		};
		
	protected:
		/** Boolean indicating that the MAC layer is doing Low Power Listening.
	
		    The reason for this variable is to be able to keep separate
		    statistics for energy spent on Low Power Listening and regular
		    radio use.
		*/
		bool lp_mode;
		/** Gate index for toAir gate. */
		int gate_to_air,
		/** Gate index for toMac gate. */
			gate_to_mac;
		/** Last time @b record_stats() was called. */
		double last_stat_time;
		/** Time spent in receive mode for Low Power Listening. */
		double stat_time_lb_rx,
		/** Time spent in transmit mode for Low Power Listening. */
			stat_time_lb_tx;
		/** Time spent in sleep mode. */
		double stat_time_sleep,
		/** Time spent in transmit mode. */
			stat_time_tx,
		/** Time spent in receive mode. */
			stat_time_rx;
		/** Time at which the last collision started. */
		double collision_start_time;
		/** @todo seemingly unused. */
		double time_since_clear;
		/** @todo is now unused, although it should be used. See FIXME in
		        neighbourTx. */
		double time_since_listen;
		/** Time spent listening to collisions. */
		double stat_time_collision;
		/** Current state of the radio. */
		RadioState radio_state;
		/** Current reception state of the radio. */
		RxState rx_state;
		/** List of nodes currently transmitting in the neighbourhood. */
		vector <NeighbourInfo> transmitters;
		/** Neighbour with signal above SNR threshold. */
		int current_source;
		/** Power of @b current_source. */
		double current_power;
		/** Total noise level. */
		double sum_noise;
		/** Is the radio currently in a collision state. */
		bool inCollision;
		/** Message currently being received. */
		Packet *scheduled_rx;
		/** Duplicate of the message currently being received, to be sent when
		    the header has been completely received. */
		Packet *scheduled_hdr_rx;
		/** SNR threshold (in dB). */
		static double snr_threshold;
		/** RX threshold (in dBm). */
		static double rx_threshold;
		/** The ambient noise (in dBm). */
		static double noise_floor;
		/** The threshold to detect transmissions (in dBm). */
		static double detect_threshold;
		/** Maximum TX power (in dBm). */
		static double max_tx_power;

		/** Send a @b Packet.
		    @param msg The message to be sent.
		
		    Before calling this function, the current state must be 
			@c RADIO_TRANSMIT. This should be achieved by sending this module a
			@c SET_TRANSMIT message.
		*/
		virtual void tx(Packet * msg);
		/** Switch the radio to transmit mode. */
		virtual void setTransmit();
		/** Switch the radio to receive mode. */
		virtual void setListen();
		/** Switch the radio to sleep mode. */
		virtual void setSleep();
		/** Handle a neighbour starting to send.
		    @param neigh The ID of the neighbour.
			@param power The received power in mW.
		
		    Checks for collisions and sends RSSI value up the stack.
		*/
		virtual void neighbourStart(int neigh, double power);
		/** Handle a neighbour that stops sending.
		    @param neigh The ID of the neighbour.
			@param power The received power in mW.
		*/
		virtual void neighbourStop(int neigh, double power);
		/** Handle packet reception start.
		    @param msg The @b Packet that is coming in.
		
		    This function gets called after the preamble has been sent. It
		    schedules the @c RX_HDR and @c RX_END messages to be sent to the
			MAC layer.
		*/
		virtual void neighbourTx(Packet * msg);
		/** Find the loudest neighbour that was considered noise up to now, and check if it is now a signal. */
		virtual void checkLoudestNoise();
		/** Handle end of a packet.
		    @param msg The @b Packet that has been received.
		
		    Called when the @b Packet has been completely received.
		*/
		virtual void rxEnd(Packet * msg);
		/** Handle header of a packet.
		    @param msg The @b Packet which header has been received.
		
		    Called when the @b Packet header has been completely received. This
		    allows protocols like LMAC to switch to sleep mode when they have
		    received the header and determined that they are not the intended
		    recipient.
		*/
		virtual void rxHdr(Packet * msg);
		/** Record the time since the last call to this function in one of the
		        statistics variables.
		
		    The variable to be used is determined from the current state of
		    @b lp_mode and @b radio_state. Therefore this function should
			always be called @b before changing either of these variables.
		*/
		virtual void record_stats();
		/** Clean up the state variables set in @b neighbourTx() for a
			    cancelled reception.
		
		    Called when a collision occurs, the radio gets set to transmit or
		    sleep mode, or the other sender aborted its transmission.
		*/
		virtual void cancelRx();
		/** Send the current RSSI value up the stack. */
		virtual void sendRssi();

	private:
		static bool parametersInitialised;
};

#endif

