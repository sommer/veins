#ifndef __PACKETMESSAGE_H__
#define __PACKETMESSAGE_H__

#include <omnetpp.h>
#include "NicControlType.h"

/** Enum to hold all the known message types. */
enum {
	TX = 1,
	RX,
	
	/* Crankshaft <-> routing interface. Message types for lost contention, for
	   no ACK received and for retries that were skipped. */
	TX_CONTEND_LOSE,
	TX_NO_ACK,
	TX_RETRY_SKIPPED
};

/** Broadcast address */
#define BROADCAST (-1)

/** Meta-data struct for the data stored by different layers */
struct MetaData {
	int offset;
	int size;
	int simulatedSize;
};

/** Number of bytes from which all size calculations of the data block start */
#define INITIAL_BLOCK_SIZE 16

class EyesMacLayer;

/** Class that models a packet in the network stack. */
class MacPacket : public cMessage {
	private:
		/** Meta-data for locating  stored data */
		MetaData metaData;
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

		    Note: if you want to create a copy of this @b MacPacket, use @b dup.
		*/
		MacPacket(const MacPacket &packet);

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
		    @param name Name used when printing information about the @b MacPacket.
		    @param kind Message type.
	
		    Message type defaults to @c TX.
		*/
		MacPacket(EyesMacLayer*, const char *name, int kind = TX);
		/** Destructor */
		~MacPacket(void) { free(dataBlock); }

		/** Store MAC-specific data in the packet.
		    @param data Pointer to the data to be stored.
		    @param size The size of the data to be stored.
		    @param simulatedSize The number of bytes that the data will use in
			the simulated packet.
	
		    If the @a simulatedSize is negative or not supplied, it is assumed
		    to be the same as the @a size argument.
		*/
		void setData(void *data, int size, int simulatedSize = -1);
		/** Discard the MAC-specific data in the packet.
		*/
		void discardData();
		/** Get a pointer to the MAC-specific data in the packet.
		    @return A pointer to the requested data, or @c NULL.
		*/
		void *getData();
		
		/** Duplicate this @b MacPacket.
			@return A pointer to a copy of this @b MacPacket.

		    Creates a copy of this @b MacPacket, taking polymorphism into account.
		*/
		virtual cObject* dup() const;
		
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
		    @param name Name used when printing information about the @b MacPacket.
		    @param r RSSI value.
		*/
		RssiMessage(const char *name = 0, double r = 0.0) : cMessage(name, NicControlType::SET_RSSI) { rssi = r; }
		/** Duplicate this @b RssiMessage.
			@return A pointer to a copy of this @b RssiMessage.

		    Creates a copy of this @b RssiMessage, taking polymorphism into
		    account.
		*/
		cObject* dup() const { return new RssiMessage(*this); }
};

#endif
