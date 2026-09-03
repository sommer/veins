#pragma once

#include <vanetza/dcc/entity.hpp>
#include <vanetza/dcc/transmit_rate_control.hpp>
#include <vanetza/dcc/channel_probe_processor.hpp>
#include <vanetza/dcc/channel_load.hpp>
#include <vanetza/common/clock.hpp>
#include <veins/modules/application/ieee80211p/ApplicationToVanetzaConverter.h>
#include <chrono>
#include <cstdint>

namespace veins {

/**
 * Implements the ETSI EN 302 637-2 §6.1.3 CAM generation rate control logic.
 *
 * Extends vanetza::dcc::TransmitRateControl to provide:
 *   - A minimum inter-CAM interval (T_GenCamMin = 100 ms)
 *   - A maximum inter-CAM interval (T_GenCamMax = 1000 ms)
 *   - Dynamic interval adjustment based on vehicle dynamics thresholds
 *     (heading, position, speed deltas)
 *
 * The interval is reduced when dynamics change significantly, and reset
 * to T_GenCamMax after N_GenCam consecutive dynamic-triggered CAMs.
 */
class VeinsTransmitRateControl : public vanetza::dcc::TransmitRateControl {
public:
    /**
     * Returns the minimum transmission delay for a CAM.
     * Fixed to T_GenCamMin (100 ms) as per ETSI standard.
     */
    vanetza::Clock::duration delay(const vanetza::dcc::Transmission&) override;

    /**
     * Returns the current CAM generation interval (mTGenCam).
     * Ranges between T_GenCamMin (100 ms) and T_GenCamMax (1000 ms)
     * depending on vehicle dynamics.
     */
    vanetza::Clock::duration interval(const vanetza::dcc::Transmission&) override;

    /**
     * Called after each transmission to update internal state.
     * Currently a no-op — DCC congestion control not yet integrated.
     */
    void notify(const vanetza::dcc::Transmission&) override;

    /**
     * Evaluates ETSI §6.1.3 CAM generation conditions.
     *
     * Returns true if a new CAM should be generated, based on:
     *   - elapsed >= T_GenCamMin AND any dynamics threshold exceeded, OR
     *   - elapsed >= mTGenCam (periodic fallback)
     *
     * Updates mTGenCam and mNGenCamCount accordingly:
     *   - On dynamics trigger: mTGenCam = elapsed, mNGenCamCount++
     *   - After N_GenCam consecutive dynamic CAMs: reset to T_GenCamMax
     *   - On periodic trigger: reset mTGenCam to T_GenCamMax
     *
     * @param deltaHeading_deg  Absolute heading change since last CAM [degrees]
     * @param deltaPos_m        Absolute position change since last CAM [meters]
     * @param deltaSpeed_ms     Absolute speed change since last CAM [m/s]
     * @param elapsed           Time elapsed since last CAM
     */
    bool applyEtsiGenerationRules(double deltaHeading_deg,
                            double deltaPos_m,
                            double deltaSpeed_ms,
                            vanetza::Clock::duration elapsed);

private:
    /// Minimum CAM generation interval (ETSI EN 302 637-2 §6.1.3)
    static constexpr auto T_GenCamMin = std::chrono::milliseconds(100);
    /// Maximum CAM generation interval (ETSI EN 302 637-2 §6.1.3)
    static constexpr auto T_GenCamMax = std::chrono::milliseconds(1000);

    /// Heading change threshold to trigger a CAM [degrees]
    static constexpr double HEADING_THRESHOLD_DEG = 4.0;
    /// Position change threshold to trigger a CAM [meters]
    static constexpr double POSITION_THRESHOLD_M  = 4.0;
    /// Speed change threshold to trigger a CAM [m/s]
    static constexpr double SPEED_THRESHOLD_MS    = 0.5;

    /// Current CAM generation interval; dynamically adjusted
    vanetza::Clock::duration mTGenCam = T_GenCamMax;
    /// Counter of consecutive dynamics-triggered CAMs
    int mNGenCamCount = 0;
    /// Number of consecutive dynamic CAMs before resetting interval to T_GenCamMax
    static constexpr int N_GenCam = 3;
};

/**
 * Stub implementation of vanetza::dcc::ChannelProbeProcessor.
 *
 * Receives channel load (CBR) measurements from the MAC layer.
 * Currently a no-op — DCC congestion-based rate adaptation not yet integrated.
 * Placeholder for future FlowControl / DCC integration (Module 2).
 */
class VeinsChannelProbeProcessor : public vanetza::dcc::ChannelProbeProcessor {
public:
    /**
     * Called by the MAC layer to report the current channel load.
     * @param load  Channel busy ratio measurement
     */
    void indicate(vanetza::dcc::ChannelLoad load) override;
};

/**
 * Central DCC entity for CAM/DENM management in the Veins simulation.
 *
 * Extends vanetza::dcc::Entity and acts as the bridge between the
 * application layer (VanetzaToVeinsBridge) and the Vanetza DCC stack.
 *
 * Responsibilities:
 *   1. Rate Control  — delegates to VeinsTransmitRateControl (ETSI §6.1.3)
 *   2. Channel Probe — delegates to VeinsChannelProbeProcessor (stub)
 *   3. ASN.1 Population — builds fully populated CAM and DENM structures
 *      ready for encoding and transmission
 */
class VanetzaEntityImplementation : public vanetza::dcc::Entity {
public:
    VanetzaEntityImplementation() = default;
    ~VanetzaEntityImplementation() = default;

    /**
     * Returns the TransmitRateControl interface (VeinsTransmitRateControl).
     * Used by VanetzaToVeinsBridge to query the current CAM generation interval.
     */
    vanetza::dcc::TransmitRateControl& transmit_rate_control() override;

    /**
     * Returns the ChannelProbeProcessor interface (VeinsChannelProbeProcessor).
     * Used by the MAC layer to report channel load measurements.
     */
    vanetza::dcc::ChannelProbeProcessor& channel_probe_processor() override;

    /**
     * Forwards the CAM generation check to VeinsTransmitRateControl.
     * Called by VanetzaToVeinsBridge::checkCamGeneration() after computing deltas.
     *
     * @param deltaHeading_deg  Absolute heading change since last CAM [degrees]
     * @param deltaPos_m        Absolute position change since last CAM [meters]
     * @param deltaSpeed_ms     Absolute speed change since last CAM [m/s]
     * @param elapsed           Time elapsed since last CAM
     * @return true if a new CAM should be generated
     */
    bool forwardCamGenerationCheck(double deltaHeading_deg,
                            double deltaPos_m,
                            double deltaSpeed_ms,
                            vanetza::Clock::duration elapsed);

private:
    VeinsTransmitRateControl   mTrc;  ///< ETSI §6.1.3 rate control implementation
    VeinsChannelProbeProcessor mCpp;  ///< Channel load processor (stub)

};

} // namespace veins
