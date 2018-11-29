/*
 * BatteryAccess.cc
 *
 *  Created on: Aug 26, 2009
 *      Author: Karl Wessel
 */

#include "veins/base/modules/BatteryAccess.h"

#include "veins/base/utils/FindModule.h"

using Veins::BatteryAccess;

BatteryAccess::BatteryAccess()
    : BaseModule()
    , battery(nullptr)
{
}

BatteryAccess::BatteryAccess(unsigned stacksize)
    : BaseModule(stacksize)
    , battery(nullptr)
{
}

void BatteryAccess::registerWithBattery(const std::string& name, int numAccounts)
{
    battery = FindModule<BaseBattery*>::findSubModule(findHost());

    if (!battery) {
        throw cRuntimeError("No battery module defined!");
    }
    else {
        deviceID = battery->registerDevice(name, numAccounts);
    }
}

void BatteryAccess::draw(DrawAmount& amount, int account)
{
    if (!battery) return;

    battery->draw(deviceID, amount, account);
}

void BatteryAccess::drawCurrent(double amount, int account)
{
    if (!battery) return;

    DrawAmount val(DrawAmount::CURRENT, amount);
    battery->draw(deviceID, val, account);
}

void BatteryAccess::drawEnergy(double amount, int account)
{
    if (!battery) return;

    DrawAmount val(DrawAmount::ENERGY, amount);
    battery->draw(deviceID, val, account);
}
