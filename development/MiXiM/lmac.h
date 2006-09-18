#ifndef __LMAC_H__
#define __LMAC_H__

#include "mac.h"

#include <inttypes.h>

#define DIV_CEIL(a,b) (((a) + ((b) - 1))/(b))

#define MAX_SLOTS 128
#define SLOT_WORDS (DIV_CEIL(MAX_SLOTS, 64))

struct NeighbourInfo {
	int id;
	uint64_t occupied_slots[SLOT_WORDS];
};

class LMac: public Mac {

	// contructor, destructor, module stuff
	Module_Class_Members(LMac, Mac, 0);

protected:
	int current_slot, max_slot, my_slot, data_length, header_length;
	int distance_to_gateway, backoff;
	bool verify_slot;
	uint64_t collision_slots[SLOT_WORDS], tentative_slots[SLOT_WORDS];
	bool use_tentative;

	int no_slots;

	NeighbourInfo neighbour_info[MAX_SLOTS];
	bool printed, initialized, ignore_failed, sendingData;
	Packet *tx_msg;

	virtual void initialize();
	virtual void finish();
	virtual void timeout(int which);
	virtual void txPacket(Packet * msg);
	virtual void rxFrame(Packet * msg);
	virtual void rxHeader(Packet * msg);
	virtual void rxStarted();
	virtual void transmitDone();
	virtual void rxFailed();
	virtual void pickSlot();

	struct Header {
		int current_slot;
		int distance_to_gateway;
		uint64_t occupied_slots[SLOT_WORDS];
		int collision_slot;
	};

public:
	virtual int headerLength();
	virtual void endForce(){}
};


#endif // __LMAC_H__
