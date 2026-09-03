#pragma once

#include <memory>
#include <string>

#include "vanetza/asn1/cam.hpp"
#include "vanetza/asn1/denm.hpp"
#include "vanetza/asn1/asn1c_wrapper.hpp"
#include "vanetza/common/clock.hpp"
#include "vanetza/units/angle.hpp"
#include "vanetza/units/velocity.hpp"
#include "vanetza/units/acceleration.hpp"
#include "vanetza/asn1/its/CauseCodeType.h"
#include <vanetza/facilities/cam_functions.hpp>
#include <vanetza/asn1/its/ReferencePosition.h>
#include <vanetza/asn1/its/Heading.h>
#include <veins/modules/application/ieee80211p/ApplicationToVanetzaConverter.h>
#include <veins/modules/application/ieee80211p/VanetzaEntityImplementation.h>

// Veins messages
#include "veins/modules/messages/CamMessage_m.h"
#include "veins/modules/messages/DenmMessage_m.h"

using namespace omnetpp;

namespace veins {

class CamMessage;
class DenmMessage;

class VanetzaToVeinsBridge: public cSimpleModule
{
public:
    VanetzaToVeinsBridge();
    ~VanetzaToVeinsBridge(){}

    // ============================
    // TX helpers
    // ============================

    // populate only CAM/DENM parts (called from DemoBaseApplLayer)
    void populateCAM(CamMessage* cam,
                     ApplicationToVanetzaConverter* vdp,
                     int headerLength,
                     int beaconUserPriority);

    void populateDENM(DenmMessage* denm,
                      ApplicationToVanetzaConverter* vdp,
                      uint16_t& sequenceNumber,
                      int causeCode,
                      int subCauseCode,
                      int headerLength,
                      int denmUserPriority,
                      int denmLengthBits);

    // ============================
    // RX helpers
    // ============================

    void onCAM(CamMessage* camMsg, const std::string& nodeName);
    void onDENM(DenmMessage* denmMsg, const std::string& nodeName);

    // ============================
    // Vanetza entity access
    // ============================

    vanetza::dcc::Entity& getVanetzaEntity();

    // ============================
    // CAM generation logic
    // ============================

    // Returns true if position, speed or heading deltas exceed ETSI thresholds,
    // triggering a new CAM generation
    bool computeAndCheckCamDeltas(const ApplicationToVanetzaConverter& vdp);

    // Updates internal state after a CAM has been successfully sent
    // (e.g. resets delta counters and last known position/speed/heading)
    void notifyCamSent(const ApplicationToVanetzaConverter& vdp);

    // DENM message cause code to string
    std::string denmCauseToString(int cause);

    void initialize() override;
    void handleMessage(omnetpp::cMessage* msg) override;

private:
    std::unique_ptr<VanetzaEntityImplementation> vanetzaEntityImplementation;

    // --- Last CAM reference state (for ETSI §6.1.3 delta checks) ---
    /** ASN.1 structured position of the vehicle when the last CAM was generated */
        ReferencePosition_t         mLastCamPos;

        /** Heading of the vehicle at the last CAM generation (in tenths of degrees) */
        HeadingValue_t              mLastCamHeading = HeadingValue_unavailable;

        /** Speed of the vehicle at the last CAM generation (in meters per second) */
        double                      mLastCamSpeed_ms = 0.0;

        /** Timestamp of the last CAM generation using Vanetza's internal clock */
        vanetza::Clock::time_point  mLastCamTime;

        /** Last CAM generation timestamp using OMNeT++ simulation time (for easy interval logging) */
        simtime_t                   mLastCamTime_sim = 0;

        /** Flag indicating if the reference state (mLastCam*) is valid for delta computation */
        bool                        mLastCamValid = false;

        /** Signal emitted when a CAM is sent by this node */
        simsignal_t                 camSentSignal;

        /** Signal emitted when a CAM is received by this node */
        simsignal_t                 camReceivedSignal;

        /** Signal emitted when a DENM is sent by this node */
        simsignal_t                 denmSentSignal;

        /** Signal emitted when a DENM is received by this node */
        simsignal_t                 denmReceivedSignal;

        // Vectors for ETSI EN 302 637-2 compliance validation
        // Recorded at each CAM transmission to correlate vehicle dynamics with CAM generation
        cOutVector                  vCamInterval;   // Time elapsed since the last CAM (seconds)
        cOutVector                  vSpeed;         // Vehicle speed at CAM generation time (m/s)
        cOutVector                  vHeading;       // Vehicle heading at CAM generation time (degrees, 0–360)
};

} // namespace veins
