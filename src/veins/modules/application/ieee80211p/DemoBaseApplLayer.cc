#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"

using namespace veins;

// ============================================================
//  DemoBaseApplLayer — implementation
// ============================================================

// ============================================================
//  OMNeT++ lifecycle
// ============================================================

// Stage 0: resolves mobility, TraCI, MAC and annotation modules;
//          reads all .ini parameters; creates self-message timers;
//          subscribes to mobility/parking/collision signals.
// Stage 1: assigns MAC address; schedules first CAM event for
//          vehicles (TraCIMobility nodes) and first DENM event
//          for RSU nodes (non-TraCIMobility) using denmStartTime.
void DemoBaseApplLayer::initialize(int stage)
{
    BaseApplLayer::initialize(stage);

    if (stage == 0) {
        // --- Resolve mobility module ---
        auto found = FindModule<BaseMobility*>::findSubModule(findHost());
        if (found) {
            mobility = static_cast<BaseMobility*>(found);
            if (auto tr = dynamic_cast<veins::TraCIMobility*>(found)) {
                traci        = tr->getCommandInterface();
                traciVehicle = tr->getVehicleCommandInterface();
            } else {
                traci        = nullptr;
                traciVehicle = nullptr;
            }
        } else {
            traci        = nullptr;
            mobility     = nullptr;
            traciVehicle = nullptr;
        }

        // Resolve ApplicationToVanetzaConverter as an OMNeT++ submodule in the host node
        applicationToVanetzaConverter = FindModule<ApplicationToVanetzaConverter*>::findSubModule(getParentModule());
        if (!applicationToVanetzaConverter) {
            EV_WARN << "ApplicationToVanetzaConverter submodule not found in host "
                    << getParentModule()->getFullName() << " — CAM/DENM generation may fail\n";
        }

        // Create the Adapter that handles serialization/deserialization between Veins and Vanetza
        mAdapter = FindModule<VanetzaToVeinsBridge*>::findSubModule(getParentModule());
        if (!mAdapter) {
            throw cRuntimeError("VanetzaToVeinsBridge submodule not found in host. Check Car.ned!");
        }

        // Access the annotation manager for visual debug overlays in the simulation
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        // Access the MAC layer (802.11p) for sending and receiving messages
        mac = FindModule<DemoBaseApplLayerToMac1609_4Interface*>::findSubModule(getParentModule());
        ASSERT(mac);

        // --- Read .ini parameters ---
        headerLength       = par("headerLength");
        sendBeacons        = par("sendBeacons").boolValue();
        beaconLengthBits   = par("beaconLengthBits");
        beaconUserPriority = par("beaconUserPriority");
        beaconInterval     = par("beaconInterval");

        dataLengthBits   = par("dataLengthBits");
        dataOnSch        = par("dataOnSch").boolValue();
        dataUserPriority = par("dataUserPriority");

        wsaInterval             = par("wsaInterval").doubleValue();
        currentOfferedServiceId = -1;

        denmLengthBits   = par("denmLengthBits");
        denmUserPriority = par("denmUserPriority");

        isParked           = false;
        denmSequenceNumber = 0;

        // --- Subscribe to signals ---
        findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);
        if (mobility) {
            mobility->subscribe(TraCIMobility::collisionSignal, this);
            mobility->subscribe(TraCIMobility::parkingStateChangedSignal, this);
        }

        // --- Allocate self-message timers ---
        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendWSAEvt    = new cMessage("wsa evt",    SEND_WSA_EVT);
        sendCamEvt    = new cMessage("cam evt",    SEND_CAM_EVT);
        sendDenmEvt   = new cMessage("denm evt",   SEND_DENM_EVT);

        // --- Reset statistics counters ---
        generatedBSMs  = 0; generatedWSAs  = 0; generatedWSMs  = 0;
        generatedCAMs  = 0; generatedDENMs = 0;
        receivedBSMs   = 0; receivedWSAs   = 0; receivedWSMs   = 0;
        receivedCAMs   = 0; receivedDENMs  = 0;
    }
    else if (stage == 1) {
        myId = mac->getMACAddress();

        // Disable SCH data if MAC is not doing channel switching
        if (dataOnSch == true && !mac->isChannelSwitchingActive()) {
            dataOnSch = false;
            EV_ERROR << "App wants to send data on SCH but MAC doesn't use any SCH. "
                        "Sending all data on CCH\n";
        }

        // Schedule first beacon with optional desynchronisation offset
        simtime_t firstBeacon = simTime();
        if (par("avoidBeaconSynchronization").boolValue() == true) {
            simtime_t randomOffset = dblrand() * beaconInterval;
            firstBeacon = simTime() + randomOffset;
            if (mac->isChannelSwitchingActive() == true) {
                if (beaconInterval.raw() % (mac->getSwitchingInterval().raw() * 2))
                    EV_ERROR << "Beacon interval not a multiple of switching interval\n";
                firstBeacon = computeAsynchronousSendingTime(beaconInterval, ChannelType::control);
            }
            if (sendBeacons) scheduleAt(firstBeacon, sendBeaconEvt);
        }

        // CAM TX: schedule first check at T_CheckCamGen = 100 ms (ETSI §6.1.3)
        if (applicationToVanetzaConverter && mobility && dynamic_cast<veins::TraCIMobility*>(mobility))
            scheduleAt(simTime() + 0.1, sendCamEvt);


        // DENM TX (RSU): schedule first event at denmStartTime for non-vehicle nodes
        int cause = par("denmDefaultCause");
        if (cause >= 0 && !dynamic_cast<veins::TraCIMobility*>(mobility)) {
            double startTime = hasPar("denmStartTime") ? par("denmStartTime").doubleValue() : 10.0;
            double stopTime  = hasPar("denmStopTime")  ? par("denmStopTime").doubleValue()  : 200.0;
            EV_INFO << "DENM [TRIGGER]: cause=" << cause
                    << " t=" << simTime() << endl;
            scheduleAt(simTime() + startTime, sendDenmEvt);
        }
    }
}

