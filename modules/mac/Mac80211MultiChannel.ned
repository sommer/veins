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

package org.mixim.modules.mac;

//
// Extends the the 802.11b MAC protocol by multi channel support meaning
// changing of the channel used for transmission and reception during run-time. 
// This module is intended to be used in combination with the 
// Decider80211MultiChannel as decider for the physical layer.
// See Nic80211MultiChannel for a pre configured NIC using this module.
//
// Note: Decider80211MultiChannel does not model interference between adjacent 
//       channels!
//
// This module supports ad-hoc mode only (does not generate or handle
// management frames), and does not support fragmentation, RTS threshold,
// duplication detection and recovery.
//
simple Mac80211MultiChannel extends Mac80211
{
    parameters:
        @class(Mac80211MultiChannel);        
}
