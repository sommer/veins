// ============================================================
//  ApplicationToVanetzaConverter — implementation
//
//  Bridges Veins BaseMobility / TraCIMobility to the
//  Vanetza-style data interface consumed by buildCam() and
//  buildDenm().  All geographic conversions assume a flat-
//  earth approximation centred on (45°N, 9°E) — sufficient
//  for small-scale SUMO scenarios.
// ============================================================

#include <veins/modules/application/ieee80211p/ApplicationToVanetzaConverter.h>

using namespace omnetpp;

Define_Module(veins::ApplicationToVanetzaConverter);

namespace veins {

// Default constructor required by Define_Module / OMNeT++ module system
ApplicationToVanetzaConverter::ApplicationToVanetzaConverter()
    : mMobility(nullptr)
{
}

ApplicationToVanetzaConverter::ApplicationToVanetzaConverter(BaseMobility* mob)
: mMobility(mob)
{
    const char *hostName = nullptr;
    if (mMobility && mMobility->getParentModule())
        hostName = mMobility->getParentModule()->getFullName();
    EV_INFO << "ApplicationToVanetzaConverter constructed; mobility=" << (void*)mMobility
            << " host=" << (hostName ? hostName : "null") << " t=" << simTime() << "\n";
}

void ApplicationToVanetzaConverter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == 0) {
        mMobility = FindModule<BaseMobility*>::findSubModule(getParentModule());
        if (!mMobility) {
            EV_WARN << "ApplicationToVanetzaConverter: mobility not found in host "
                    << getParentModule()->getFullName() << "\n";
        } else {
            EV_INFO << "ApplicationToVanetzaConverter initialized as OMNeT++ module at "
                    << getFullPath() << "\n";
        }
    }
}

void ApplicationToVanetzaConverter::handleMessage(omnetpp::cMessage* msg)
{
    // ApplicationToVanetzaConverter does not receive messages directly.
    // All data is read on demand via getter methods.
    EV_WARN << "ApplicationToVanetzaConverter: unexpected message received: "
            << msg->getName() << "\n";
    delete msg;
}

// Converts SUMO Y-coordinate (metres north of origin) to decimal degrees latitude.
vanetza::units::GeoAngle ApplicationToVanetzaConverter::latitude() const {
    if (!mMobility) return 0.0 * boost::units::degree::degree;
    double lat_m = mMobility->getPositionAt(simTime()).y;
    double deg = 45.0 + (lat_m / 111000.0);
    return deg * boost::units::degree::degree;
}

// Converts SUMO X-coordinate (metres east of origin) to decimal degrees longitude.
vanetza::units::GeoAngle ApplicationToVanetzaConverter::longitude() const {
    if (!mMobility) return 0.0 * boost::units::degree::degree;
    double lon_m = mMobility->getPositionAt(simTime()).x;
    double deg = 9.0 + (lon_m / 111000.0);
    return deg * boost::units::degree::degree;
}

// Returns speed in m/s.  Prefers TraCIMobility::getSpeed() for accuracy;
// falls back to BaseMobility::getCurrentSpeed().length().
vanetza::units::Velocity ApplicationToVanetzaConverter::speed() const {
    using namespace boost::units;
    using namespace boost::units::si;
    if (!mMobility) return 0.0 * meter_per_second;
    if (auto traciMob = dynamic_cast<veins::TraCIMobility*>(mMobility))
        return traciMob->getSpeed() * meter_per_second;
    return mMobility->getCurrentSpeed().length() * meter_per_second;
}

// Returns heading in radians [0, 2π), normalised from TraCIMobility or
// computed via atan2 from the orientation vector for generic mobility.
vanetza::units::Angle ApplicationToVanetzaConverter::heading() const {
    using namespace boost::units;
    using namespace boost::units::si;
    if (!mMobility) return 0.0 * radian;
    double rad = 0.0;
    if (auto traciMob = dynamic_cast<veins::TraCIMobility*>(mMobility)) {
        rad = traciMob->getHeading().getRad();
    } else {
        auto ori = mMobility->getCurrentOrientation();
        rad = std::atan2(ori.y, ori.x);
    }
    const double two_pi = 2.0 * M_PI;
    double normalized_rad = std::fmod(std::fmod(rad, two_pi) + two_pi, two_pi);
    return normalized_rad * radian;
}

// Maps the OMNeT++ module ID to an ITS StationID (uint32_t).
uint32_t ApplicationToVanetzaConverter::station_id() const {
    if (!mMobility) return 0u;
    return static_cast<uint32_t>(mMobility->getId());
}

// Converts the current simulation time to a vanetza::Clock::time_point
// (millisecond resolution) for use in CAM generationDeltaTime and
// DENM detectionTime / referenceTime fields.
vanetza::Clock::time_point ApplicationToVanetzaConverter::timestamp() const {
    using namespace std::chrono;
    auto ms = duration_cast<milliseconds>(duration<double>(simTime().dbl()));
    return vanetza::Clock::time_point(ms);
}

