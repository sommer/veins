#pragma once

namespace Veins {

/**
 * Coord equivalent for storing TraCI coordinates
 */
struct TraCICoord {
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

} // namespace Veins
