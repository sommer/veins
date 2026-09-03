#include <omnetpp.h>
#include <veins/modules/application/ieee80211p/VanetzaEntityImplementation.h>
#include <cmath>
#include <cstdlib>

using namespace omnetpp;

namespace veins {

// ── VeinsTransmitRateControl ──────────────────────────────────────────────────

// Returns the minimum allowed delay before a CAM can be transmitted.
// Fixed to T_GenCamMin (100 ms) regardless of channel conditions,
// as DCC congestion-based delay adaptation is not yet integrated.
vanetza::Clock::duration VeinsTransmitRateControl::delay(const vanetza::dcc::Transmission&)
{
    return T_GenCamMin;
}

// Returns the current CAM generation interval (mTGenCam).
// This value is dynamically adjusted by applyEtsiGenerationRules():
// reduced when vehicle dynamics change, reset to T_GenCamMax otherwise.
vanetza::Clock::duration VeinsTransmitRateControl::interval(const vanetza::dcc::Transmission&)
{
    return mTGenCam;
}

// Called by Vanetza after each transmission to allow DCC feedback.
// Currently a no-op — congestion-based interval adaptation not yet integrated.
void VeinsTransmitRateControl::notify(const vanetza::dcc::Transmission&)
{
}

bool VeinsTransmitRateControl::applyEtsiGenerationRules(double deltaHeading_deg,
                                                   double deltaPos_m,
                                                   double deltaSpeed_ms,
                                                   vanetza::Clock::duration elapsed)
{
    // Condition 1 (ETSI §6.1.3): elapsed time exceeds T_GenCamMin AND
    // at least one dynamics threshold is exceeded → dynamics-triggered CAM
    if (elapsed >= T_GenCamMin) {
        bool dynamicsChanged =
            (deltaHeading_deg > HEADING_THRESHOLD_DEG) ||
            (deltaPos_m       > POSITION_THRESHOLD_M)  ||
            (deltaSpeed_ms    > SPEED_THRESHOLD_MS);

        if (dynamicsChanged) {
            mTGenCam = elapsed;
            mNGenCamCount++;
            if (mNGenCamCount >= N_GenCam) {
                mTGenCam      = T_GenCamMax;
                mNGenCamCount = 0;
            }
            return true;
        }
    }

    // Condition 2 (ETSI §6.1.3): elapsed time exceeds current mTGenCam
    // → periodic fallback CAM; reset interval to T_GenCamMax
    if (elapsed >= mTGenCam) {
        mNGenCamCount = 0;
        mTGenCam      = T_GenCamMax;
        return true;
    }

    return false;
}

// ── VeinsChannelProbeProcessor ────────────────────────────────────────────────

// Stub: CBR measurement received from MAC layer but not yet processed.
// Future integration point for DCC congestion-based rate adaptation.
void VeinsChannelProbeProcessor::indicate(vanetza::dcc::ChannelLoad)
{
}

// ── VanetzaEntityImplementation ────────────────────────────────────────────────────────────

vanetza::dcc::TransmitRateControl& VanetzaEntityImplementation::transmit_rate_control()
{
    return mTrc;
}

vanetza::dcc::ChannelProbeProcessor& VanetzaEntityImplementation::channel_probe_processor()
{
    return mCpp;
}

// Bridge method between VanetzaToVeinsBridge and VeinsTransmitRateControl.
// Receives pre-computed delta values from the Adapter and delegates
// the ETSI generation decision to the TRC component.
bool VanetzaEntityImplementation::forwardCamGenerationCheck(double deltaHeading_deg,
                                         double deltaPos_m,
                                         double deltaSpeed_ms,
                                         vanetza::Clock::duration elapsed)
{
    return mTrc.applyEtsiGenerationRules(deltaHeading_deg, deltaPos_m, deltaSpeed_ms, elapsed);
}

} // namespace veins
