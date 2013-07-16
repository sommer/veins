//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package org.mixim.modules.mobility.VACaMobil;

import org.mixim.modules.mobility.traci.TraCIScenarioManagerLaunchd;


//
//
simple VACaMobil extends TraCIScenarioManagerLaunchd
{
    parameters:
    @display("i=block/source");
    @class("VACaMobil");
    
    string vehicleRates = default("");
    int meanNumberOfCars = default(0);
    int carHysteresisValue = default(0);
    int warmUpSeconds @unit(s) = default(-1s); //With a value lower than firstStepAt uses simulation.getWarmupPeriod()
    
    bool getStatistics = default(false); //If set to true it generates some kind-of heat maps
    string statFiles = default("");
    bool doNothing = default(false); //If it's set to true it should work like TRACI Scenario Manager Launchd
    xml rsuPlacement = default(xml("<poas></poas>")); //VACaMobil supports the creation of Road Side Units which location is specified in this file
    string rsuModule = default(""); //The omnetpp module of RSU.
    string rsuPrefix = default(""); //The omnetpp module of RSU.
    int nRandomRsu = default(0); //number of RSU place randomly in joints.
    @signal[onSimulationCarsSignal];
    @statistic[onSimulationCars](title="Active cars";source="onSimulationCarsSignal";record=vector,stats);
}
