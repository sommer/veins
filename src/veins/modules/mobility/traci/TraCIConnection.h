#ifndef VEINS_MOBILITY_TRACI_TRACICONNECTION_H_
#define VEINS_MOBILITY_TRACI_TRACICONNECTION_H_

#include <stdint.h>
#include <memory>
#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include <veins/modules/mobility/traci/TraCICoordinateTransformation.h>
#include "veins/base/utils/Coord.h"

namespace Veins {

class TraCIConnection
{
	public:
		static TraCIConnection* connect(const char* host, int port);
		void setNetbounds(TraCICoord netbounds1, TraCICoord netbounds2, int margin);
		~TraCIConnection();

		/**
		 * sends a single command via TraCI, checks status response, returns additional responses
		 */
		TraCIBuffer query(uint8_t commandId, const TraCIBuffer& buf = TraCIBuffer());

		/**
		 * sends a single command via TraCI, expects no reply, returns true if successful
		 */
		TraCIBuffer queryOptional(uint8_t commandId, const TraCIBuffer& buf, bool& success, std::string* errorMsg = 0);

		/**
		 * sends a message via TraCI (after adding the header)
		 */
		void sendMessage(std::string buf);

		/**
		 * receives a message via TraCI (and strips the header)
		 */
		std::string receiveMessage();

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
		TraCIConnection(void*);

		void* socketPtr;
		std::unique_ptr<TraCICoordinateTransformation> coordinateTransformation;
};

/**
 * returns byte-buffer containing a TraCI command with optional parameters
 */
std::string makeTraCICommand(uint8_t commandId, const TraCIBuffer& buf = TraCIBuffer());

}

#endif /* VEINS_MOBILITY_TRACI_TRACICONNECTION_H_ */
