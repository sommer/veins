#pragma once

#include "veins/veins.h"

namespace veins {

/**
 * Coord equivalent for storing TraCI coordinates
 */
struct VEINS_API TraCICoord {
    TraCICoord()
        : x(0.0)
        , y(0.0)
    {
    }
    TraCICoord(double x, double y)
        : x(x)
        , y(y)
    {
    }
    double x;
    double y;
};

} // namespace veins
