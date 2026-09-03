#pragma once

// ============================================================
//  Standard library
// ============================================================
#include <veins/modules/application/ieee80211p/ApplicationToVanetzaConverter.h>
#include <veins/modules/application/ieee80211p/VanetzaToVeinsBridge.h>
#include <map>
#include <memory>
#include <cmath>
#include <chrono>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <set>

// ============================================================
//  Veins / OMNeT++ includes
// ============================================================
#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/modules/messages/DemoServiceAdvertisement_m.h"
#include "veins/modules/messages/DemoSafetyMessage_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/DemoBaseApplLayerToMac1609_4Interface.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

// ============================================================
//  Application messages (CAM / DENM)
// ============================================================
#include "veins/modules/messages/CamMessage_m.h"
#include "veins/modules/messages/DenmMessage_m.h"

// ============================================================
//  Vanetza (ASN.1) — used to build and encode CAM / DENM
//  structures that are serialized into ByteBuffer and copied
//  into the OMNeT++ message payload (vanetzaPayload).
// ============================================================
#include "vanetza/asn1/cam.hpp"
#include "vanetza/asn1/denm.hpp"
#include "vanetza/asn1/asn1c_wrapper.hpp"
#include "vanetza/common/clock.hpp"
#include "vanetza/units/angle.hpp"
#include "vanetza/units/velocity.hpp"
#include "vanetza/units/acceleration.hpp"
#include "vanetza/asn1/its/CauseCodeType.h"
#include "vanetza/asn1/its/AccidentSubCauseCode.h"

// ============================================================
//  ApplicationToVanetzaConverter
// ============================================================

namespace veins {

using veins::AnnotationManager;
using veins::AnnotationManagerAccess;
using veins::TraCICommandInterface;
using veins::TraCIMobilityAccess;

// ============================================================
//  DemoBaseApplLayer
//
//  Application layer for IEEE 802.11p / ITS-G5 nodes.
//  Implements the CAM and DENM transmission/reception flows:
//
//  CAM TX flow:
//    initialize() → schedules sendCamEvt (vehicles only)
//    handleSelfMsg(SEND_CAM_EVT) → creates CamMessage, calls
//      populateWSM() → buildCam() builds the ASN.1 structure,
//      vanetza encodes it into ByteBuffer → copied to payload →
//      sendDown() + checkAndTrackPacket() → MAC layer
//
//  CAM RX flow:
//    handleLowerMsg() → onCAM() → cam.decode(buffer) →
//      deserializes and extracts stationID, position, speed
//
//  DENM TX flow (RSU — periodic):
//    initialize() → schedules sendDenmEvt at denmStartTime
//    handleSelfMsg(SEND_DENM_EVT) → creates DenmMessage, calls
//      populateWSM() → buildDenm() builds the ASN.1 structure,
//      vanetza encodes it → copied to payload, CauseCode set →
//      sendDown() + checkAndTrackPacket() → MAC layer →
//      rescheduled periodically until denmStopTime (RSU only)
//
//  DENM TX flow (vehicle — event-driven):
//    handlePositionUpdate() → evaluates trigger condition →
//    triggerDenm() → validates event and rate-limits →
//    sendDenmNow() → cancels pending event, schedules
//      sendDenmEvt immediately (one-shot, no reschedule)
//
//  DENM RX flow:
//    handleLowerMsg() → onDENM() → denm.decode(buffer) →
//      deserializes and extracts causeCode, subCauseCode
// ============================================================
class VEINS_API DemoBaseApplLayer : public BaseApplLayer {
public:
    ~DemoBaseApplLayer() override;

    // --------------------------------------------------------
    //  OMNeT++ lifecycle
    // --------------------------------------------------------

    /// Stage 0: initializes mobility, MAC, parameters, self-message timers.
    /// Stage 1: schedules first CAM event (vehicles) and first DENM event (RSU).
    void initialize(int stage) override;

    /// Records TX/RX statistics scalars at end of simulation.
    void finish() override;

    /// Dispatches mobility, parking and collision signals to the appropriate handlers.
    void receiveSignal(cComponent* source, simsignal_t signalID,
                       cObject* obj, cObject* details) override;

    // --------------------------------------------------------
    //  Self-message event kinds
    // --------------------------------------------------------
    enum DemoApplMessageKinds {
        SEND_BEACON_EVT, ///< Periodic BSM/beacon transmission
        SEND_WSA_EVT,    ///< Periodic WSA transmission
        SEND_CAM_EVT,    ///< Periodic CAM transmission (vehicles only)
        SEND_DENM_EVT    ///< DENM transmission (periodic for RSU, one-shot for vehicles)
    };


protected:
    // --------------------------------------------------------
    //  Mobility & infrastructure
    // --------------------------------------------------------

