/*
 * BaseDecider.cc
 *
 *  Created on: 24.02.2009
 *      Author: karl
 */

#include "veins/base/phyLayer/BaseDecider.h"
#include "veins/base/messages/AirFrame_m.h"

using namespace Veins;

simtime_t BaseDecider::processSignal(AirFrame* frame)
{

    ASSERT(frame);
    EV_TRACE << "Processing AirFrame..." << endl;

    switch (getSignalState(frame)) {
    case NEW:
        return processNewSignal(frame);
    case EXPECT_HEADER:
        return processSignalHeader(frame);
    case EXPECT_END:
        return processSignalEnd(frame);
    default:
        return processUnknownSignal(frame);
    }
}

simtime_t BaseDecider::processNewSignal(AirFrame* frame)
{
    if (currentSignal.first != 0) {
        EV_TRACE << "Already receiving another AirFrame!" << endl;
        return notAgain;
    }

    // get the receiving power of the Signal at start-time
    Signal& signal = frame->getSignal();
    double recvPower = signal.getMax();

    // check whether signal is strong enough to receive
    if (recvPower < minPowerLevel) {
        EV_TRACE << "Signal is too weak (" << recvPower << " < " << minPowerLevel << ") -> do not receive." << endl;
        // Signal too weak, we can't receive it, tell PhyLayer that we don't want it again
        return notAgain;
    }

    // Signal is strong enough, receive this Signal and schedule it
    EV_TRACE << "Signal is strong enough (" << recvPower << " > " << minPowerLevel << ") -> Trying to receive AirFrame." << endl;

    currentSignal.first = frame;
    currentSignal.second = EXPECT_END;

    // channel turned busy
    setChannelIdleStatus(false);

    return signal.getReceptionEnd();
}

simtime_t BaseDecider::processSignalEnd(AirFrame* frame)
{
    EV_INFO << "packet was received correctly, it is now handed to upper layer...\n";
    phy->sendUp(frame, new DeciderResult(true));

    // we have processed this AirFrame and we prepare to receive the next one
    currentSignal.first = 0;

    // channel is idle now
    setChannelIdleStatus(true);

    return notAgain;
}

simtime_t BaseDecider::processUnknownSignal(AirFrame* frame)
{
    throw cRuntimeError("Unknown state for the AirFrame with ID %d", frame->getId());
    return notAgain;
}

int BaseDecider::getSignalState(AirFrame* frame)
{
    if (frame == currentSignal.first) return currentSignal.second;

    return NEW;
}

void BaseDecider::setChannelIdleStatus(bool isIdle)
{
    isChannelIdle = isIdle;
}

void BaseDecider::getChannelInfo(simtime_t_cref start, simtime_t_cref end, AirFrameVector& out)
{
    phy->getChannelInfo(start, end, out);
}
