#ifndef TESTMANAGER_H_
#define TESTMANAGER_H_

#include <omnetpp.h>
#include <map>

class TestModule;

/**
 * The TestManager is an Omnet module which is used by every 
 * TestModule in the simulation to get access to the other 
 * TestModules.
 */
class TestManager:public cSimpleModule {
protected:
	typedef std::map<std::string, TestModule*> ModuleMap;
	
	ModuleMap modules;
public:
	/**
	 * Registers the passed TestModule with the passed name
	 * at the database.
	 */
	void registerModule(const std::string& name, TestModule* module);
	
	/**
	 * Returns a pointer to the TestModule with the passed name.
	 * Or null if no module with this name is registered.
	 */
	TestModule* getModule(const std::string& name) const;
};

#endif /*TESTMANAGER_H_*/
