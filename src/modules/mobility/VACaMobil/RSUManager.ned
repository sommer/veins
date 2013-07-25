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


//
// VACaMobil was presented in:
// 
// [1] M. Báguena, S. M. Tornell, A. Torres, C. T. Calafate, J.-C. Cano, and P. Manzoni, 
// “VACaMobil: VANET Car Mobility Manager for OMNeT++,” in IEEE International Conference
// on Communications 2013: IEEE ICC’13 - 3rd IEEE International Workshop on Smart 
// Communication Protocols and Algorithms (SCPA 2013) (ICC'13 - IEEE ICC'13 - Workshop SCPA), 2013.
//
// please cite our paper if you use it.
// 
// If you use VACaMobil for your publications, please inform us at www.grc.upv.es
//
simple RSUManager
{
    parameters:
    @display("i=block/source");
    @class("RSUManager");
    xml rsuPlacement = default(xml("<poas></poas>")); //VACaMobil supports the creation of Road Side Units which location is specified in this file
    string rsuModule = default(""); //The omnetpp module of RSU.
    string rsuPrefix = default(""); //The omnetpp module of RSU.
    int nRandomRsu = default(0); //number of RSU place randomly in joints.
}