// Records all TX/RX counters as OMNeT++ scalars for post-simulation analysis.
void DemoBaseApplLayer::finish()
{
    recordScalar("generatedWSMs",  generatedWSMs);
    recordScalar("receivedWSMs",   receivedWSMs);
    recordScalar("generatedBSMs",  generatedBSMs);
    recordScalar("receivedBSMs",   receivedBSMs);
    recordScalar("generatedWSAs",  generatedWSAs);
    recordScalar("receivedWSAs",   receivedWSAs);
    recordScalar("generatedCAMs",  generatedCAMs);
    recordScalar("receivedCAMs",   receivedCAMs);
    recordScalar("generatedDENMs", generatedDENMs);
    recordScalar("receivedDENMs",  receivedDENMs);
}

// Cancels and deletes all self-message timers; unsubscribes from signals.
DemoBaseApplLayer::~DemoBaseApplLayer()
{
    cancelAndDelete(sendBeaconEvt);
    cancelAndDelete(sendWSAEvt);
    cancelAndDelete(sendCamEvt);
    cancelAndDelete(sendDenmEvt);

    findHost()->unsubscribe(BaseMobility::mobilityStateChangedSignal, this);
    if (mobility) {
        mobility->unsubscribe(TraCIMobility::collisionSignal, this);
        mobility->unsubscribe(TraCIMobility::parkingStateChangedSignal, this);
    }
}

// ============================================================
//  TX PATH
// ============================================================

