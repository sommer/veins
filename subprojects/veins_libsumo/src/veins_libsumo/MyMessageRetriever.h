//
// Copyright (C) 2006-2020 Christoph Sommer <sommer@cms-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
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

#pragma once

// SUMO includes
#include "utils/geom/PositionVector.h"
#include "libsumo/Simulation.h"
#include "libsumo/Vehicle.h"
#include "utils/common/MsgHandler.h"
#include "utils/common/MsgRetrievingFunction.h"

namespace veins {

class MyMessageRetriever {
protected:
    OutputDevice* myMessageRetriever;
    OutputDevice* myWarningRetriever;
    OutputDevice* myErrorRetriever;
    OutputDevice* myDebugRetriever;
    OutputDevice* myGLDebugRetriever;

public:
    void init()
    {
        // suppress all console output
        MsgHandler::getMessageInstance()->removeRetriever(&OutputDevice::getDevice("stdout"));
        MsgHandler::getWarningInstance()->removeRetriever(&OutputDevice::getDevice("stderr"));
        MsgHandler::getErrorInstance()->removeRetriever(&OutputDevice::getDevice("stderr"));

        myMessageRetriever = new MsgRetrievingFunction<MyMessageRetriever>(this, &MyMessageRetriever::retrieveMessage, MsgHandler::MT_MESSAGE);
        myWarningRetriever = new MsgRetrievingFunction<MyMessageRetriever>(this, &MyMessageRetriever::retrieveMessage, MsgHandler::MT_WARNING);
        myErrorRetriever = new MsgRetrievingFunction<MyMessageRetriever>(this, &MyMessageRetriever::retrieveMessage, MsgHandler::MT_ERROR);
        myDebugRetriever = new MsgRetrievingFunction<MyMessageRetriever>(this, &MyMessageRetriever::retrieveMessage, MsgHandler::MT_DEBUG);
        myGLDebugRetriever = new MsgRetrievingFunction<MyMessageRetriever>(this, &MyMessageRetriever::retrieveMessage, MsgHandler::MT_GLDEBUG);

        MsgHandler::getMessageInstance()->addRetriever(myMessageRetriever);
        MsgHandler::getWarningInstance()->addRetriever(myWarningRetriever);
        MsgHandler::getErrorInstance()->addRetriever(myErrorRetriever);
        MsgHandler::getDebugInstance()->addRetriever(myDebugRetriever);
        MsgHandler::getGLDebugInstance()->addRetriever(myGLDebugRetriever);
    }

    void
    retrieveMessage(const MsgHandler::MsgType type, const std::string& msg)
    {

        std::string s = msg;
        s.erase(std::remove(s.end() - 1, s.end(), '\n'), s.end());
        s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
        std::replace(s.begin(), s.end(), '\n', '/');

        std::string t = "UNKNOWN";
        switch (type) {
        case MsgHandler::MT_MESSAGE:
            t = "default";
            break;
        case MsgHandler::MT_WARNING:
            t = "WARNING";
            break;
        case MsgHandler::MT_ERROR:
            t = "ERROR";
            break;
        case MsgHandler::MT_DEBUG:
            t = "DEBUG";
            break;
        case MsgHandler::MT_GLDEBUG:
            t = "GLDEBUG";
            break;
        }

        EV_DEBUG << "message from libsumo [" << t << "]: \"" << s << "\"" << std::endl;
    }

    void done()
    {
        delete myMessageRetriever;
        delete myWarningRetriever;
        delete myErrorRetriever;
        delete myDebugRetriever;
        delete myGLDebugRetriever;
    }
};

} // namespace veins
