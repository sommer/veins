#pragma once

// Standard
#include <cstdint>

// Veins / OMNeT++
#include <omnetpp.h>
#include "veins/base/modules/BaseMobility.h"

// Vanetza types
#include "vanetza/common/clock.hpp"
#include "vanetza/units/angle.hpp"
#include "vanetza/units/velocity.hpp"
#include "vanetza/units/acceleration.hpp"
#include <vanetza/asn1/cam.hpp>
#include <vanetza/asn1/denm.hpp>
#include <vanetza/asn1/its/TimestampIts.h>
#include <vanetza/facilities/cam_functions.hpp>
#include <vanetza/common/position_fix.hpp>

#include "veins/modules/mobility/traci/TraCIMobility.h"

using namespace omnetpp;

namespace veins
{

using veins::TraCIMobility;

// ============================================================
//  ApplicationToVanetzaConverter
//
//  Bridge between the Veins mobility module (BaseMobility /
//  TraCIMobility) and the Vanetza-style vehicle data interface.
//  Used by buildCam() and buildDenm() to read position, speed,
//  heading and station ID when populating message structures.
// ============================================================
class VEINS_API ApplicationToVanetzaConverter : public cSimpleModule{
public:
    ApplicationToVanetzaConverter();
    explicit ApplicationToVanetzaConverter(BaseMobility* mob);

    // OMNeT++ module interface
    void initialize(int stage) override;
    void handleMessage(omnetpp::cMessage* msg) override;

    // Geographic position (converted from SUMO Cartesian metres)
    vanetza::units::GeoAngle latitude()  const;
    vanetza::units::GeoAngle longitude() const;

    // Kinematic data read from TraCIMobility (or fallback BaseMobility)
    vanetza::units::Velocity     speed()       const;
    vanetza::units::Angle        heading()     const;
    vanetza::units::Acceleration acceleration() const;

    // ITS station identifier (mapped from OMNeT++ module ID)
    uint32_t station_id() const;

    // Simulation timestamp converted to vanetza::Clock::time_point
    vanetza::Clock::time_point timestamp() const;

    // Optional: event cause/subcause for DENM generation
    int get_event_cause()    const;
    int get_event_subcause() const;

    // ============================================================
    // ASN.1 message builders
    // Populate CAM and DENM structures from the current vehicle state.
    // ============================================================

    // Builds a fully populated ASN.1 CAM structure from the current vehicle state.
    vanetza::asn1::Cam  buildCam() const;

    // Builds a fully populated ASN.1 DENM structure from the current vehicle state.
    vanetza::asn1::Denm buildDenm(uint16_t sequenceNumber,
                                   int causeCode,
                                   int subCauseCode) const;

private:
    // Encodes a 64-bit millisecond timestamp into an ASN.1 TimestampIts_t (BER).
    static void setTimestamp(TimestampIts_t& ts, uint64_t ms);
    BaseMobility* mMobility; ///< Pointer to the underlying mobility module

};

} // namepspace veins
