#include "message.h"
#include <assert.h>
#include "mac.h"
//#include "mixim.h"

MacPacket::MacPacket(EyesMacLayer *mac, const char *name, int kind) :cMessage(name, kind) {
	preamble_time = mac->preamble_time;
	
	/* Set all values in the metaData array to -1 */
	memset(&metaData, 0xff, sizeof(metaData));
	dataBlock = NULL;
	dataBlockSize = 0;
	freeOffset = 0;
	serial = -1;
	power = 0.0;
}

MacPacket::MacPacket(const MacPacket &packet) : cMessage(packet){
	/* Copy Packet specific member variables */
	from = packet.from;
	to = packet.to;
	local_from = packet.local_from;
	local_to = packet.local_to;
	memcpy(&metaData, &packet.metaData, sizeof(metaData));
	dataBlockSize = packet.dataBlockSize;
	freeOffset = packet.freeOffset;
	preamble_time = packet.preamble_time;
	power = packet.power;
	serial = packet.serial;
	
	/* Copy data, which means we may have to allocate a new block of memory */
	if (packet.dataBlock != NULL) {
		dataBlock = (char *) malloc(dataBlockSize);
		if (dataBlock == NULL) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
		memcpy(dataBlock, packet.dataBlock, dataBlockSize);
	} else {
		dataBlock = NULL;
	}
}

cObject *MacPacket::dup() const {
	MacPacket *result = new MacPacket(*this);
	return result;
}

static int smallestBlock(int minimum, int current) {
	while (current < minimum) current <<= 1;
	return current;
}

void MacPacket::setData(void *data, int size, int simulatedSize) {
	assert(size > 0);
	
	/* No simulated size has been entered, or someone made a booboo */
	if (simulatedSize < 0)
		simulatedSize = size;

	/* If dataBlock is NULL we surely have to allocate a new block of memory */
	if (dataBlock == NULL) {
		dataBlockSize = smallestBlock(size, INITIAL_BLOCK_SIZE);
		dataBlock = (char *) malloc(dataBlockSize);
		if (dataBlock == NULL) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	} else {
		/* Only check for existing data and/or sufficient free space if there
		   already was a an existing dataBlock to begin with */
		
		/* Check if this layer already has data stored */
		if (metaData.offset >= 0) {
			/* Remove previous simulated size */
			addBitLength( -metaData.simulatedSize );
			/* Check whether the new data fits over the old */
			if (size <= metaData.size) {
				/* Use memmove just to be sure */
				memmove(dataBlock + metaData.offset, data, size);
				metaData.size = size;
				metaData.simulatedSize = simulatedSize;
				addBitLength(simulatedSize);
				return;
			}
		}
	
		/* Check whether we need to reallocate the dataBlock to accomodate the
		   new data */
		if ((size_t)(freeOffset + size) > dataBlockSize) {
			dataBlockSize = smallestBlock(freeOffset + size, dataBlockSize);
			dataBlock = (char *) realloc(dataBlock, dataBlockSize);
			if (dataBlock == NULL) {
				fprintf(stderr, "Out of memory\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	
	/* Copy the data into our memory block. memcpy can safely be used here as
	   we write over previously unused memory. */
	memcpy(dataBlock + freeOffset, data, size);
	metaData.offset = freeOffset;
	metaData.size = size;
	metaData.simulatedSize = simulatedSize;
	
	/* adjust size and offset members */
	freeOffset += size;
	addBitLength(simulatedSize);
}

void MacPacket::discardData() {
	/* Remove all previously set data. This also means that we will not use
	   this bit of memory again! */
	if (metaData.offset >= 0) {
		addBitLength(-metaData.simulatedSize);
		metaData.offset = -1;
		metaData.size = -1;
		metaData.simulatedSize = -1;
	}
}

void *MacPacket::getData() {
	if (metaData.offset < 0)
		return NULL;
	return dataBlock + metaData.offset;
}

void MacPacket::setFrom(int f) { from = f; }
void MacPacket::setLocalFrom(int f){ local_from = f; }
void MacPacket::setTo(int t){ to = t; }
void MacPacket::setLocalTo(int t){ local_to = t; }
void MacPacket::increaseLength(int inc) {
	addBitLength(inc);
}

void MacPacket::decreaseLength(int dec) {
	assert(length() >= dec);
	addBitLength(-dec);
}
