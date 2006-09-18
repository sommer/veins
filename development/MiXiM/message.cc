#include "message.h"
#include "mixim.h"

Packet::Packet(const char *name, int kind) :cMessage(name, kind) {
	preamble_time = PREAMBLE_TIME;
	
	/* Set all values in the metaData array to -1 */
	memset(metaData, 0xff, sizeof(metaData));
	dataBlock = NULL;
	dataBlockSize = 0;
	freeOffset = 0;
	serial = -1;
	power = 0.0;
}

Packet::Packet(Packet &packet) : cMessage(packet) {
	/* Copy Packet specific member variables */
	from = packet.from;
	to = packet.to;
	local_from = packet.local_from;
	local_to = packet.local_to;
	memcpy(metaData, packet.metaData, sizeof(metaData));
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

cObject *Packet::dup() {
	Packet *result = new Packet(*this);
	return result;
}

static int smallestBlock(int minimum, int current) {
	while (current < minimum) current <<= 1;
	return current;
}

void Packet::setData(DataType type, void *data, int size, int simulatedSize) {
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
		if (metaData[type].offset >= 0) {
			/* Remove previous simulated size */
			addLength( -metaData[type].simulatedSize );
			/* Check whether the new data fits over the old */
			if (size <= metaData[type].size) {
				/* Use memmove just to be sure */
				memmove(dataBlock + metaData[type].offset, data, size);
				metaData[type].size = size;
				metaData[type].simulatedSize = simulatedSize;
				addLength(simulatedSize);
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
	metaData[type].offset = freeOffset;
	metaData[type].size = size;
	metaData[type].simulatedSize = simulatedSize;
	
	/* adjust size and offset members */
	freeOffset += size;
	addLength(simulatedSize);
}

void Packet::discardData(DataType type) {
	/* Remove all previously set data. This also means that we will not use
	   this bit of memory again! */
	if (metaData[type].offset >= 0) {
		addLength(-metaData[type].simulatedSize);
		metaData[type].offset = -1;
		metaData[type].size = -1;
		metaData[type].simulatedSize = -1;
	}
}

void *Packet::getData(DataType type) {
	if (metaData[type].offset < 0)
		return NULL;
	return dataBlock + metaData[type].offset;
}

void Packet::setFrom(int f) { from = f; }
void Packet::setLocalFrom(int f){ local_from = f; }
void Packet::setTo(int t){ to = t; }
void Packet::setLocalTo(int t){ local_to = t; }
void Packet::increaseLength(int inc) {
	addLength(inc);
}

void Packet::decreaseLength(int dec) {
	assert(length() >= dec);
	addLength(-dec);
}
