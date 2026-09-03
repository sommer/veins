#include <omnetpp.h>
#include <veins/modules/application/ieee80211p/VanetzaToVeinsBridge.h>

using namespace omnetpp;

Define_Module(veins::VanetzaToVeinsBridge);

namespace veins {

VanetzaToVeinsBridge::VanetzaToVeinsBridge()
    : vanetzaEntityImplementation(std::make_unique<VanetzaEntityImplementation>())
{
}

// ============================================================
// Vanetza entity access
// Returns a reference to the internal VanetzaEntity, which manages
// CAM generation scheduling and acts as the entry point between
// the Veins simulation environment and the Vanetza protocol stack.
// ============================================================

vanetza::dcc::Entity& VanetzaToVeinsBridge::getVanetzaEntity()
{
    return *vanetzaEntityImplementation;
}

// ============================================================
// CAM generation check — ETSI EN 302 637-2 §6.1.3
//
// Computes the delta values (position, heading, speed, elapsed time)
// between the current vehicle state and the state at the last CAM sent,
// then delegates the generation decision to VanetzaEntityImplementation (TRC rules).
//
// Returns true  → a new CAM should be generated and transmitted.
// Returns false → no CAM needed at this T_CheckCamGen tick.
//
// On the very first call (mLastCamValid == false), always returns true
// to ensure the first CAM is sent unconditionally.
// ============================================================

bool VanetzaToVeinsBridge::computeAndCheckCamDeltas(const ApplicationToVanetzaConverter& vdp)
{
    // No reference state yet: force the first CAM transmission
    if (!mLastCamValid) return true;

    // Build current ASN.1 position (scaled to 1e-7 degrees as per ETSI)
    ReferencePosition_t posNow{};
    posNow.latitude  = static_cast<Latitude_t>(std::lround(vdp.latitude().value()  * 1e7));
    posNow.longitude = static_cast<Longitude_t>(std::lround(vdp.longitude().value() * 1e7));
    posNow.altitude.altitudeValue      = AltitudeValue_unavailable;
    posNow.altitude.altitudeConfidence = AltitudeConfidence_unavailable;

    // Compute Euclidean distance from last CAM position (metres)
    vanetza::units::Length deltaPos =
        vanetza::facilities::distance(posNow, mLastCamPos);
    double deltaPos_m = deltaPos / vanetza::units::si::meter;

    // Compute shortest angular difference between current and last heading (degrees)
    // mLastCamHeading is stored in tenths of degrees (ETSI HeadingValue_t), so divide by 10
    double headingNow_deg  = std::fmod(vdp.heading().value() * (180.0 / M_PI) + 360.0, 360.0);
    double headingLast_deg = std::fmod(mLastCamHeading / 10.0 + 360.0, 360.0);
    double deltaHeading_deg = std::abs(headingNow_deg - headingLast_deg);
    if (deltaHeading_deg > 180.0) deltaHeading_deg = 360.0 - deltaHeading_deg;

    // Compute absolute speed difference (m/s)
    double deltaSpeed_ms = std::abs(vdp.speed().value() - mLastCamSpeed_ms);

    // Compute elapsed time since last CAM (used by TRC to enforce T_GenCam limits)
    auto now = vanetza::Clock::time_point(
        std::chrono::milliseconds(
            static_cast<long long>(simTime().dbl() * 1000.0)));
    auto elapsed = now - mLastCamTime;

    // Delegate the generation decision to VanetzaEntityImplementation (ETSI TRC rules)
    return vanetzaEntityImplementation->forwardCamGenerationCheck(
        deltaHeading_deg,
        deltaPos_m,
        deltaSpeed_ms,
        elapsed);
}

// Called by the application layer immediately after a CAM has been sent.
// Saves the current vehicle state as the new reference point for the next
// delta computation in computeAndCheckCamDeltas().
// Without this call, the delta thresholds would always be evaluated against
// the very first CAM ever sent, causing incorrect generation decisions.
void VanetzaToVeinsBridge::notifyCamSent(const ApplicationToVanetzaConverter& vdp)
{
    // Compute elapsed time since the last CAM and update the reference timestamp
    simtime_t interval = simTime() - mLastCamTime_sim;
    mLastCamTime_sim = simTime();

    // Convert heading from radians to degrees (0–360 range)
    double headingNow_deg = std::fmod(vdp.heading().value() * (180.0 / M_PI) + 360.0, 360.0);

    // Log vehicle state at CAM generation time for ETSI EN 302 637-2 compliance validation.
    // "PERIODIC" if the CAM was triggered by the 1s timeout, "DYNAMIC" if triggered by a delta threshold.
    EV_INFO << "[ETSI_VALIDATION]"
            << " t=" << simTime()
            << " | module=" << getParentModule()->getFullPath()
            << " | interval=" << interval << "s"
            << " | speed=" << vdp.speed().value() << " m/s"
            << " | heading=" << headingNow_deg << " deg"
            << " | lat=" << vdp.latitude().value()
            << " | lon=" << vdp.longitude().value()
            << " | reason=" << (interval.dbl() >= 0.99 ? "PERIODIC" : "DYNAMIC")
            << endl;

    // Record vectors for graphical ETSI validation
    vCamInterval.record(interval.dbl());
    vSpeed.record(vdp.speed().value());
    vHeading.record(headingNow_deg);

    // Save current position as ASN.1 ReferencePosition (scaled to 1e-7 degrees)
    mLastCamPos.latitude  = static_cast<Latitude_t>(std::lround(vdp.latitude().value()  * 1e7));
    mLastCamPos.longitude = static_cast<Longitude_t>(std::lround(vdp.longitude().value() * 1e7));
    mLastCamPos.altitude.altitudeValue      = AltitudeValue_unavailable;
    mLastCamPos.altitude.altitudeConfidence = AltitudeConfidence_unavailable;

    // Save heading in tenths of degrees (ETSI HeadingValue_t format)
    double h_tenths = std::fmod(
        std::fmod(vdp.heading().value() * (180.0 / M_PI), 360.0) + 360.0, 360.0) * 10.0;
    mLastCamHeading = static_cast<HeadingValue_t>(std::lround(h_tenths));

    // Save speed (m/s) and timestamp for next delta computation
    mLastCamSpeed_ms = vdp.speed().value();
    mLastCamTime = vanetza::Clock::time_point(
        std::chrono::milliseconds(
            static_cast<long long>(simTime().dbl() * 1000.0)));

    // Mark reference state as valid so future calls to computeAndCheckCamDeltas()
    // will perform the full delta check instead of forcing transmission
    mLastCamValid = true;
}

// ============================================================
// Helper: DENM cause code → human-readable string
//
// Maps ETSI EN 302 637-3 CauseCode integer values to their
// corresponding string identifiers for logging and debug purposes.
// Source: ETSI EN 302 637-3 Annex A, Table A.1.
// ============================================================

std::string VanetzaToVeinsBridge::denmCauseToString(int cause)
{
    switch (cause) {
        case 1:  return "trafficCondition";
        case 2:  return "accident";
        case 3:  return "roadworks";
        case 5:  return "impassability";
        case 6:  return "adverseWeatherCondition_Adhesion";
        case 7:  return "aquaplanning";
        case 9:  return "hazardousLocation_SurfaceCondition";
        case 10: return "hazardousLocation_ObstacleOnTheRoad";
        case 11: return "hazardousLocation_AnimalOnTheRoad";
        case 12: return "humanPresenceOnTheRoad";
        case 14: return "wrongWayDriving";
        case 15: return "rescueAndRecoveryWorkInProgress";
        case 17: return "adverseWeatherCondition_ExtremeWeather";
        case 18: return "adverseWeatherCondition_Visibility";
        case 19: return "adverseWeatherCondition_Precipitation";
        case 26: return "slowVehicle";
        case 27: return "dangerousEndOfQueue";
        case 91: return "vehicleBreakdown";
        case 92: return "postCrash";
        case 93: return "humanProblem";
        case 94: return "stationaryVehicle";
        case 95: return "emergencyVehicleApproaching";
        case 96: return "hazardousLocation_DangerousCurve";
        case 97: return "collisionRisk";
        case 98: return "signalViolation";
        case 99: return "dangerousSituation";
        default: return "unknown";
    }
}

// ============================================================
// POPULATE CAM (TX serialization pipeline)
//
// Serialization flow:
//   1. buildCam()  — ApplicationToVanetzaConverter populates the ASN.1 CAM
//                    structure using the current vehicle state
//   2. encode()    — Vanetza encodes the ASN.1 structure into a raw byte buffer
//   3. setVanetzaPayload() — the byte buffer is copied into the OMNeT++ message
//                            field-by-field so it can be transmitted by Veins
// ============================================================

void VanetzaToVeinsBridge::populateCAM(CamMessage* cam,
                                 ApplicationToVanetzaConverter* vdp,
                                 int headerLength,
                                 int priority)
{
    if (!vdp) return;

    // Step 1: build and encode the CAM via VanetzaEntityImplementation
    auto vanetzaCam = vdp->buildCam();
    vanetza::ByteBuffer buffer = vanetzaCam.encode();

    // Step 2: copy the encoded bytes into the OMNeT++ CamMessage payload
    cam->setVanetzaPayloadArraySize(buffer.size());
    for (size_t i = 0; i < buffer.size(); ++i)
        cam->setVanetzaPayload(i, buffer[i]);

    // Step 3: set message size and radio priority
    cam->setByteLength(buffer.size());
    cam->setBitLength(headerLength + buffer.size() * 8);
    cam->setUserPriority(priority);
}

// ============================================================
// POPULATE DENM (TX serialization pipeline)
//
// Serialization flow:
//   1. buildDenm() — ApplicationToVanetzaConverter populates the ASN.1 DENM
//                    structure using the current vehicle state and event data
//   2. encode()    — Vanetza encodes the ASN.1 structure into a raw byte buffer
//   3. setVanetzaPayload() — the byte buffer is copied into the OMNeT++ message
//                            field-by-field so it can be transmitted by Veins
// ============================================================

void VanetzaToVeinsBridge::populateDENM(DenmMessage* denm,
                                  ApplicationToVanetzaConverter* vdp,
                                  uint16_t& seq,
                                  int cause,
                                  int subcause,
                                  int headerLength,
                                  int priority,
                                  int fallbackBits)
{
    if (!vdp) return;

    auto vanetzaDenm = vdp->buildDenm(seq, cause, subcause);
    vanetza::ByteBuffer buffer = vanetzaDenm.encode();

    denm->setVanetzaPayloadArraySize(buffer.size());
    for (size_t i = 0; i < buffer.size(); ++i)
        denm->setVanetzaPayload(i, buffer[i]);

    denm->setByteLength(buffer.size());
    if (buffer.size() == 0)
        denm->addBitLength(fallbackBits);

    denm->setBitLength(headerLength + denm->getByteLength() * 8);
    denm->setUserPriority(priority);
}

// ============================================================
// RX CAM
// ============================================================

void VanetzaToVeinsBridge::onCAM(CamMessage* camMsg, const std::string& nodeName)
{
    size_t n = camMsg->getVanetzaPayloadArraySize();
    vanetza::ByteBuffer buffer(n);

    for (size_t i = 0; i < n; ++i)
        buffer[i] = camMsg->getVanetzaPayload(i);

    try {
        vanetza::asn1::Cam cam;
        cam.decode(buffer);

        EV_INFO << "CAM [RX]: stationID=" << cam->header.stationID
                << " name=" << nodeName
                << " t=" << simTime() << "\n";

    } catch (...) {
        EV_ERROR << "CAM decode error\n";
    }
}

// ============================================================
// RX DENM
// ============================================================

void VanetzaToVeinsBridge::onDENM(DenmMessage* denmMsg, const std::string& nodeName)
{
    size_t n = denmMsg->getVanetzaPayloadArraySize();
    if (n == 0) return;

    vanetza::ByteBuffer buffer(n);

    for (size_t i = 0; i < n; ++i)
        buffer[i] = denmMsg->getVanetzaPayload(i);

    try {
        vanetza::asn1::Denm denm;
        denm.decode(buffer);

        int cause = denm->denm.situation
            ? denm->denm.situation->eventType.causeCode
            : -1;

        EV_INFO << "DENM [RX]: cause=" << cause << " (" << denmCauseToString(cause) << ")"
                << " receiver=" << nodeName
                << " t=" << simTime() << "\n";

    } catch (...) {
        EV_ERROR << "DENM decode error\n";
    }
}

// ============================================================
// OMNeT++ module initialization (stage 0)
//
// Called once when the simulation starts. Registers output signals
// for scalar/statistics collection and names the output vectors
// used for ETSI compliance validation charts.
// ============================================================

void VanetzaToVeinsBridge::initialize()
{
    EV_INFO << "VanetzaToVeinsBridge initialized as OMNeT++ module at "
            << getFullPath() << "\n";

    // Scalar signals for OMNeT++ .sca statistics (total CAM/DENM sent/received)
    camSentSignal = registerSignal("camSent");
    camReceivedSignal = registerSignal("camReceived");
    denmSentSignal = registerSignal("denmSent");
    denmReceivedSignal = registerSignal("denmReceived");

    // Output vectors for time-series validation of ETSI EN 302 637-2 rules
    // These vectors are recorded per-CAM to correlate vehicle dynamics
    // with generation intervals in the Analysis Editor charts.
    vCamInterval.setName("camInterval");
    vSpeed.setName("speedAtCam");
    vHeading.setName("headingAtCam");
}

// ============================================================
// OMNeT++ message handler — protocol layer forwarding
//
// Implements the forwarding plane of the VanetzaAdapter:
//   - Data messages (CAM/DENM) are relayed between the upper
//     application layer and the lower NIC (802.11p MAC/PHY).
//   - Control messages are passed through transparently.
//   - Scalar signals are emitted for each TX/RX event so that
//     the Analysis Editor can count total messages per node.
//
// The gate names match the NED declaration:
//   upperLayerIn / upperLayerOut   → Application (e.g., DemoBaseApplLayer)
//   lowerLayerIn / lowerLayerOut   → NIC (e.g., Nic80211p)
//   upperControlIn / upperControlOut
//   lowerControlIn / lowerControlOut
// ============================================================

void VanetzaToVeinsBridge::handleMessage(cMessage* msg)
{
    EV_INFO << "VanetzaToVeinsBridge received type=" << msg->getClassName()
            << " name='" << msg->getName() << "' on gate "
            << msg->getArrivalGate()->getFullName()
            << " at " << getFullPath() << "\n";

    // --------------------------------------------------------
    // UPLINK: Application → NIC (TX path)
    // Distinguish DENM from CAM for separate signal counting,
    // then forward to the lower layer for wireless transmission.
    // --------------------------------------------------------
    if (msg->arrivedOn("upperLayerIn")) {
        if (dynamic_cast<DenmMessage*>(msg)) {
            emit(denmSentSignal, 1.0);
        } else {
            emit(camSentSignal, 1.0);
        }
        send(msg, "lowerLayerOut");
    }
    // --------------------------------------------------------
    // DOWNLINK: NIC → Application (RX path)
    // Count received messages and hand them up to the
    // application layer (e.g., onCAM/onDENM callbacks).
    // --------------------------------------------------------
    else if (msg->arrivedOn("lowerLayerIn")) {
        if (dynamic_cast<DenmMessage*>(msg)) {
            emit(denmReceivedSignal, 1.0);
        } else {
            emit(camReceivedSignal, 1.0);
        }
        send(msg, "upperLayerOut");
    }
    // --------------------------------------------------------
    // CONTROL PATH: pass-through between upper and lower layers
    // --------------------------------------------------------
    else if (msg->arrivedOn("upperControlIn")) {
        send(msg, "lowerControlOut");
    }
    else if (msg->arrivedOn("lowerControlIn")) {
        send(msg, "upperControlOut");
    }
    // --------------------------------------------------------
    // SAFETY: unknown gate → discard to prevent memory leaks
    // --------------------------------------------------------
    else {
        EV_WARN << "VanetzaToVeinsBridge received message on unexpected gate "
                << msg->getArrivalGate()->getFullName()
                << "; deleting message\n";
        delete msg;
    }
}

} // namespace veins
