#ifndef __CRANKSHAFTF1_H__
#define __CRANKSHAFTF1_H__

#include "crankshaft-base.h"

class CrankshaftF1 : public CrankshaftBase {

	// contructor, destructor, module stuff
	//Module_Class_Members(CrankshaftF1, CrankshaftBase, 0);

private:
	static bool parametersInitialised;

protected:
	static int slots;

	virtual void initialize();
	virtual void finish();
	virtual void txPacket(MacPacket * msg);
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
	virtual ~CrankshaftF1();
};


#endif
