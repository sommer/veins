#include "veins/modules/mobility/traci/TraCIVehicleSignals.h"

namespace Veins {

TraCIVehicleSignal::TraCIVehicleSignal() :
	flags(VEH_SIGNAL_NONE), undef(true) {
}

TraCIVehicleSignal::TraCIVehicleSignal(SignalFlags _flags) :
	flags(_flags), undef(false) {
}

bool TraCIVehicleSignal::test(SignalFlags flag) const {
	return (flags & flag) == flag;
}

void TraCIVehicleSignal::set(SignalFlags flag) {
	undef = false;
	flags |= flag;
}

void TraCIVehicleSignal::clear(SignalFlags flag) {
	flags &= ~flag;
}

TraCIVehicleSignal::operator bool() const {
	return !undef;
}

} // namespace Veins
