/*
 * Antenna.h
 *
 *  Created on: Jun 13, 2016
 *      Author: Alexander Brummer
 */

#pragma once

#include "veins/base/utils/Coord.h"

namespace Veins {

/**
 * @brief The Antenna class is the base class of all antenna models.
 *
 * The purpose of all Antenna classes is to calculate the antenna gain
 * based on the current positions and orientations of the involved nodes.
 *
 * This base Antenna acts as an isotropic antenna, it always returns
 * a gain of 1.0. It is assigned to all nodes if the user does not specify
 * another antenna type.
 *
 * @author Alexander Brummer
 */
class Antenna {
public:
    Antenna(){};
    virtual ~Antenna(){};

    /**
     * Calculates the antenna gain of the represented antenna.
     *
     * In the case of this class, a value of 1.0 is returned always,
     * representing an isotropic radiator.
     *
     * Nevertheless, all Antenna subclasses' getGain() methods have to
     * take the following three parameters as the gain depends on the angle
     * of incidence in general.
     *
     * @param ownPos    - states the position of this antenna
     * @param ownOrient - the direction the antenna/the host is pointing in
     * @param otherPos  - the position of the other antenna which this antenna
     * is sending to or receiving from
     *
     * @return Returns the gain in this specific direction.
     */
    virtual double getGain(Coord ownPos, Coord ownOrient, Coord otherPos);

    virtual double getLastAngle()
    {
        return -1.0;
    };
};

} // namespace Veins
