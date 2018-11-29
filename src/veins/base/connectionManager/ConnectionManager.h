#pragma once

#include "veins/veins.h"

#include "veins/base/connectionManager/BaseConnectionManager.h"

namespace Veins {

/**
 * @brief BaseConnectionManager implementation which only defines a
 * specific max interference distance.
 *
 * Calculates the maximum interference distance based on the transmitter
 * power, wavelength, pathloss coefficient and a threshold for the
 * minimal receive Power.
 *
 * @ingroup connectionManager
 */
class VEINS_API ConnectionManager : public BaseConnectionManager {
protected:
    /**
     * @brief Calculate interference distance
     *
     * You may want to overwrite this function in order to do your own
     * interference calculation
     */
    double calcInterfDist() override;
};

} // namespace Veins
