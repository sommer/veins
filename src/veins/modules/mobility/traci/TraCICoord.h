#ifndef VEINS_MOBILITY_TRACI_TRACICOORD_H_
#define VEINS_MOBILITY_TRACI_TRACICOORD_H_

namespace Veins {

/**
 * Coord equivalent for storing TraCI coordinates
 */
struct TraCICoord {
	TraCICoord() : x(0.0), y(0.0), z(0.0) {}
	TraCICoord(double x, double y) : x(x), y(y), z(0.0) {}
	TraCICoord(double x, double y, double z) : x(x), y(y), z(z) {}
	double x;
	double y;
	double z;
};

}

#endif /* VEINS_MOBILITY_TRACI_TRACICOORD_H_ */