// ------------------------------------------------------------
//  TX — Self-message dispatcher automatically called by OMNeT++
//  framework when a self-message arrives to the module that is
//  treated as an alarm for the module to send anoter message
//
//  Central entry point for all periodic and one-shot TX events.
//
//  SEND_BEACON_EVT : creates BSM, populates, sends, reschedules.
//  SEND_WSA_EVT    : creates WSA, populates, sends, reschedules.
//  SEND_CAM_EVT    : creates CamMessage, calls populateWSM()
//                    which invokes buildCam() → vanetza encode →
//                    ByteBuffer → vanetzaPayload, then sendDown().
//                    Always rescheduled every 1 s (vehicles only).
//  SEND_DENM_EVT   : creates DenmMessage, calls populateWSM()
//                    which invokes buildDenm() → vanetza encode →
//                    ByteBuffer → vanetzaPayload, then sendDown().
//                    RSU: rescheduled periodically until denmStopTime.
//                    Vehicle: one-shot, NOT rescheduled here.
// ------------------------------------------------------------
void DemoBaseApplLayer::handleSelfMsg(cMessage* msg)
{
    switch (msg->getKind()) {

    // --- BSM / Beacon ---
    case SEND_BEACON_EVT: {
        DemoSafetyMessage* bsm = new DemoSafetyMessage();
        populateWSM(bsm);
        sendDown(bsm);
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
        break;
    }

    // --- WSA ---
    case SEND_WSA_EVT: {
        DemoServiceAdvertisment* wsa = new DemoServiceAdvertisment();
        populateWSM(wsa);
        sendDown(wsa);
        scheduleAt(simTime() + wsaInterval, sendWSAEvt);
        break;
    }

    /// --- CAM TX (vehicles only) — ETSI EN 302 637-2 §6.1.3 ---
    case SEND_CAM_EVT: {
        if (!applicationToVanetzaConverter || !mobility) {
            EV_WARN << "CAM [TX]: ApplicationToVanetzaConverter not ready — retrying in 100ms"
                    << " name=" << getParentModule()->getFullName() << "\n";
            scheduleAt(simTime() + 0.1, sendCamEvt); // retry after T_CheckCamGen
            break;
        }

        // Evaluate ETSI §6.1.3 CAM generation conditions via VanetzaToVeinsBridge
        if (mAdapter->computeAndCheckCamDeltas(*applicationToVanetzaConverter)) {
            CamMessage* cam = new CamMessage();
            populateWSM(cam); // → buildCam() → vanetza encode → vanetzaPayload
            EV_INFO << "CAM [TX]: stationID=" << applicationToVanetzaConverter->station_id()
                    << " pos=(" << applicationToVanetzaConverter->latitude().value()
                    << ", "     << applicationToVanetzaConverter->longitude().value() << ")"
                    << " speed=" << applicationToVanetzaConverter->speed().value() << " m/s"
                    << " name=" << getParentModule()->getFullName()
                    << " t=" << simTime() << "\n";
            sendDown(cam);

            // Notify adapter that a CAM was sent — resets delta reference state
            mAdapter->notifyCamSent(*applicationToVanetzaConverter);
        }

        // Always reschedule at T_CheckCamGen = 100 ms (ETSI §6.1.3)
        scheduleAt(simTime() + 0.1, sendCamEvt);
        break;
    }

    // --- DENM TX ---
    case SEND_DENM_EVT: {
        if (!applicationToVanetzaConverter) {
            EV_WARN << "DENM [TX]: ApplicationToVanetzaConverter not ready — skipping"
                    << " name=" << getParentModule()->getFullName() << "\n";
            break;
        }

        // Stop condition: do not send (or reschedule) past denmStopTime
        double stopTime = hasPar("denmStopTime") ? par("denmStopTime").doubleValue() : 200.0;
        if (simTime().dbl() > stopTime) {
            EV_INFO << "DENM [TX]: stopTime=" << stopTime << "s reached — stopping"
                    << " node=" << getParentModule()->getFullName() << "\n";
            break;
        }

        DenmMessage* denm = new DenmMessage();
        populateWSM(denm); // → buildDenm() → vanetza encode → vanetzaPayload
        int cause = par("denmDefaultCause");

        EV_INFO << "DENM [TX]: cause=" << cause
                << " sender=" << getParentModule()->getFullName()
                << " t=" << simTime() << endl;
        sendDown(denm);

        // RSU: reschedule periodically until stopTime.
        // Vehicle: one-shot — sendDenmNow() already fired this event; do NOT reschedule.
        if (!dynamic_cast<veins::TraCIMobility*>(mobility)) {
            double interval = hasPar("denmInterval") ? par("denmInterval").doubleValue() : 1.0;
            double nextTime = simTime().dbl() + interval;
            if (nextTime <= stopTime)
                scheduleAt(nextTime, sendDenmEvt);
            else
                EV_INFO << "DENM [TX]: next send would exceed stopTime — stopping\n";
        }
        break;
    }

    default:
        EV_WARN << "handleSelfMsg: unknown message kind=" << msg->getKind()
                << " name=" << getParentModule()->getFullName() << endl;
        break;
    }
}

