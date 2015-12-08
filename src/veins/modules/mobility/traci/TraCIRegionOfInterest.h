#ifndef VEINS_WORLD_TRACI_TRACIREGIONOFINTEREST_H
#define VEINS_WORLD_TRACI_TRACIREGIONOFINTEREST_H

#include "veins/modules/mobility/traci/TraCICoord.h"
#include <list>
#include <set>
#include <string>
#include <utility>

namespace Veins {

class TraCIRegionOfInterest
{
	public:
		enum ConstraintResult {
			NONE, // when no constraints exist
			SATISFY,
			BREAK
		};
		/**
		 * Add roads to contraints
		 * \param roads given as road ids separated by spaces
		 */
		void addRoads(const std::string& roads);

		/**
		 * Add rectangles to constraints
		 * \param rects given as x1,y1-x2,y2 pairs separated by spaces
		 */
		void addRectangles(const std::string& rects);

		/**
		 * Remove all constraints
		 */
		void clear();

		/**
		 * Check if position lies on any ROI rectangle
		 * \param pos Position to check
		 * \return true if on any rectangle
		 */
		bool onAnyRectangle(const TraCICoord& pos) const;

		/**
		 * Check if a given road is part of interest roads
		 * \param road_id
		 * \return true if part of ROI roads
		 */
		bool partOfRoads(const std::string& road_id) const;

		/**
		 * Check if any constraints are defined
		 * \return true if constraints exist
		 */
		 bool hasConstraints() const;

	private:
		std::set<std::string> roiRoads; /**< roads (e.g. "hwy1 hwy2") constituting region of interest */
		std::list< std::pair<TraCICoord, TraCICoord> > roiRects; /**< rectangles (e.g. "0,0-10,10 20,20-30,30) constituting the region of interest */
};

} // namespace Veins

#endif
