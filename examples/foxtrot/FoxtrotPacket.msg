cplusplus {{
#include "foxtrot_types.h"
#include "BasePacket.h"
}}



class BasePacket;


struct foxtrot_data;



message FoxtrotPacket extends BasePacket
{
    @customize(true);
    unsigned int when;
    foxtrot_data loc[AXES];
    foxtrot_data data[];
}
