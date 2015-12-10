#ifndef VEINS_WORLD_TRACI_TRACIVEHICLESIGNALS_H
#define VEINS_WORLD_TRACI_TRACIVEHICLESIGNALS_H

namespace Veins {

class TraCIVehicleSignal
{
	public:
		// see http://sumo.dlr.de/wiki/TraCI/Vehicle_Signalling
		enum SignalFlags {
			VEH_SIGNAL_NONE = 0,
			VEH_SIGNAL_BLINKER_RIGHT = 1,
			VEH_SIGNAL_BLINKER_LEFT = 2,
			VEH_SIGNAL_BLINKER_EMERGENCY = 4,
			VEH_SIGNAL_BRAKELIGHT = 8,
			VEH_SIGNAL_FRONTLIGHT = 16,
			VEH_SIGNAL_FOGLIGHT = 32,
			VEH_SIGNAL_HIGHBEAM = 64,
			VEH_SIGNAL_BACKDRIVE = 128,
			VEH_SIGNAL_WIPER = 256,
			VEH_SIGNAL_DOOR_OPEN_LEFT = 512,
			VEH_SIGNAL_DOOR_OPEN_RIGHT = 1024,
			VEH_SIGNAL_EMERGENCY_BLUE = 2048,
			VEH_SIGNAL_EMERGENCY_RED = 4096,
			VEH_SIGNAL_EMERGENCY_YELLOW = 8192
		};

		TraCIVehicleSignal();
		TraCIVehicleSignal(SignalFlags);

		/**
		 * Test if certain flag is set
		 * \param flag Test this flag (might be OR-ed)
		 * \return true if (all given) flags are set
		 */
		bool test(SignalFlags flag) const;

		/**
		 * Enable certain flags
		 * \param flag Set this flag (might be OR-ed)
		 */
		void set(SignalFlags flag);

		/**
		 * Disable certain flags
		 * \param flag Unset this flag (might be OR-ed)
		 */
		void clear(SignalFlags flag);

		/**
		 * Test if flags have been set previously
		 * \return true if signal object is usable
		 */
		operator bool() const;

	private:
		int flags;
		bool undef;
};

} // namespace Veins

#endif // VEINS_WORLD_TRACI_TRACIVEHICLESIGNALS_H
