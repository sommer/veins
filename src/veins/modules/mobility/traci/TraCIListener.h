#ifndef VEINS_WORLD_TRACI_TRACILISTENER_H
#define VEINS_WORLD_TRACI_TRACILISTENER_H

namespace Veins {

class TraCIListener
{
	public:
		virtual ~TraCIListener() {}
		virtual void init() {}
		virtual void step() {}
		virtual void close() {}
};

} // namespace Veins

#endif /* VEINS_WORLD_TRACI_TRACILISTENER_H */

