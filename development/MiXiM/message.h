#ifndef __PACKETMESSAGE_H__
#define __PACKETMESSAGE_H__

#include <omnetpp.h>
//#include "mixim.h"

/** Enum to hold all the known message types. */
enum {
	TX = 1,
	RX,
	RX_END,
	RX_FAIL,
	RX_START,
	TX_DONE,
	TX_FAILED,
	PREFER_TX,
	SET_TRANSMIT,
	SET_LISTEN,
	SET_SLEEP,
	NEIGHBOUR_START,
	NEIGHBOUR_STOP,
	NEIGHBOUR_TX,
	TX_START,
	TX_STOP,
	SET_RSSI,
	BECOME_ACTIVE,
	BECOME_IDLE,
	GENERATE,
	TX_EXTRA,
	IDLE_PATTERNS,
	ACTIVE_PATTERNS,
	
	SET_NORMAL_POWER,
	SET_LOW_POWER,
	RX_COLLISION,
	RX_HDR,
	SEND_LATENCY,
	
	/* these two are for the "force" extensions needed for Guesswork */
	FORCE_AWAKE_REQ,
	FORCE_AWAKE_GRANT,
	FORCE_END,

	/* G-MAC <-> routing interface. Message types for lost contention, for
	   no ACK received and for retries that were skipped. */
	TX_CONTEND_LOSE,
	TX_NO_ACK,
	TX_RETRY_SKIPPED
};

/** Broadcast address */
#define BROADCAST (-1)

/** Enum that enumerates all layers that can store data in a packet */
enum DataType {
	MAC_DATA,
	ROUTING_DATA,
	APPLICATION_DATA,
	PATTERN_DATA,

	/* Add all data types before this MAX_DATA_TYPE. MAX_DATA_TYPE is the array
	   size for storing data, as it is one larger than the highest numbered
	   defined type. */
	MAX_DATA_TYPE
};

/** Meta-data struct for the data stored by different layers */
struct MetaData {
	int offset;
	int size;
	int simulatedSize;
};

/** Number of bytes from which all size calculations of the data block start */
#define INITIAL_BLOCK_SIZE 16

/** Class that models a packet in the network stack. */
class Packet : public cMessage {
	private:
		/** Meta-data for locating per-layer stored data */
		MetaData metaData[MAX_DATA_TYPE];
		/** Pointer to data storage for per-layer stored data, or @c NULL if
		    not yet allocated. */
		char *dataBlock;
		/** Size of allocated block */
		size_t dataBlockSize;
		/** Offset for the end of used data in the block */
		int freeOffset;
		/** Simulated length of the preamble */
		double preamble_time;
	
	protected:
		/** Copy constructor.

		    Note: if you want to create a copy of this @b Packet, use @b dup.
		*/
		Packet(Packet &packet);

    public:
		/** Network layer originator address */
		int from,
		/** Network layer destination address */
			to,
		/** MAC layer originator address */
			local_from,
		/** MAC layer destination address. */
			local_to,
		/** Network layer serial number. */
			serial;
		/** Power of the message (only for use by the radio layer). */
		double power;
		
		/** Constructor
		    @param name Name used when printing information about the @b Packet.
		    @param kind Message type.
	
		    Message type defaults to @c TX.
		*/
		Packet(const char *name, int kind = TX);
		/** Destructor */
		~Packet(void) { free(dataBlock); }

		/** Store layer-specific data in the packet.
		    @param type Type (layer) of data to be stored.
		    @param data Pointer to the data to be stored.
		    @param size The size of the data to be stored.
		    @param simulatedSize The number of bytes that the data will use in
			the simulated packet.
	
		    If the @a simulatedSize is negative or not supplied, it is assumed
		    to be the same as the @a size argument.
		*/
		void setData(DataType type, void *data, int size, int simulatedSize = -1);
		/** Discard the layer-specific data in the packet.
		    @param type Type (layer) of data to be discarded.
		*/
		void discardData(DataType type);
		/** Get a pointer to the layer-specific data in the packet.
		    @param type Type (layer) of data stored.
		    @return A pointer to the requested data, or @c NULL.
		*/
		void *getData(DataType type);
		
		/** Duplicate this @b Packet.
			@return A pointer to a copy of this @b Packet.

		    Creates a copy of this @b Packet, taking polymorphism into account.
		*/
		virtual cObject* dup();
		
		/** Set the @a from field. */
		void setFrom(int f);
		/** Set the @a local_from field. */
		void setLocalFrom(int f);
		/** Set the @a to field. */
		void setTo(int t);
		/** Set the @a local_to field. */
		void setLocalTo(int t);
		/** Increase the simulated length of the packet.
		    @param inc The number of bytes to simulate.
		*/
		void increaseLength(int inc);
		/** Decrease the simulated length of the packet.
		    @param dec The number of bytes to decrease the simulated size.
		*/
		void decreaseLength(int dec);
		/** Get the duration of the preamble.
			@return The duration of the preamble that is simulated.
		*/
		double getPreambleTime() { return preamble_time; }
		/** Set the duration of the preamble.
			@param pt The duration of the preamble to simulate.
		*/
		void setPreambleTime(double pt) { preamble_time = pt; }
};

/** Simple class to transfer RSSI values. */
class RssiMessage : public cMessage {
	public:
		/** RSSI value to be communicated. */
		double rssi;
		/** Constructor
		    @param name Name used when printing information about the @b Packet.
		    @param r RSSI value.
		*/
		RssiMessage(const char *name = 0, double r = 0.0) : cMessage(name, SET_RSSI) { rssi = r; }
		/** Duplicate this @b RssiMessage.
			@return A pointer to a copy of this @b RssiMessage.

		    Creates a copy of this @b RssiMessage, taking polymorphism into
		    account.
		*/
		cObject* dup() const { return new RssiMessage(*this); }
};

#endif