// ------------------------------------------------------------
//  TX — Message population
//
//  Fills all fields of the outgoing BaseFrame1609_4 subclass.
//
//  CamMessage branch:
//    buildCam(*applicationToVanetzaConverter) → vanetza::asn1::Cam
//    cam.encode() → ByteBuffer → copied into vanetzaPayload[]
//
//  DenmMessage branch:
//    buildDenm(*applicationToVanetzaConverter, seqNum, cause, subcause)
//    → vanetza::asn1::Denm
//    denm.encode() → ByteBuffer → copied into vanetzaPayload[]
//
//  BSM / WSA / generic WSM: standard Veins field population.
// ------------------------------------------------------------
void DemoBaseApplLayer::populateWSM(BaseFrame1609_4* wsm, LAddress::L2Type rcvId, int serial)
{
    wsm->setRecipientAddress(rcvId);
    wsm->setBitLength(headerLength);

    if (DemoSafetyMessage* bsm = dynamic_cast<DemoSafetyMessage*>(wsm)) {
        // BSM: embed current position and speed for neighbour awareness
        bsm->setSenderPos(curPosition);
        bsm->setSenderSpeed(curSpeed);
        bsm->setPsid(-1);
        bsm->setChannelNumber(static_cast<int>(Channel::cch));
        bsm->addBitLength(beaconLengthBits);
        wsm->setUserPriority(beaconUserPriority);
    }
    else if (DemoServiceAdvertisment* wsa = dynamic_cast<DemoServiceAdvertisment*>(wsm)) {
        // WSA: advertise the currently offered service on CCH
        wsa->setChannelNumber(static_cast<int>(Channel::cch));
        wsa->setTargetChannel(static_cast<int>(currentServiceChannel));
        wsa->setPsid(currentOfferedServiceId);
        wsa->setServiceDescription(currentServiceDescription.c_str());
    }
    else if (CamMessage* cam = dynamic_cast<CamMessage*>(wsm)) {
        // CAM TX path:
        //   1. buildCam() fills the ASN.1 structure from ApplicationToVanetzaConverter
        //   2. vanetza encodes it into a ByteBuffer
        //   3. ByteBuffer is copied byte-by-byte into vanetzaPayload[]
        if (applicationToVanetzaConverter) {
            mAdapter->populateCAM(cam, applicationToVanetzaConverter, headerLength, beaconUserPriority);
            cam->setChannelNumber(static_cast<int>(Channel::cch));
            cam->setPsid(36); // ITS CAM PSID
        } else {
            EV_WARN << "CAM [BUILD]: no ApplicationToVanetzaConverter — payload empty\n";
            cam->setVanetzaPayloadArraySize(0);
            cam->setByteLength(0);
        }
    }
    else if (DenmMessage* denm = dynamic_cast<DenmMessage*>(wsm)) {
        // DENM TX path:
        //   1. denmSequenceNumber is incremented for each new DENM
        //   2. buildDenm() fills the ASN.1 structure (cause, subcause, position, timestamps)
        //   3. vanetza encodes it into a ByteBuffer
        //   4. ByteBuffer is copied byte-by-byte into vanetzaPayload[]
        if (applicationToVanetzaConverter) {
            try {
                denmSequenceNumber++;
                mAdapter->populateDENM(denm,
                                       applicationToVanetzaConverter,
                                       denmSequenceNumber,
                                       par("denmDefaultCause"),
                                       par("denmDefaultSubcause"),
                                       headerLength,
                                       denmUserPriority,
                                       denmLengthBits);

                denm->setChannelNumber(static_cast<int>(Channel::cch));
                denm->setPsid(-1);
            } catch (...) {
                EV_WARN << "DENM [BUILD]: vanetza encoding failed — payload empty\n";
                denm->setVanetzaPayloadArraySize(0);
                denm->setByteLength(0);
            }
        } else {
            denm->setVanetzaPayloadArraySize(0);
            denm->setByteLength(0);
        }
        denm->setChannelNumber(static_cast<int>(Channel::cch));
        denm->setPsid(-1);
        if (denm->getByteLength() == 0) denm->addBitLength(denmLengthBits);
        denm->setUserPriority(denmUserPriority);
    }
    else {
        // Generic WSM: route to SCH or CCH based on .ini configuration
        if (dataOnSch) wsm->setChannelNumber(static_cast<int>(Channel::sch1));
        else           wsm->setChannelNumber(static_cast<int>(Channel::cch));
        wsm->addBitLength(dataLengthBits);
        wsm->setUserPriority(dataUserPriority);
    }
}

