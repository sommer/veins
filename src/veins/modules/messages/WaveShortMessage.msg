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

packet WaveShortMessage {
	//Version of the Wave Short Message
	int wsmVersion = 0;
	//Determine which security mechanism was used
	int securityType = 0;
	//Channel Number on which this packet was sent
	int channelNumber;
	//Data rate with which this packet was sent
	int dataRate = 1;
	//User priority with which this packet was sent (note the AC mapping rules in Mac1609_4::mapUserPriority)
	int userPriority = 7;
	//Unique number to identify the service
	int psid = 0;
	//Provider Service Context
	string psc = "Service with some Data";
	//Length of Wave Short Message
	int wsmLength;
	//Data of Wave Short Message
	string wsmData = "Some Data";

	int senderAddress = 0;
	int recipientAddress = -1;
	int serial = 0;
	simtime_t timestamp = 0;
}
