#ifndef VEINS_WORLD_TRACI_TRACIVEHICLESIGNALS_H
#define VEINS_WORLD_TRACI_TRACIVEHICLESIGNALS_H

namespace Veins {

class TraCIVehicleSignal
{
	public:
		// see http://sumo.dlr.de/wiki/TraCI/Vehicle_Signalling
		enum SignalFlags {
			VEH_SIGNAL_NONE = 0,
			VEH_SIGNAL_BLINKER_RIGHT = 1 << 0,
			VEH_SIGNAL_BLINKER_LEFT = 1 << 1,
			VEH_SIGNAL_BLINKER_EMERGENCY = 1 << 2,
			VEH_SIGNAL_BRAKELIGHT = 1 << 3,
			VEH_SIGNAL_FRONTLIGHT = 1 << 4,
			VEH_SIGNAL_FOGLIGHT = 1 << 5,
			VEH_SIGNAL_HIGHBEAM = 1 << 6,
			VEH_SIGNAL_BACKDRIVE = 1 << 7,
			VEH_SIGNAL_WIPER = 1 << 8,
			VEH_SIGNAL_DOOR_OPEN_LEFT = 1 << 9,
			VEH_SIGNAL_DOOR_OPEN_RIGHT = 1 << 10,
			VEH_SIGNAL_EMERGENCY_BLUE = 1 << 11,
			VEH_SIGNAL_EMERGENCY_RED = 1 << 12,
			VEH_SIGNAL_EMERGENCY_YELLOW = 1 << 13
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
