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

simple TraCITDERSU11p extends BaseWaveApplLayer
{
    @class(TraCITDERSU11p);
    @display("i=block/app2");
    
    //This is for the signal emitted in the C++ code.
    @signal[detectedVehiclesLV](type="short");
    @signal[detectedVehiclesHV](type="short");
    @signal[detectedVehiclesMC](type="short");
    @signal[detectedVehiclesTotal](type="short");
    @signal[trafficVolumeSignal](type="double");
    @signal[VCRatioSignal](type="double");
    
    
    //Recording statistics from the model
    @statistic[currentNumberofDetectedLV](title="Detected Light Vehicles"; source="detectedVehiclesLV"; record=vector);
    @statistic[currentNumberofDetectedHV](title="Detected Heavy Vehicles"; source="detectedVehiclesHV"; record=vector);
    @statistic[currentNumberofDetectedMC](title="Detected Motorcycles"; source="detectedVehiclesMC"; record=vector);
    @statistic[currentNumberofTotalDetectedVehicles](title="Total Detected Vehicles"; source="detectedVehiclesTotal"; record=vector);
    @statistic[currentVolume](title="Traffic Volume"; source="trafficVolume"; record=vector);
    @statistic[vcr](title="Volume to Capacity Ratio"; source="VCRatio"; record=vector);
    
    bool debugAppTDE = default(false); //print TDE application messages
    bool enableTDE = default(false);
    double statInterval @unit("s") = default(0.1s);
    double timeoutInterval @unit("s") = default(10s);
    double timeoutMsgInterval @unit("s") = default(1s); //checking every seconds
    
    double pceMC = default(0.40);
    double pceLV = default(1.0);
    double pceHV = default(1.3);
    
    double basicCapacity;
    double CFWidth;
    double CFSideFriction;
    double CFCitySize;
    double numberOfLanes;
    string roadId;
    double roadCondUpdateInterval @unit("s") = default(60s);
    
    bool interventionTest = default(false);
    double interventionTime @unit("s");
}
