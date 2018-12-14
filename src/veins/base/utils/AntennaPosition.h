#pragma once

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"

namespace Veins {

/**
 * Stores the position of the host's antenna along with its speed, so that it can be linearly extrapolated.
 */
class AntennaPosition {

public:
    AntennaPosition()
        : id(-1)
        , p()
        , v()
        , t()
        , undef(true)
    {
    }

    /**
     * Store a position p that changes by v for every second after t.
     */
    AntennaPosition(int id, Coord p, Coord v, simtime_t t)
        : id(id)
        , p(p)
        , v(v)
        , t(t)
        , undef(false)
    {
    }

    /**
     * Get the (linearly extrapolated) position at time t.
     */
    Coord getPositionAt(simtime_t t = simTime()) const
    {
        ASSERT(t >= this->t);
        ASSERT(!undef);
        auto dt = t - this->t;
        return p + v * dt.dbl();
    }

    bool isSameAntenna(const AntennaPosition& o) const
    {
        ASSERT(!undef);
        ASSERT(!o.undef);
        return (id == o.id);
    }

protected:
    int id; /**< unique identifier of antenna returned by ChannelAccess::getId() */
    Coord p; /**< position for linear extrapolation */
    Coord v; /**< speed for linear extrapolation */
    simtime_t t; /**< time for linear extrapolation */
    bool undef; /**< true if created using default constructor */
};

} // namespace Veins
