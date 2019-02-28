# Simulation Signal Management {#simsignal_management}

[TOC]

OMNeT++ provides the *Simulation Signals* (or just *Signals* for short) as a publish-subscribe mechanism in the simulation.
Assume some signal emitting Module like this:

```{.cpp}
// module emitting some signal whenever it receives a message (contrived example)
class SignalEmitter: public cModule {
public:
    static const simsignal_t counterSignal;
    static const simsignal_t nameSignal;
protected:
    long messageCounter = 0;
    void handleMessage(cMessage *msg) {
        emit(counterSignal, messageCounter);
        messageCounter++;
        emit(nameSignal, msg->str());
        // ...
    }
};

const simsignal_t SignalEmitter::counterSignal = registerSignal("messageCounter");
const simsignal_t SignalEmitter::nameSignal = registerSignal("messageName");
```

## Using plain OMNeT++ signals
While signals are easy to configure and emit, reacting to them involves a lot of boilerplate code:

```{.cpp}
// module reacting to signals in plain OMNeT++ fashion, has to inherit from cListener or implement cIListener
class WithoutSignalCallbacks: public cModule, public cListener {
protected:
    void initialize() override {
        // subscribe to the signal with itself as handler
        subscribe(SignalEmitter::counterSignal, this);
        subscribe(SignalEmitter::nameSignal, this);
    }
public:
    // signal handler for all signals with a long parameter
    void receiveSignal(cComponent* source, simsignal_t signalID, long l, cObject* details) override {
        // identify signal to handle
        if (signalID == SignalEmitter::counterSignal) {
            // react to signal
            std::cerr << "Module " << source->getFullName() << " received message nr " << l << std::endl;
        }
    }
    // signal handler for all signals with a string parameter
    void receiveSignal(cComponent* source, simsignal_t signalID, const char* s, cObject* details) override {
        // identify signal to handle
        if (signalID == SignalEmitter::nameSignal) {
            // react to signal
            std::cerr << "Module " << source->getFullName() << " received message with content " << c << std::endl;
        }
    }
};
```
## SignalManager
With the [SignalManager], all signal handling can be encapsulated and configuration will stay in one place:

```{.cpp}
// module reacting to signals using the SignalManager
class WithSignalManager: public cModule {
protected:
    veins::SignalManager signalManager(this);
    void initialize() override {
        // reaction to the signal (note how unused parameters are omitted from the lambda function signature).
        auto nameSignalCallback = [this](cComponent *source, const char* c) {
            std::cerr << "Module " << source->getFullName() << " received message with content " << c << std::endl;
        };
        auto counterSignalCallback = [this](cComponent* source, simsignal_t signalID, long l) {
            std::cerr << "Module " << source->getFullName() << " received message nr " << l << " via signal " << signalID << std::endl;
        };
        // register callbacks with the signal manager, which takes care to perform the actual subscription
        signalManager.subscribeCallback(getSystemModule(), SignalEmitter::nameSignal, nameSignalCallback);
        signalManager.subscribeCallback(getSystemModule(), SignalEmitter::counterSignal, counterSignalCallback);
    }
};
```

## SignalCallback
Alternatively, we could omit the manager part and make create individual [SignalCallback] objects for each registered callback:
```{.cpp}
// module reacting to signals using the SignalCallback instances
class WithSignalCallbacks: public cModule {
protected:
    veins::SignalCallback counterCallback;
    veins::SignalCallback nameCallback;
    void initialize() override {
        // reaction to the signal (note how unused parameters are omitted from the lambda function signature).
        auto nameSignalCallback = [this](cComponent *source, const char* c) {
            std::cerr << "Module " << source->getFullName() << " received message with content " << c << std::endl;
        };
        auto counterSignalCallback = [this](cComponent* source, simsignal_t signalID, long l) {
            std::cerr << "Module " << source->getFullName() << " received message nr " << l << " via signal " << signalID << std::endl;
        };
        // register callback objects, which takes care to perform the actual subscription
        counterCallback = veins::SignalCallback::make(getSystemModule(), SignalEmitter::counterSignal, counterSignalCallback);
        nameCallback = veins::SignalCallback::make(getSystemModule(), SignalEmitter::nameSignal, nameCallback);
    }
};
```

This allows a closer control over the lifetime of each individual [SignalCallback] instance.

## Common Features

With both [SignalManager] and [SignalCallback], there is no need to implement the cIListener yourself.
The lifetime is managed automatically and the Code is more self-explanatory and kept in one place.
In addtion to Lambdas, other forms of callbacks can be used, too, such as free functions and functors.

As shown in the examples, the callbacks can also implement just a subset of the cIListener::receiveSignal signature.
Valid subsets can be seen by checking the implemented overloads of the @ref veins::SignalCallbackManagement::invokeCallback template.

[SignalManager]: @ref veins::SignalManager
[SignalCallback]: @ref veins::SignalCallback
