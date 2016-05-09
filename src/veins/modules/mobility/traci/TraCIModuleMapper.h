#ifndef VEINS_WORLD_TRACI_TRACIMODULEMAPPER_H
#define VEINS_WORLD_TRACI_TRACIMODULEMAPPER_H

#include <map>
#include <string>

namespace Veins {

/**
 * TraCIModuleMapper maps vehicle types to module configurations
 *
 * A module configuration consists of module type, module name and an optional display string.
 *
 * Possible syntaxes of configuration strings:
 *
 * "a"          : assign module type "a" to all nodes (for backward compatibility)
 * "a=b"        : assign module type "b" to vehicle type "a". the presence of any other vehicle type in the simulation will cause the simulation to stop
 * "a=b c=d"    : assign module type "b" to vehicle type "a" and "d" to "c". the presence of any other vehicle type in the simulation will cause the simulation to stop
 * "a=b c=d *=e": everything which is not of vehicle type "a" or "b", assign module type "e"
 * "a=b c="     : for vehicle type "c" no module should be instantiated
 * "a=b c=d *=" : everything which is not of vehicle type a or c should not be instantiated
 */
class TraCIModuleMapper
{
	public:
		struct ModuleConfig
		{
			std::string type; /**< module type to be used in the simulation for managed vehicle */

			std::string name; /**< module name to be used in the simulation for managed vehicle */
			std::string displayString; /**< module displayString to be used in the simulation for managed vehicle */
		};

		/**
		 * Parse configuration and create internal mapping
		 *
		 * @param types module types, see class header for syntax description
		 * @param names module names, see class header for syntax description
		 * @param displayStrings module display strings, see class header for syntax description
		 *
		 */
		void parseConfig(const std::string& types, const std::string& names, const std::string& displayStrings);

		/**
		 * Get module configuration for a specific vehicle type
		 *
		 * @throws cRuntimeError if no mapping exists for provided vehicle type
		 * @param vehicleType vehicle type provided by TraCI
		 * @return module configuration
		 */
		const ModuleConfig& getModuleConfig(const std::string& vehicleType) const;

	private:
		std::map<std::string, std::string> parseMappings(const std::string& parameter, const std::string& parameterName, bool allowEmpty) const;

		// maps from vehicle type to config (type, name and display string)
		std::map<std::string, ModuleConfig> mapping;
};

} // namespace Veins

#endif