// ------------------------------------------------------------
//  TX — DENM event-driven flow (vehicle nodes)
//
//  Signal dispatcher → handlePositionUpdate() evaluates trigger
//  conditions → triggerDenm() validates and rate-limits →
//  sendDenmNow() cancels any pending event and fires immediately.
// ------------------------------------------------------------

// Dispatches OMNeT++ signals to the appropriate handler:
//   mobilityStateChangedSignal → handlePositionUpdate()
//   parkingStateChangedSignal  → triggerDenm() + handleParkingUpdate()
//   collisionSignal            → triggerDenm()
//   automatically called by the framework OMNeT++
void DemoBaseApplLayer::receiveSignal(cComponent* source, simsignal_t signalID,
                                      cObject* obj, cObject* details)
{
    Enter_Method_Silent();

    if (signalID == BaseMobility::mobilityStateChangedSignal) {
        // check if there is any event in order to decide if we have to send a DENM or not
        handlePositionUpdate(obj);
    }
    else if (signalID == TraCIMobility::parkingStateChangedSignal) {
        // we are sure that a DENM need to be sent and we update also the internal state of the module
        int cause    = par("denmDefaultCause");
        int subcause = par("denmDefaultSubcause");
        triggerDenm(static_cast<CauseCodeType_t>(cause), cause, subcause, nullptr, true);
        handleParkingUpdate(obj);
    }
    else if (signalID == TraCIMobility::collisionSignal) {
        // we are sure that a DENM need to be sent
        int cause    = par("denmDefaultCause");
        int subcause = par("denmDefaultSubcause");
        triggerDenm(static_cast<CauseCodeType_t>(cause), cause, subcause, nullptr, true);
    }
}

