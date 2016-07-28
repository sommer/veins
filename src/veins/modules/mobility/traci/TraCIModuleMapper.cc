#include "veins/modules/mobility/traci/TraCIModuleMapper.h"
#include <omnetpp.h>
#include <vector>

namespace omnetpp { }
using namespace omnetpp;

namespace Veins {

const TraCIModuleMapper::ModuleConfig& TraCIModuleMapper::getModuleConfig(const std::string& vehicleType) const
{
	std::map<std::string, ModuleConfig>::const_iterator found = mapping.find(vehicleType);
	if (found == mapping.end()) {
		found = mapping.find("*");
		if (found == mapping.end())
			throw cRuntimeError("No mapping found for vehicle type \"%s\"", vehicleType.c_str());
	}

	return found->second;
}

void TraCIModuleMapper::parseConfig(const std::string& types, const std::string& names, const std::string& displayStrings)
{
	typedef std::map<std::string, std::string> mappings_type;
	mappings_type moduleTypes = parseMappings(types, "moduleType", false);
	mappings_type moduleNames = parseMappings(names, "moduleName", false);
	mappings_type moduleDisplayStrings = parseMappings(displayStrings, "moduleDisplayString", true);

	std::map<std::string, ModuleConfig> staging;
	for (mappings_type::const_iterator type_it = moduleTypes.begin(); type_it != moduleTypes.end(); ++type_it) {
		const std::string& vehicleType = type_it->first;
		mappings_type::const_iterator name_it = moduleNames.find(vehicleType);
		mappings_type::const_iterator display_it = moduleDisplayStrings.find(vehicleType);

		//perform consistency check. for each vehicle type in moduleType there must be a vehicle type
		//in moduleName (and in moduleDisplayString if moduleDisplayString is not empty)
		if (name_it == moduleNames.end())
		    throw cRuntimeError("mapping of moduleType and moduleName does not match");
		else if (!moduleDisplayStrings.empty() && display_it == moduleDisplayStrings.end())
		    throw cRuntimeError("mapping of moduleType and moduleDisplayString does not match");

		ModuleConfig config;
		config.type = type_it->second;
		config.name = name_it->second;
		config.displayString = moduleDisplayStrings.empty() ? "" : display_it->second;
		staging[vehicleType] = config;
	}

	// swap temporary map and member map when no exception occurred
	std::swap(staging, mapping);
}

std::map<std::string, std::string> TraCIModuleMapper::parseMappings(const std::string& parameter, const std::string& parameterName, bool allowEmpty) const
{
	std::map<std::string, std::string> map;

	//tokenizer to split into mappings ("a=b c=d", -> ["a=b", "c=d"])
	cStringTokenizer typesTz(parameter.c_str(), " ");
	//get all mappings
	std::vector<std::string> typeMappings = typesTz.asVector();
	//and check that there exists at least one
	if (typeMappings.size() == 0) {
		if (!allowEmpty)
			throw cRuntimeError("parameter \"%s\" is empty", parameterName.c_str());
		else
			return map;
	}

	//loop through all mappings
	for (unsigned i = 0; i < typeMappings.size(); i++) {

		//tokenizer to find the mapping from vehicle type to module type
		std::string typeMapping = typeMappings[i];
		cStringTokenizer typeMappingTz(typeMapping.c_str(), "=");
		std::vector<std::string> mapping = typeMappingTz.asVector();

		if (mapping.size() == 1) {
			//we are where there is no actual assignment
			//"a": this is good
			//"a b=c": this is not
			if (typeMappings.size() != 1)
				//stop simulation with an error
				throw cRuntimeError("parameter \"%s\" includes multiple mappings, but \"%s\" is not mapped to any vehicle type", parameterName.c_str(), mapping[0].c_str());
			else
				//all vehicle types should be instantiated with this module type
				map["*"] = mapping[0];
		}
		else {
			//check that mapping is valid (a=b and not like a=b=c)
			if (mapping.size() != 2)
				throw cRuntimeError("invalid syntax for mapping \"%s\" for parameter \"%s\"", typeMapping.c_str(), parameterName.c_str());
			//check that the mapping does not already exist
			if (map.find(mapping[0]) != map.end())
				throw cRuntimeError("duplicated mapping for vehicle type \"%s\" for parameter \"%s\"", mapping[0].c_str(), parameterName.c_str());

			//finally save the mapping
			map[mapping[0]] = mapping[1];
		}
	}

	return map;
}

} // namespace Veins