vanetza::asn1::Cam ApplicationToVanetzaConverter::buildCam() const
{
    vanetza::asn1::Cam cam;

    // --- ITS PDU Header ---
    cam->header.messageID       = ItsPduHeader__messageID_cam;
    cam->header.protocolVersion = 2;
    cam->header.stationID       = station_id();

    uint32_t now_ms = static_cast<uint32_t>(std::lround(simTime().dbl() * 1000.0)) % 65536;
    cam->cam.generationDeltaTime = static_cast<GenerationDeltaTime_t>(now_ms);

    // --- Basic Container ---
    auto& basic = cam->cam.camParameters.basicContainer;
    basic.stationType = StationType_passengerCar;

    vanetza::PositionFix fix;
    fix.latitude  = latitude();
    fix.longitude = longitude();
    vanetza::facilities::copy(fix, basic.referencePosition);

    // --- High Frequency Container ---
    auto& hfc = cam->cam.camParameters.highFrequencyContainer;
    hfc.present = HighFrequencyContainer_PR_basicVehicleContainerHighFrequency;
    auto& bvchf = hfc.choice.basicVehicleContainerHighFrequency;

    double spd_cm_s = speed().value() * 100.0;
    if (!std::isfinite(spd_cm_s) || spd_cm_s < 0.0 || spd_cm_s >= static_cast<double>(SpeedValue_unavailable))
        bvchf.speed.speedValue = SpeedValue_unavailable;
    else
        bvchf.speed.speedValue = static_cast<SpeedValue_t>(std::lround(spd_cm_s));
    bvchf.speed.speedConfidence = SpeedConfidence_unavailable;

    double heading_tenths = std::fmod(std::fmod(heading().value() * (180.0 / M_PI), 360.0) + 360.0, 360.0) * 10.0;
    if (!std::isfinite(heading_tenths))
        bvchf.heading.headingValue = HeadingValue_unavailable;
    else {
        heading_tenths = std::max(0.0, std::min(3600.0, heading_tenths));
        bvchf.heading.headingValue = static_cast<HeadingValue_t>(std::lround(heading_tenths));
    }
    bvchf.heading.headingConfidence = HeadingConfidence_unavailable;

    bvchf.driveDirection = DriveDirection_forward;
    bvchf.vehicleLength.vehicleLengthValue = VehicleLengthValue_unavailable;
    bvchf.vehicleLength.vehicleLengthConfidenceIndication = VehicleLengthConfidenceIndication_unavailable;
    bvchf.vehicleWidth = VehicleWidth_unavailable;
    bvchf.longitudinalAcceleration.longitudinalAccelerationValue      = LongitudinalAccelerationValue_unavailable;
    bvchf.longitudinalAcceleration.longitudinalAccelerationConfidence = AccelerationConfidence_unavailable;
    bvchf.curvature.curvatureValue      = CurvatureValue_unavailable;
    bvchf.curvature.curvatureConfidence = CurvatureConfidence_unavailable;
    bvchf.curvatureCalculationMode = CurvatureCalculationMode_unavailable;
    bvchf.yawRate.yawRateValue      = YawRateValue_unavailable;
    bvchf.yawRate.yawRateConfidence = YawRateConfidence_unavailable;

    return cam;
}

vanetza::asn1::Denm ApplicationToVanetzaConverter::buildDenm(uint16_t sequenceNumber,
                                                              int causeCode,
                                                              int subCauseCode) const
{
    vanetza::asn1::Denm denm;

    // --- ITS PDU Header ---
    denm->header.messageID       = ItsPduHeader__messageID_denm;
    denm->header.protocolVersion = 2;
    denm->header.stationID       = station_id();

    // --- Management Container ---
    auto& mgmt = denm->denm.management;
    mgmt.actionID.originatingStationID = station_id();
    mgmt.actionID.sequenceNumber       = sequenceNumber;

    uint64_t its_now = static_cast<uint64_t>(std::lround(simTime().dbl() * 1000.0));
    setTimestamp(mgmt.detectionTime, its_now);
    setTimestamp(mgmt.referenceTime, its_now);

    double lat_deg = latitude().value();
    double lon_deg = longitude().value();
    mgmt.eventPosition.latitude  = static_cast<Latitude_t>(std::lround(lat_deg * 1e7));
    mgmt.eventPosition.longitude = static_cast<Longitude_t>(std::lround(lon_deg * 1e7));
    mgmt.eventPosition.positionConfidenceEllipse.semiMajorConfidence  = SemiAxisLength_unavailable;
    mgmt.eventPosition.positionConfidenceEllipse.semiMinorConfidence  = SemiAxisLength_unavailable;
    mgmt.eventPosition.positionConfidenceEllipse.semiMajorOrientation = HeadingValue_unavailable;
    mgmt.eventPosition.altitude.altitudeValue      = AltitudeValue_unavailable;
    mgmt.eventPosition.altitude.altitudeConfidence = AltitudeConfidence_unavailable;

    if (!mgmt.validityDuration)
        mgmt.validityDuration = (ValidityDuration_t*)calloc(1, sizeof(ValidityDuration_t));
    *mgmt.validityDuration = 600;
    mgmt.stationType = StationType_passengerCar;

    // --- Situation Container ---
    denm->denm.situation = (SituationContainer_t*)calloc(1, sizeof(SituationContainer_t));
    if (denm->denm.situation) {
        denm->denm.situation->informationQuality     = 5;
        denm->denm.situation->eventType.causeCode    = causeCode;
        denm->denm.situation->eventType.subCauseCode = subCauseCode;
    }

    return denm;
}

void ApplicationToVanetzaConverter::setTimestamp(TimestampIts_t& ts, uint64_t ms)
{
    if (ts.buf) free(ts.buf);
    ts.size = 0;
    uint64_t temp = ms;
    do { ts.size++; temp >>= 8; } while (temp > 0);

    ts.buf = (uint8_t*)malloc(ts.size);
    for (size_t i = 0; i < ts.size; ++i)
        ts.buf[ts.size - 1 - i] = (ms >> (8 * i)) & 0xFF;
}

} // namespace veins
