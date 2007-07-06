#ifndef FOXTROT_PACKET_STORAGE
#define FOXTROT_PACKET_STORAGE

#include <vector>
#include "FoxtrotPacket.h"

class FoxtrotPacketStorage:public std::vector < FoxtrotPacket * >
{
  public:

	virtual void erase(unsigned int k)
	{
		std::vector < FoxtrotPacket * >::iterator p = begin() + k;
		delete *p;
		std::vector < FoxtrotPacket * >::erase(p);
	}

	virtual void clear()
	{
		while (size() > 0)
			erase(size() - 1);
	}
};

#endif
