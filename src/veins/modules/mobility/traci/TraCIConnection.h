#pragma once

#include <stdint.h>
#include <memory>

#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/modules/mobility/traci/TraCICoordinateTransformation.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/Heading.h"
#include "veins/modules/utility/HasLogProxy.h"

namespace Veins {

class TraCIConnection : public HasLogProxy {
public:
    static TraCIConnection* connect(cComponent* owner, const char* host, int port);
    void setNetbounds(TraCICoord netbounds1, TraCICoord netbounds2, int margin);
    ~TraCIConnection();

    /**
     * sends a single command via TraCI, checks status response, returns additional responses
     */
    TraCIBuffer query(uint8_t commandId, const TraCIBuffer& buf = TraCIBuffer());

    /**
     * sends a single command via TraCI, expects no reply, returns true if successful
     */
    TraCIBuffer queryOptional(uint8_t commandId, const TraCIBuffer& buf, bool& success, std::string* errorMsg = nullptr);

    /**
     * sends a message via TraCI (after adding the header)
     */
    void sendMessage(std::string buf);

    /**
     * receives a message via TraCI (and strips the header)
     */
    std::string receiveMessage();

    /**
     * convert TraCI heading to OMNeT++ heading (in rad)
     */
    Heading traci2omnetHeading(double heading) const;

    /**
     * convert OMNeT++ heading (in rad) to TraCI heading
     */
    double omnet2traciHeading(Heading heading) const;

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
    TraCIConnection(cComponent* owner, void* ptr);

    void* socketPtr;
    std::unique_ptr<TraCICoordinateTransformation> coordinateTransformation;
};

/**
 * returns byte-buffer containing a TraCI command with optional parameters
 */
std::string makeTraCICommand(uint8_t commandId, const TraCIBuffer& buf = TraCIBuffer());

} // namespace Veins
