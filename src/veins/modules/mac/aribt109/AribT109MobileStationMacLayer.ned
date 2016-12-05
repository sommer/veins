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
simple AribT109MobileStationMacLayer extends BaseMacLayer like IAribT109Mac
{
    parameters:
        @class(AribT109MobileStationMacLayer);

        double carrierFrequency @unit(MHz) = default(760 MHz);

        // bit rate 
        double bitrate @unit(bps) = default(18 Mbps);

        // little asynchronization between cars
        double syncOffset @unit(s) = default(4 ms);

        //tx power [mW]
        double txPower @unit(mW) = default(10 mW);

        double ccaThreshold @unit(dBm) = default(-53 dBm);

         // Roadside-to-Vehicle period information valid time
         // ORV
         // Represents the valid time of the received timestamp and the roadside period information.
         // The value is in ms.
         // The received roadside period information shall be updated every time this field elapses
         // Values 300..65536
        int roadsidePeriodInformationValidTime @unit(ms) = default(300 ms);

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
        @statistic[distributedSpace](source=sigDistributedSpace; record=stats,vector?; title="Distributed space between two MAC packets."; unit=us);

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

        // logging the current sync status
        @signal[sigSyncStatus](type=long);
        @statistic[syncStatus](source=sigSyncStatus; record=vector; title="Current synchronization status");

        // logging the corrected timer value
        @signal[sigCorrectTimerByValue](type=simtime_t);
        @statistic[correctTimerByValue](source=sigCorrectTimerByValue; record=stats,vector?; title="Corrected timer value"; unit=s);

        // logging a hash value of the currently held rvc information
        @signal[sigCurrentRvcInformation](type=unsigned long);
        @statistic[rvcInformation](source=sigCurrentRvcInformation; record=vector; title="Currently held RVC information");

	// logging a hash value of the currently held rvc information (TRC)
        @signal[sigCurrentRvcTcInformation](type=unsigned long);
        @statistic[rvcInformationTc](source=sigCurrentRvcTcInformation; record=vector; title="Currently held RVC information");

        // logging the current 1s timer
        @signal[sigTimer](type=simtime_t);
        @statistic[timerValue](source=sigTimer; record=vector; title="The current 1s timer value");

        // loggind invalid control fields in ivc packets
        @signal[sigInvalidControlFieldReceived](type=bool);
        @statistic[receivedInvalidControlFields](source=sigInvalidControlFieldReceived; record=count; title="Received invalid control fields in IVC packet");

	// miscellaneous
}
