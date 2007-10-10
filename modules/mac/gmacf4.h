#ifndef __GMACF4_H__
#define __GMACF4_H__

#include "gmac.h"

class GMacF4 : public GMac {

	// contructor, destructor, module stuff
	Module_Class_Members(GMacF4, GMac, 0);

private:
	static bool parametersInitialised;

protected:
	static int slots, frames, frameShift, bcast_slots;
	static bool slotted_bcast;

	int current_frame;
	unsigned int myHash;

	virtual void initialize();
	virtual void finish();
	virtual void wrapSlotCounter();
	virtual SlotState getCurrentSlotState();
	virtual int slotsUntilWake(int destination);

	class HeaderF : public Header {
		struct {
			MsgType type;
		} _data;
	public:
		HeaderF(MsgType type) { _data.type = type; }
		HeaderF(void *data) { memcpy(&_data, data, sizeof(_data)); }
		virtual void *data() { return &_data; }
		virtual int dataSize() { return sizeof(_data); }
		virtual int extraLength() { return _data.type == MSG_ACK || _data.type == MSG_DATA ? ADDRESS_BYTES : 0; }
		virtual MsgType getType() { return _data.type; }
		virtual ~HeaderF() {}
	};

	virtual Header *newHeader(MsgType type) { return new HeaderF(type); }
	virtual Header *newHeader(void *data) { return new HeaderF(data); }
	virtual ~GMacF4();
};


#endif
