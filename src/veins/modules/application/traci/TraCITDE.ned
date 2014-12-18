//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package org.car2x.veins.modules.application.traci;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

simple TraCITDE extends BaseWaveApplLayer
{
        //This is for the signal emitted in the C++ code.
        @signal[detectedVehiclesLV](type="short");
        @signal[detectedVehiclesHV](type="short");
        @signal[detectedVehiclesMC](type="short");
        
        //Recording statistics from the model
        @statistic[currentNumberofDetectedLV](title="Detected Light Vehicles"; source="detectedVehiclesLV"; record=stats,vector);
        @statistic[currentNumberofDetectedHV](title="Detected Heavy Vehicles"; source="detectedVehiclesHV"; record=stats,vector);
        @statistic[currentNumberofDetectedMC](title="Detected Motorcycles"; source="detectedVehiclesMC"; record=stats,vector);
        
        //Registering the class
        @class(TraCITDE);
        @display("i=block/app2");
        
        //Additional parameters
        bool sendWhileParking  = default(false); //send messages when the car is parked
        bool debugAppTDE = default(false); //print TDE application messages
}