// Called on every mobilityStateChangedSignal.
// Updates curPosition / curSpeed; lazily initialises applicationToVanetzaConverter.
// For vehicle nodes: evaluates per-cause trigger conditions and calls
// triggerDenm() when the condition is met and the rate-limit window has elapsed.
// Respects the global [denmStartTime, denmStopTime] window from .ini.
void DemoBaseApplLayer::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mob = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mob->getPositionAt(simTime());
    curSpeed    = mob->getCurrentSpeed();

    int cause = par("denmDefaultCause");
    if (cause < 0) return; // no DENM cause configured — nothing to do

    // Respect the global DENM time window
    double denmStart = hasPar("denmStartTime") ? par("denmStartTime").doubleValue() : 0.0;
    double denmStop  = hasPar("denmStopTime")  ? par("denmStopTime").doubleValue()  : 1e9;
    if (simTime().dbl() < denmStart || simTime().dbl() > denmStop) return;

    double speed_ms  = curSpeed.length();
    double speed_kmh = speed_ms * 3.6;
    bool   triggerCondition = false;
    double defaultInterval  = 1.0;

    // Helper: reads accidentStart and accidentDuration from the mobility module .ini params
    auto getAccidentWindow = [&](simtime_t& start, simtime_t& end) -> bool {
        cModule* mobMod = dynamic_cast<cModule*>(this->mobility);
        if (!mobMod || !mobMod->hasPar("accidentStart")) return false;
        start = mobMod->par("accidentStart").doubleValue();
        end   = start + simtime_t(mobMod->par("accidentDuration").doubleValue());
        return true;
    };

    // Helper: true if vehicle is stopped (< 0.1 m/s) within the accident time window
    auto stoppedInWindow = [&]() -> bool {
        simtime_t start, end;
        return getAccidentWindow(start, end) && speed_ms < 0.1
               && simTime() >= start && simTime() <= end;
    };

    // Helper: true if vehicle is below threshold_kmh within the accident time window
    auto slowInWindow = [&](double threshold_kmh) -> bool {
        simtime_t start, end;
        return getAccidentWindow(start, end) && speed_kmh < threshold_kmh
               && simTime() >= start && simTime() <= end;
    };


    //Debug log
    /*EV_INFO << "DENM [DEBUG]: cause=" << cause
            << " speed_kmh=" << speed_kmh
            << " t=" << simTime()
            << " denmStart=" << denmStart
            << " denmStop=" << denmStop << endl;*/

    // Evaluate trigger condition and default re-send interval per cause code
    switch (cause) {

        // --- Traffic / speed conditions ---
        case CauseCodeType_trafficCondition:
            triggerCondition = true;
            defaultInterval = 2.0;
            break;
        case CauseCodeType_slowVehicle:
            if (slowInWindow(15.0) && speed_kmh > 0.5) triggerCondition = true;
            defaultInterval = 2.0;
            break;
        case CauseCodeType_dangerousEndOfQueue:
            if (speed_kmh < 10.0 && speed_kmh > 0.1) triggerCondition = true;
            defaultInterval = 1.0;
            break;

        // --- Stationary vehicle conditions (require stopped-in-window) ---
        case CauseCodeType_accident:
        case CauseCodeType_impassability:
        case CauseCodeType_vehicleBreakdown:
        case CauseCodeType_postCrash:
        case CauseCodeType_humanProblem:
        case CauseCodeType_stationaryVehicle:
            if (stoppedInWindow()) triggerCondition = true;
            defaultInterval = 1.0;
            break;
        case CauseCodeType_rescueAndRecoveryWorkInProgress:
            if (stoppedInWindow()) triggerCondition = true;
            defaultInterval = 2.0;
            break;

        // --- Environmental / weather conditions (always active within window) ---
        case CauseCodeType_adverseWeatherCondition_Adhesion:
        case CauseCodeType_adverseWeatherCondition_ExtremeWeatherCondition:
            triggerCondition = true;
            defaultInterval = 5.0;
            break;
        case CauseCodeType_aquaplannning:
        case CauseCodeType_adverseWeatherCondition_Visibility:
        case CauseCodeType_adverseWeatherCondition_Precipitation:
        case CauseCodeType_hazardousLocation_SurfaceCondition:
        case CauseCodeType_hazardousLocation_DangerousCurve:
            triggerCondition = true;
            defaultInterval = 3.0;
            break;

        // --- Localised hazards and dangerous behaviours ---
        case CauseCodeType_hazardousLocation_ObstacleOnTheRoad:
        case CauseCodeType_hazardousLocation_AnimalOnTheRoad:
        case CauseCodeType_humanPresenceOnTheRoad:
            triggerCondition = true;
            defaultInterval = 2.0;
            break;
        case CauseCodeType_wrongWayDriving:
        case CauseCodeType_emergencyVehicleApproaching:
        case CauseCodeType_collisionRisk:
        case CauseCodeType_signalViolation:
        case CauseCodeType_dangerousSituation:
            triggerCondition = true;
            defaultInterval = 1.0;
            break;

        default:
            break;
    }

    if (triggerCondition) {
        // Use explicit .ini denmInterval if provided, otherwise fall back to per-cause default
        double finalInterval = hasPar("denmInterval") ? par("denmInterval").doubleValue() : defaultInterval;
        if (simTime() - lastDenmTime >= finalInterval) {
            EV_INFO << "DENM [TRIGGER]: cause=" << cause
                    << " speed=" << speed_kmh
                    << " node=" << getParentModule()->getFullName()
                    << " t=" << simTime() << endl;
            triggerDenm(static_cast<CauseCodeType_t>(cause), cause,
                        par("denmDefaultSubcause"), nullptr, true);
        }
    } else {
        // Condition no longer met: remove from active event set
        if (activeDenmEvents.count(cause)) {
            EV_INFO << "DENM [EVENT ENDED]: cause=" << cause << endl;
            activeDenmEvents.erase(cause);
        }
    }
}

