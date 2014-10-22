#ifndef VEINS_MOBILITY_TRACI_TRACICOORD_H_
#define VEINS_MOBILITY_TRACI_TRACICOORD_H_

namespace Veins {

/**
 * Coord equivalent for storing TraCI coordinates
 */
struct TraCICoord {
	TraCICoord() : x(0.0), y(0.0) {}
	TraCICoord(double x, double y) : x(x), y(y) {}
	double x;
	double y;
};

}

#endif /* VEINS_MOBILITY_TRACI_TRACICOORD_H_ */
