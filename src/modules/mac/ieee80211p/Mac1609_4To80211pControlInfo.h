//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

/**
 * @brief Interface between Mac1609_4 and 802.11p.
 *
 * @class Mac1609_4
 * @ingroup Mac1609_4
 * @author Christopher Saloman
 * @author David Eckhoff
 */

#include <NetwToMacControlInfo.h>
#include <Consts80211p.h>

#ifndef ___MAC1609_4TO80211PCONTROLINFO_H_
#define ___MAC1609_4TO80211PCONTROLINFO_H_


class Mac1609_4To80211pControlInfo : public NetwToMacControlInfo {
	public:
		Mac1609_4To80211pControlInfo(int nextHopMac, int priority, double frequency, t_channel channel):
			priority(priority),frequency(frequency),channel(channel) {
		};

		virtual ~Mac1609_4To80211pControlInfo() { };
		void setPriority(int priority) {
			this->priority = priority;
		}
		int priority;
		double frequency;
		t_channel channel;
};

#endif /* ___MAC1609_4TO80211PCONTROLINFO_H_ */