    BaseMobility*                           mobility     = nullptr; ///< Underlying mobility module
    TraCICommandInterface*                  traci        = nullptr; ///< TraCI command interface (vehicles only)
    TraCICommandInterface::Vehicle*         traciVehicle = nullptr; ///< Per-vehicle TraCI interface
    AnnotationManager*                      annotations  = nullptr; ///< Annotation manager for visualization
    DemoBaseApplLayerToMac1609_4Interface*  mac          = nullptr; ///< Interface to MAC 802.11p layer

    /// Pointer to the ApplicationToVanetzaConverter submodule in the host node.
    /// Resolved in initialize() via FindModule; provides kinematic data for CAM/DENM construction.
    ApplicationToVanetzaConverter* applicationToVanetzaConverter = nullptr;


    /// Adapter module bridging the Veins application layer and the NIC.
    /// Created in initialize() and handles encoding/decoding of V2X messages (CAM, DENM) via Vanetza.
    VanetzaToVeinsBridge* mAdapter = nullptr;

    // --------------------------------------------------------
    //  Vehicle state
    // --------------------------------------------------------

    bool isParked = false; // = true when the vehicle is in parking state (TraCI) and it can be used for other logic

    // --------------------------------------------------------
    //  DENM event tracking
    // --------------------------------------------------------

    /// Set of currently active DENM cause codes (used to detect event end).
    std::set<int> activeDenmEvents;

    // --------------------------------------------------------
    //  BSM / Beacon parameters (from .ini)
    // --------------------------------------------------------
    uint32_t  beaconLengthBits   = 0;
    uint32_t  beaconUserPriority = 0;
    simtime_t beaconInterval     = 0;
    bool      sendBeacons        = false;

    // --------------------------------------------------------
    //  Generic WSM / data parameters (from .ini)
    // --------------------------------------------------------
    uint32_t dataLengthBits   = 0;
    uint32_t dataUserPriority = 0;
    bool     dataOnSch        = false;

    // --------------------------------------------------------
    //  DENM parameters (from .ini)
    // --------------------------------------------------------
    uint32_t  denmLengthBits     = 0;
    uint32_t  denmUserPriority   = 0;
    uint16_t  denmSequenceNumber = 0; ///< Incremented at each DENM build in populateWSM()

    // --------------------------------------------------------
    //  WSA / service parameters
    // --------------------------------------------------------
    int         currentOfferedServiceId = 0;
    std::string currentServiceDescription;
    Channel     currentServiceChannel;
    simtime_t   wsaInterval = 0;

    // --------------------------------------------------------
    //  Runtime state
    // --------------------------------------------------------
    Coord            curPosition; ///< Updated on every mobilityStateChangedSignal
    Coord            curSpeed;    ///< Updated on every mobilityStateChangedSignal
    LAddress::L2Type myId  = 0;
    int              mySCH = 0;

    // --------------------------------------------------------
    //  Statistics counters (recorded in finish())
    // --------------------------------------------------------
    uint32_t generatedWSMs  = 0;
    uint32_t generatedWSAs  = 0;
    uint32_t generatedBSMs  = 0;
    uint32_t generatedCAMs  = 0;  ///< Incremented by checkAndTrackPacket() on each CAM TX
    uint32_t generatedDENMs = 0;  ///< Incremented by checkAndTrackPacket() on each DENM TX
    uint32_t receivedWSMs   = 0;
    uint32_t receivedWSAs   = 0;
    uint32_t receivedBSMs   = 0;
    uint32_t receivedCAMs   = 0;  ///< Incremented in handleLowerMsg() on each CAM RX
    uint32_t receivedDENMs  = 0;  ///< Incremented in handleLowerMsg() on each DENM RX

    // --------------------------------------------------------
    //  Self-message timers
    // --------------------------------------------------------
    cMessage* sendBeaconEvt = nullptr;
    cMessage* sendWSAEvt    = nullptr;
    cMessage* sendCamEvt    = nullptr;  ///< Scheduled in initialize() for vehicles; periodic 1 Hz
    cMessage* sendDenmEvt   = nullptr;  ///< Scheduled in initialize() for RSU; one-shot for vehicles

