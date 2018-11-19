/*
 * BatteryAccess.h
 *
 *  Created on: Aug 26, 2009
 *      Author: karl
 */

#pragma once

#include "veins/veins.h"

#include "veins/base/modules/BaseModule.h"
#include "veins/base/modules/BaseBattery.h"

namespace Veins {

/**
 * @brief Extends BaseModule by several methods which provide
 * access to the battery module.
 *
 * @ingroup power
 * @ingroup baseModules
 * @author Karl Wessel
 */
class VEINS_API BatteryAccess : public BaseModule {
protected:
    /** @brief Stores pointer to the battery module. */
    BaseBattery* battery;

    /** @brief This devices id for the battery module. */
    int deviceID;

protected:
    /**
     * @brief Registers this module as a device with the battery module.
     *
     * If no battery module is available than nothing happens.
     */
    void registerWithBattery(const std::string& name, int numAccounts);

    /**
     * @brief Draws the amount defined by the passed DrawAmount from the
     * battery on account of the passed account.
     *
     * If no battery module is available than nothing happens.
     */
    void draw(DrawAmount& amount, int account);

    /**
     * @brief Draws the passed amount of current (in mA) over time from the
     * battery on account of the passed account.
     *
     * If no battery module is available than nothing happens.
     */
    void drawCurrent(double amount, int account);

    /**
     * @brief Draws the passed amount of energy (in mWs) from the
     * battery on account of the passed account.
     *
     * If no battery module is available than nothing happens.
     */
    void drawEnergy(double amount, int account);

public:
    BatteryAccess();
    BatteryAccess(unsigned stacksize);
};

} // namespace Veins
