/* -*- mode:c++ -*- ********************************************************
 * file:        SingleChannelRadio.h
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 ***************************************************************************/

#ifndef SINGLECHANNELRADIO_H
#define SINGLECHANNELRADIO_H

#include <omnetpp.h>
#include <BasicModule.h>
#include <ModuleAccess.h>
#include <ActiveChannel.h>
#include "RadioState.h"
#include "Bitrate.h"

/**
 * @brief A simple radio being able to send / transmit on one channel
 *
 * @ingroup radios
 * @author Andreas Koepke
 **/
class SingleChannelRadio : public BasicModule
{
    Module_Class_Members(SingleChannelRadio,BasicModule,0);

 protected:
    /** @brief hold radio state */
    RadioState state;
    /** @brief channel radio category */
    int stateCat;

    /** @brief holds active channel, default channel is set by MAC protocol */
    ActiveChannel aChannel;
    /** @brief active channel category */
    int aChannelCat;

    /** @brief current bit rate for this radio, default is set by MAC */
    Bitrate bitrate;
    /** @brief current bit rate and the category number */
    int bitrateCat;

    /** variable that holds the time the radio needs to switch
     * to a certain state
     *@{
     */
    simtime_t swSleep;
    simtime_t swSend;
    simtime_t swRecv;

    cMessage *timer;
    /** @} */
    
    /** semaphore to prevent changing of radio during a change */
    bool handlingTimer;

    /** id of the surrounding nic module */
    int nicModuleId;
    
protected:
    /**
     * perform switch if possible
     */
    bool switchTo(RadioState::States, simtime_t delta);
    
public:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();
    
    /** @brief should not be called,
     *  instead direct calls to the radio methods should be used.
     */
    virtual void handleMessage( cMessage* );
    
    /**
     * Methods to control state of this radio
     * returns whether state was actually changed
     */
    bool switchToSleep();
    bool switchToSend();
    bool switchToRecv();

    /** @brief change the active channel, never fails (returns true). */
    bool setActiveChannel(int c);

    /** @brief change the bit rate, never fails (returns true) */
    bool setBitrate(double b);
};

class  SingleChannelRadioAccess : public ModuleAccess<SingleChannelRadio>
{
    public:
        SingleChannelRadioAccess() : ModuleAccess<SingleChannelRadio>("radio") {}
};

#endif
