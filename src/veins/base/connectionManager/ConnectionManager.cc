#include "veins/base/connectionManager/ConnectionManager.h"

#include <cmath>

#include "veins/base/modules/BaseWorldUtility.h"

#ifndef ccEV
#define ccEV EV_DEBUG << getName() << ": "
#endif

Define_Module( ConnectionManager );

double ConnectionManager::calcInterfDist()
{
    /* With the introduction of antenna models, calculating the maximum
     * interference distance only based on free space loss doesn't make any sense
     * any more as it could also be much bigger due to positive antenna gains.
     * Therefore, the user has to provide a reasonable maximum interference
     * distance himself. */
    if (hasPar("maxInterfDist")) {
        double interfDistance = par("maxInterfDist").doubleValue();
        ccEV << "max interference distance:" << interfDistance << endl;
        return interfDistance;
    } else {
        throw cRuntimeError("ConnectionManager: No value for maximum interference distance (maxInterfDist) provided.");
    }
}