// Updates the isParked flag from the TraCIMobility parking state.
// Called from receiveSignal() on parkingStateChangedSignal.
void DemoBaseApplLayer::handleParkingUpdate(cObject* obj)
{
    if (auto traciMob = dynamic_cast<veins::TraCIMobility*>(mobility))
        isParked = traciMob->getParkingState();
    else
        isParked = false;
}

// Validates the event cause code, applies the denmMinInterval rate limit,
// registers the cause in activeDenmEvents, then delegates to sendDenmNow().
// Called from handlePositionUpdate() (speed/position trigger) or from
// receiveSignal() (parking / collision signals).
void DemoBaseApplLayer::triggerDenm(CauseCodeType_t eventCause, int cause, int subcause,
                                     const Coord* eventPos, bool sendImmediate)
{
    int actualCause    = (cause >= 0)    ? cause    : static_cast<int>(eventCause);
    int actualSubcause = (subcause >= 0) ? subcause : 0;

    // Validate cause code range (ITS-G5: 0–255)
    if (actualCause < 0 || actualCause > 255) {
        EV_ERROR << "DENM [TRIGGER]: invalid causeCode=" << actualCause << endl;
        return;
    }

    // Rate limiting: enforce minimum inter-DENM interval
    if (simTime() - lastDenmTime < denmMinInterval) {
        EV_WARN << "DENM [TRIGGER]: rate-limited — cause=" << actualCause
                << " node=" << getParentModule()->getFullName() << endl;
        return;
    }

    Coord pos = eventPos ? *eventPos : curPosition;
    lastDenmTime = simTime();
    activeDenmEvents.insert(actualCause);
    sendDenmNow(eventCause, cause, subcause, pos);
}

// Cancels any pending sendDenmEvt and reschedules it at simTime() (immediate).
// handleSelfMsg(SEND_DENM_EVT) will then call populateWSM() → buildDenm() → sendDown().
// For vehicle nodes this is a one-shot fire; RSU periodic reschedule is
// handled inside handleSelfMsg(SEND_DENM_EVT).
void DemoBaseApplLayer::sendDenmNow(CauseCodeType_t eventCause, int cause, int subcause,
                                     const Coord& eventPos)
{
    if (!applicationToVanetzaConverter) {
        EV_WARN << "DENM [SEND NOW]: no ApplicationToVanetzaConverter — aborted\n";
        return;
    }

    // Cancel any already-pending DENM event to avoid double-send
    if (sendDenmEvt->isScheduled())
        cancelEvent(sendDenmEvt);

    // Fire immediately; actual message construction happens in handleSelfMsg()
    scheduleAt(simTime(), sendDenmEvt);

    int causeCode = (cause >= 0) ? cause : static_cast<int>(eventCause);
    EV_INFO << "DENM [SEND NOW]: cause=" << causeCode
            << " node=" << getParentModule()->getFullName()
            << " t=" << simTime() << endl;
}

// ------------------------------------------------------------
//  TX — Send to MAC layer
// ------------------------------------------------------------

// Increments the appropriate generated* counter, then forwards
// the message down to BaseApplLayer::sendDown() → MAC layer.
void DemoBaseApplLayer::sendDown(cMessage* msg)
{
    checkAndTrackPacket(msg);
    BaseApplLayer::sendDown(msg);
}

// Delayed variant: increments counter then calls sendDelayedDown().
void DemoBaseApplLayer::sendDelayedDown(cMessage* msg, simtime_t delay)
{
    checkAndTrackPacket(msg);
    BaseApplLayer::sendDelayedDown(msg, delay);
}