    // --------------------------------------------------------
    //  DENM rate limiting
    // --------------------------------------------------------
    simtime_t denmMinInterval  = 1.0;         ///< Minimum time between two consecutive DENMs
    simtime_t lastDenmTime     = SIMTIME_ZERO; ///< Timestamp of the last DENM sent
    int       denmDefaultCause    = 0;
    int       denmDefaultSubcause = 0;

    // ========================================================
    //  TX PATH
    // ========================================================

    // --------------------------------------------------------
    //  TX — Self-message dispatcher
    // --------------------------------------------------------

    /// Handles self-scheduled events: SEND_CAM_EVT, SEND_DENM_EVT, SEND_BEACON_EVT, SEND_WSA_EVT.
    void handleSelfMsg(cMessage* msg) override;

    // --------------------------------------------------------
    //  TX — Message population
    // --------------------------------------------------------

    /**
     * Fills the fields of a BaseFrame1609_4 subclass.
     * - CamMessage:  calls buildCam(), encodes via vanetza, copies ByteBuffer → vanetzaPayload
     * - DenmMessage: calls buildDenm(), encodes via vanetza, copies ByteBuffer → vanetzaPayload
     * - BSM / WSA / generic WSM: fills standard Veins fields
     */
    virtual void populateWSM(BaseFrame1609_4* wsm,
                              LAddress::L2Type rcvId = LAddress::L2BROADCAST(),
                              int serial = 0);

    // --------------------------------------------------------
    //  TX — DENM event-driven flow (vehicles)
    // --------------------------------------------------------

    /**
     * Called on every mobilityStateChangedSignal.
     * Updates curPosition / curSpeed, lazily creates applicationToVanetzaConverter,
     * and evaluates DENM trigger conditions for vehicles.
     */
    virtual void handlePositionUpdate(cObject* obj);

    /// Updates isParked flag from TraCIMobility parking state.
    virtual void handleParkingUpdate(cObject* obj);

    /**
     * Validates the event, applies rate limiting (denmMinInterval),
     * registers the cause in activeDenmEvents, then calls sendDenmNow().
     * Called from handlePositionUpdate() when a trigger condition is met,
     * or from receiveSignal() on parking/collision events.
     */
    virtual void triggerDenm(CauseCodeType_t eventCause,
                              int cause         = -1,
                              int subcause      = -1,
                              const Coord*      eventPos      = nullptr,
                              bool              sendImmediate = true);

    /**
     * Cancels any pending sendDenmEvt and reschedules it immediately.
     * handleSelfMsg() will then call populateWSM() → buildDenm() → sendDown().
     * For vehicles this is one-shot; for RSU the periodic reschedule
     * is handled inside handleSelfMsg(SEND_DENM_EVT).
     */
    virtual void sendDenmNow(CauseCodeType_t eventCause,
                             int cause, int subcause,
                             const Coord& eventPos);

    // --------------------------------------------------------
    //  TX — Send to MAC layer
    // --------------------------------------------------------

    /// Calls checkAndTrackPacket() then forwards to BaseApplLayer::sendDown().
    virtual void sendDown(cMessage* msg);
    virtual void sendDelayedDown(cMessage* msg, simtime_t delay);

    /// Increments the appropriate generated* counter based on message type.
    virtual void checkAndTrackPacket(cMessage* msg);

    // --------------------------------------------------------
    //  TX — WSA service management
    // --------------------------------------------------------
    virtual void startService(Channel channel, int serviceId,
                               std::string serviceDescription);
    virtual void stopService();

    // ========================================================
    //  RX PATH
    // ========================================================

    // --------------------------------------------------------
    //  RX — Lower message dispatcher
    // --------------------------------------------------------

    /// Dispatches incoming messages from MAC to the appropriate on*() handler.
    void handleLowerMsg(cMessage* msg) override;

    // --------------------------------------------------------
    //  RX — Message callbacks
    // --------------------------------------------------------

    virtual void onWSM(BaseFrame1609_4* wsm) {}
    virtual void onBSM(DemoSafetyMessage* bsm) {}
    virtual void onWSA(DemoServiceAdvertisment* wsa) {}

    /// CAM RX: decodes vanetzaPayload via cam.decode(buffer), extracts stationID/position/speed.
    //virtual void onCAM(CamMessage* cam);

    /// DENM RX: decodes vanetzaPayload via denm.decode(buffer), extracts causeCode/subCauseCode.
    //virtual void onDENM(DenmMessage* denm);

    // ========================================================
    //  Utility
    // ========================================================

    /// Computes a desynchronized first-send time to avoid beacon collisions.
    virtual simtime_t computeAsynchronousSendingTime(simtime_t interval,
                                                      ChannelType chantype);
};

} // namespace veins
