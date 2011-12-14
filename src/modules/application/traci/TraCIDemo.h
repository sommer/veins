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

#ifndef TraCIDemo_H
#define TraCIDemo_H

#include "BaseApplLayer.h"
#include "mobility/traci/TraCIMobility.h"

/**
 * Small IVC Demo
 */
class TraCIDemo : public BaseApplLayer
{
	public:
		virtual void initialize(int);

	protected:
		static const simsignalwrap_t mobilityStateChangedSignal;

	protected:
		TraCIMobility* traci;
		bool sentMessage;

	protected:
		virtual void handleSelfMsg(cMessage*);
		virtual void handleLowerMsg(cMessage*);

		virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

		void sendMessage();
		void handlePositionUpdate();
};

#endif