// Inspects the dynamic type of the outgoing message and increments
// the matching generatedXxx counter (BSM, WSA, CAM, DENM, or generic WSM).
void DemoBaseApplLayer::checkAndTrackPacket(cMessage* msg)
{
    if      (dynamic_cast<DemoSafetyMessage*>(msg))       { generatedBSMs++;  }
    else if (dynamic_cast<DemoServiceAdvertisment*>(msg)) { generatedWSAs++;  }
    else if (dynamic_cast<CamMessage*>(msg))              { generatedCAMs++;  }
    else if (dynamic_cast<DenmMessage*>(msg))             { generatedDENMs++; }
    else if (dynamic_cast<BaseFrame1609_4*>(msg))         { generatedWSMs++;  }
}

// ------------------------------------------------------------
//  TX — WSA service management
// ------------------------------------------------------------

// Starts advertising a service on the given channel.
// Schedules the first WSA using a desynchronised sending time to
// avoid simultaneous WSA bursts from multiple nodes.
void DemoBaseApplLayer::startService(Channel channel, int serviceId,
                                      std::string serviceDescription)
{
    if (sendWSAEvt->isScheduled())
        throw cRuntimeError("Starting service although another service was already started");
    mac->changeServiceChannel(channel);
    currentOfferedServiceId   = serviceId;
    currentServiceChannel     = channel;
    currentServiceDescription = serviceDescription;
    scheduleAt(computeAsynchronousSendingTime(wsaInterval, ChannelType::control), sendWSAEvt);
}

// Stops the currently advertised service and cancels the WSA timer.
void DemoBaseApplLayer::stopService()
{
    cancelEvent(sendWSAEvt);
    currentOfferedServiceId = -1;
}

// ============================================================
//  RX PATH
// ============================================================

// ------------------------------------------------------------
//  RX — Lower message dispatcher
//
//  Entry point for all messages arriving from the MAC layer.
//  Casts the message to the correct subtype and dispatches to
//  the appropriate on*() callback; increments the matching
//  receivedXxx counter; deletes the message when done.
// ------------------------------------------------------------
void DemoBaseApplLayer::handleLowerMsg(cMessage* msg)
{
    BaseFrame1609_4* wsm = dynamic_cast<BaseFrame1609_4*>(msg);
    ASSERT(wsm);

    if (DemoSafetyMessage* bsm = dynamic_cast<DemoSafetyMessage*>(wsm)) {
        receivedBSMs++;  onBSM(bsm);
    } else if (DemoServiceAdvertisment* wsa = dynamic_cast<DemoServiceAdvertisment*>(wsm)) {
        receivedWSAs++;  onWSA(wsa);
    } else if (CamMessage* cam = dynamic_cast<CamMessage*>(wsm)) {
        receivedCAMs++;
        mAdapter->onCAM(cam, getParentModule()->getFullName());
    }
    else if (DenmMessage* denm = dynamic_cast<DenmMessage*>(wsm)) {
        receivedDENMs++;
        mAdapter->onDENM(denm, getParentModule()->getFullName());
    } else {
        receivedWSMs++;  onWSM(wsm);
    }
    delete msg;
}

// ============================================================
//  Utility
// ============================================================

// Computes a desynchronised first-send time to avoid simultaneous
// beacon / WSA bursts from multiple nodes at simulation start.
// Aligns the offset to the correct CCH/SCH slot boundary when
// channel switching is active.
simtime_t DemoBaseApplLayer::computeAsynchronousSendingTime(simtime_t interval, ChannelType chan)
{
    simtime_t randomOffset      = dblrand() * interval;
    simtime_t switchingInterval = mac->getSwitchingInterval();
    simtime_t nextCCH;

    if (mac->isCurrentChannelCCH())
        nextCCH = simTime() - SimTime().setRaw(simTime().raw() % switchingInterval.raw())
                  + switchingInterval * 2;
    else
        nextCCH = simTime() - SimTime().setRaw(simTime().raw() % switchingInterval.raw())
                  + switchingInterval;

    simtime_t firstEvent = nextCCH + randomOffset;
    if (firstEvent.raw() % (2 * switchingInterval.raw()) > switchingInterval.raw()) {
        if (chan == ChannelType::control) firstEvent -= switchingInterval;
    } else {
        if (chan == ChannelType::service) firstEvent += switchingInterval;
    }
    return firstEvent;
}
