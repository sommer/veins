#include "TestManager.h"

Define_Module(TestManager);

/**
 * Registers the passed TestModule with the passed name
 * at the database.
 */
void TestManager::registerModule(const std::string& name, TestModule* module) {
	modules[name] = module;
}

/**
 * Returns a pointer to the TestModule with the passed name.
 * Or null if no module with this name is registered.
 */
TestModule* TestManager::getModule(const std::string& name) const {
	ModuleMap::const_iterator it = modules.find(name);
	if(it == modules.end())
		return 0;
	else
		return it->second;
}
