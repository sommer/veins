cplusplus {{
#include "foxtrot_types.h"
#include "BasePacket.h"
}};

class BasePacket;
struct foxtrot_data;

message FoxtrotPacket extends BasePacket
{
	properties:	
		customize = true;
	fields:
		unsigned int when;
		foxtrot_data data[];
}
