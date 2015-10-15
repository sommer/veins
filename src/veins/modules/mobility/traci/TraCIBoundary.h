#ifndef VEINS_MOBILITY_TRACIBOUNDARY_H_
#define VEINS_MOBILITY_TRACIBOUNDARY_H_

#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/base/utils/Coord.h"

namespace Veins {

class TraCIBoundary
{
	public:
		TraCIBoundary();

		/**
		 * set boundary limits
		 * @param topleft Top left corner reported by SUMO
		 * @param bottomright Bottom right corner reported by SUMO
		 * @param margin SUMO coordinate should have this margin offset
		 */
		void set(TraCICoord topleft, TraCICoord bottomright, int margin);

		/**
		 * Test if a coordinate is within boundary
		 * @note z coordinate is ignored
		 * @param c Coordinate to test
		 * @return true if coordinate lies inside
		 */
		bool within(const Coord& c) const;

		/**
		 * get maximum coordinate according to boundary settings
		 */
		Coord max() const;

		/**
		 * convert TraCI angle to OMNeT++ angle (in rad)
		 */
		double traci2omnetAngle(double angle) const;

		/**
		 * convert OMNeT++ angle (in rad) to TraCI angle
		 */
		double omnet2traciAngle(double angle) const;

		/**
		 * convert TraCI coordinates to OMNeT++ coordinates
		 */
		Coord traci2omnet(TraCICoord coord) const;
		std::list<Coord> traci2omnet(const std::list<TraCICoord>&) const;

		/**
		 * convert OMNeT++ coordinates to TraCI coordinates
		 */
		TraCICoord omnet2traci(Coord coord) const;
		std::list<TraCICoord> omnet2traci(const std::list<Coord>&) const;

	private:
		TraCICoord netbounds1; /* network boundaries as reported by TraCI (x1, y1) */
		TraCICoord netbounds2; /* network boundaries as reported by TraCI (x2, y2) */
		int margin;
};

} /* namespace Veins */

#endif /* VEINS_MOBILITY_TRACIBOUNDARY_H_ */
