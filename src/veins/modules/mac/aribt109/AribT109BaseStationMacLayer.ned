//
// Copyright (C) 2016 Julian Heinovski <julian.heinovski@ccs-labs.org>
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
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package org.car2x.veins.modules.mac.aribt109;

import org.car2x.veins.base.modules.BaseMacLayer;
import org.car2x.veins.modules.mac.aribt109.IAribT109Mac;

// Manages virtual and physical carrier sense function for medium access.
// Does not manage logical links
//
// @author Julian Heinovski
//
simple AribT109BaseStationMacLayer extends BaseMacLayer like IAribT109Mac
{
    parameters:
        @class(AribT109BaseStationMacLayer);

        double carrierFrequency @unit(MHz) = default(760 MHz);

        // bit rate
        double bitrate @unit(bps) = default(18 Mbps);

        //tx power [mW]
        double txPower @unit(mW) = default(10 mW);

        double ccaThreshold @unit(dBm) = default(-53 dBm);

        // every 6240us a new rvc period has to start
        // so there is no definition for the start times
        // instead, there is a definition for the duration
        // which is between 0 and 3024us (incl.)
        // NOTE: all base stations have to have the same periods

        // RVC period durations
        // determines the length of each period in us
        // example: 1000,3000,580,1456,792,1324,887,1542,0,120,3012,545,325,2565,10,0
        string rvcPeriodDurations;

        // base station period start in rvc period
        // determines which time intervall of the complete rvc period this particular base station is transmitting
        // values have to be between 0 and the value of the related rvc duration
        // example durations: 1000,2000,500,0,0,0,0,0,0,0,0,0,0,0,0,0
        // Base station a: 0,1500,400,0,0,0,0,0,0,0,0,0,0,0,0,0 -> durations: 500,1000,100
        // Base station b: 500,300,0,0,0,0,0,0,0,0,0,0,0,0,0,0 -> durations: 300,1200,150
        // Base station c: 800,0,150,0,0,0,0,0,0,0,0,0,0,0,0,0 -> durations: 200,300,150
        string baseStationTransmissionPeriodStarts;

        // values have to be between 0 and the value of the related rvc duration and invers to all other base stations transmission period starts
        // such that all starts of all base stations for one rvc period sum up to the period duration
        // Base station a: 500,1000,100,0,0,0,0,0,0,0,0,0,0,0,0,0
        // Base station b: 300,1200,150,0,0,0,0,0,0,0,0,0,0,0,0,0
        // Base station c: 100,300,150,0,0,0,0,0,0,0,0,0,0,0,0,0
        string baseStationTransmissionPeriodDurations;

        // set to 16*3024us if you want to disable the corresponding check
        double maxRsuTxLength @unit(ms) = default(16*3024us);

        /////////////////// signals & statistics ///////////////////

        // stuff

        // signal informing interested application about channel busy state
        @signal[sigChannelBusy](type=bool);

        // signal informing interested application about a collision
        @signal[sigCollision](type=bool);
	@statistic[collisions](source=sigCollision; record=count);

        // signal informing & logging a not allowed radio switch to TX while receiving a packet
        @signal[sigSwitchToTxWhileBusy](type=bool);

        // general numbers

        // logging the number of data packets to send
        @signal[sigDataPacketToSend](type=cPacket);
        @statistic[dataPacketsToSend](source=sigDataPacketToSend; record=count);

        // logging the number of sent packets
        @signal[sigMacPacketSent](type=AribT109MacPacket);
        @statistic[sentPackets](source=sigMacPacketSent; record=count; title="Sent MAC packets");

        // logging the number of received packets
        @signal[sigMacPacketReceived](type=bool);
        @statistic[receivedPackets](source=sigMacPacketReceived; record=count);

        // logging the number of received packets which were addressed to the node
        // in broadcast mode, this number should be equal to the complete received packets
        @signal[sigAddressedMacPacketReceived](type=bool);
        @statistic[receivedAddressedMacPackets](source=sigAddressedMacPacketReceived; record=count);

        // logging the distributed space between to mac packets
        @signal[sigDistributedSpace](type=long);
        @statistic[distributedSpace](source=sigDistributedSpace; record=count; title="Distributed space between two MAC packets."; unit=us);

        // packet size stuff

        // logging the data size received from the appl
        @statistic[dataPacketSize](source=packetBits(sigDataPacketToSend); record=stats; title="Data size of received packets from appl."; unit=b);

        // logging the data size of an ivc packet received from the virtual ivc layer
        @signal[sigIvcDataBits](type=long);
        @statistic[ivcPacketSize](source=sigIvcDataBits; record=stats; title="Data size of generated IVC packets."; unit=b);

        // logging the data size of a sent mac packet
        @statistic[macPacketSize](source=packetBits(sigSentMacPackets); record=stats; title="Data size of sent MAC packets."; unit=b);

        // discarded before transmission

        // logging the number of discarded data packets (before transmission)
        @signal[sigDataPacketDiscarded](type=bool);
        @statistic[discardedDataPackets](source=sigDataPacketDiscarded; record=count);

        // logging the number of discarded IVC packets (before transmission)
        @signal[sigIvcPacketDiscarded](type=bool);
        @statistic[discardedIvcPackets](source=sigIvcPacketDiscarded; record=count);

        // logging the number of discarded MAC packets (before transmission)
        @signal[sigMacPacketDiscarded](type=bool);
        @statistic[discardedMacPackets](source=sigMacPacketDiscarded; record=count);

        // discarded while transmission

        // logging the number of dropped packets from the deciders point of view
        @signal[sigMacPacketDropped](type=bool);
        @statistic[doppedMacPackets](source=sigMacPacketDropped; record=count);

        // logging the number of packets not received because we were in tx
        @signal[sigRecWhileSend](type=bool);
        @statistic[recWhileSend](source=sigRecWhileSend; record=count; title="Mac packets which were not received because there was an ongoing transmission.");

        // logging the number of packets received with an error
	// which means the sender was too far away from the receiver
        @signal[sigMacPacketReceivedWithError](type=bool);

        // discarded after transmission

        // logging the number of discarded received MAC packets
        @signal[sigReceivedMacPacketDiscarded](type=bool);
        @statistic[discardedReceivedMacPackets](source=sigReceivedMacPacketDiscarded; record=count);

        // logging the number of discarded received IVC packets
        @signal[sigReceivedIvcPacketDiscarded](type=bool);
        @statistic[discardedReceivedIvcPackets](source=sigReceivedIvcPacketDiscarded; record=count);

	// transmission times

        // logging the complete transmission time of a MAC packet
        @signal[sigMacPacketTransmissionTime](type=simtime_t);
        @statistic[macPacketTransmissionTime](source=sigMacPacketTransmissionTime; record=stats,vector?);

        // logging the complete transmission time of an IVC packet
        // this time equals the data transmission time
        @signal[sigIvcPacketTransmissionTime](type=simtime_t);
        @statistic[ivcPacketTransmissionTime](source=sigIvcPacketTransmissionTime; record=stats,vector?);

	// logging the IVC packet in queue waiting time
	// basically, tdma delay
        @signal[sigIvcPacketQueueWaitingTime](type=simtime_t);
        @statistic[ivcPacketQueueWaitingTime](source=sigIvcPacketQueueWaitingTime; record=stats,vector?);

	// logging the delay introduced by the csma
	@signal[sigCsmaDelay](type=simtime_t);
	@statistic[csmaDelay](source=sigCsmaDelay; record=stats,vector?);

	// communication control

        // logging the current amount of IVC packets in queue
        @signal[sigCurrentIvcPacketsInQueue](type=long);
        @statistic[currentIvcPacketsInQueue](source=sigCurrentIvcPacketsInQueue; record=stats,vector?);

        // logging the number of received rvc information which is wrong
        @signal[sigReceivedWrongRvcInformation](type=bool);
        @statistic[ivcPacketsWithWrongRvcInformation](source=sigReceivedWrongRvcInformation; record=count);

        // logging the distributed space between to mac packets
        @signal[sigDistributedSpace](type=long);
        @statistic[distributedSpace](source=sigDistributedSpace; record=stats; title="Distributed space between two MAC packets."; unit=us);

        // logging a hash value of the currently held rvc information
        @signal[sigCurrentRvcInformation](type=unsigned long);
        @statistic[rvcInformation](source=sigCurrentRvcInformation; record=vector; title="Currently held RVC information");

	// logging a hash value of the currently held rvc information (TRC)
        @signal[sigCurrentRvcTcInformation](type=unsigned long);
        @statistic[rvcInformationTc](source=sigCurrentRvcTcInformation; record=vector; title="Currently held RVC information");

        // logging the current 1s timer
        @signal[sigTimer](type=simtime_t);
        @statistic[timerValue](source=sigTimer; record=vector; title="The current 1s timer value");
        
	// miscellaneous
}
