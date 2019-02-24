//
// Copyright (C) 2019-2019 Dominik S. Buse <buse@ccs-labs.org>
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
#pragma once

#include "veins/veins.h"

#include <functional>
#include <memory>
#include <typeinfo>

#include "veins/modules/utility/CallableInfo.h"

namespace veins {

namespace SignalCallbackManagement {

// Generic case: error
template <typename Payload, typename Callback>
void invokeCallback(Callback callback, cComponent* source, simsignal_t signalID, Payload p, cObject* details)
{
    std::string msg = std::string("Invalid signal subscription for callback with payload type: ") + typeid(Payload).name();
    throw cRuntimeError(msg.c_str());
}

// Specific cases (payload matches callback) -> actually invoke callback

// No arguments at all
template <typename Payload>
void invokeCallback(std::function<void(void)> callback, cComponent* source, simsignal_t signalID, Payload p, cObject* details)
{
    callback();
}

// Only payload
template <typename Payload>
void invokeCallback(std::function<void(Payload p)> callback, cComponent* source, simsignal_t signalID, Payload p, cObject* details)
{
    callback(p);
}

// source and payload
template <typename Payload>
void invokeCallback(std::function<void(cComponent*, Payload p)> callback, cComponent* source, simsignal_t signalID, Payload p, cObject* details)
{
    callback(source, p);
}

// source, signalID and payload
template <typename Payload>
void invokeCallback(std::function<void(cComponent*, simsignal_t, Payload p)> callback, cComponent* source, simsignal_t signalID, Payload p, cObject* details)
{
    callback(source, signalID, p);
}

// full information (source, signalId, payload and details)
template <typename Payload>
void invokeCallback(std::function<void(cComponent*, simsignal_t, Payload p, cObject*)> callback, cComponent* source, simsignal_t signalID, Payload p, cObject* details)
{
    callback(source, signalID, p, details);
}

template <typename Payload, typename Callback>
class CallbackListener : public cListener {
public:
    CallbackListener(Callback callback, cModule* receptor, simsignal_t signal)
        : callback(callback)
        , receptor(receptor)
        , signal(signal)
    {
        receptor->subscribe(signal, this);
    }

    ~CallbackListener()
    {
        receptor->unsubscribe(signal, this);
    }

    void receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) override
    {
        invokeCallback<bool>(callback, source, signalID, b, details);
    }

    void receiveSignal(cComponent* source, simsignal_t signalID, long l, cObject* details) override
    {
        invokeCallback<long>(callback, source, signalID, l, details);
    }

    void receiveSignal(cComponent* source, simsignal_t signalID, unsigned long l, cObject* details) override
    {
        invokeCallback<unsigned long>(callback, source, signalID, l, details);
    }

    void receiveSignal(cComponent* source, simsignal_t signalID, double d, cObject* details) override
    {
        invokeCallback<double>(callback, source, signalID, d, details);
    }

    void receiveSignal(cComponent* source, simsignal_t signalID, const SimTime& t, cObject* details) override
    {
        invokeCallback<const SimTime&>(callback, source, signalID, t, details);
    }

    void receiveSignal(cComponent* source, simsignal_t signalID, const char* c, cObject* details) override
    {
        invokeCallback<const char*>(callback, source, signalID, c, details);
    }

    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* c, cObject* details) override
    {
        invokeCallback<cObject*>(callback, source, signalID, c, details);
    }

private:
    const Callback callback;
    cModule* const receptor;
    const simsignal_t signal;
};

} // namespace SignalCallbackManagement

class SignalManager;

/*
 * Handle for a callback subscribed to a signal.
 */
class SignalCallback {
protected:
    friend class SignalManager;
    template <typename Payload, typename Func>
    static SignalCallback make(cModule* receptor, simsignal_t signal, const Func callback)
    {
        // Note: we have to covert to the target function layout here to allow passing lambdas as callback
        // (lambdas can only be *converted* to functions, but template deduction needs exactly matching types).
        using CallbackType = typename CallableInfo::type<Func>::function_equivalent;
        using CallbackListener = SignalCallbackManagement::CallbackListener<Payload, CallbackType>;

        const CallbackType converted = callback;
        return SignalCallback(make_unique<CallbackListener>(converted, receptor, signal));
    }

private:
    SignalCallback(std::unique_ptr<cIListener> listener)
        : listener(std::move(listener))
    {
    }
    std::unique_ptr<cIListener> listener;
};

/*
 * Semantic encapsulation for OMNeT++ simulation signals usinng callbacks.
 *
 * TODO: Extend documentation.
 */
class VEINS_API SignalManager {
public:
    /*
     * Create a subscription to signal that triggers callback.
     */
    template <typename Payload, typename Func>
    void subscribeCallback(cModule* receptor, omnetpp::simsignal_t signal, const Func callback)
    {
        auto signalCallback = SignalCallback::make<Payload, Func>(receptor, signal, callback);
        storedCallbacks.push_back(std::move(signalCallback));
    }

private:
    std::vector<SignalCallback> storedCallbacks;
};

} // namespace veins
