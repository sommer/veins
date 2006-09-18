#ifndef __SCENARIO_H__
#define __SCENARIO_H__

#include "mixim.h"
#include <vector>

struct PatternDescription {
	cModuleType * type;
	int active;
	int msglength;
	double msginterval;
	std::vector <cPar*> *params;
	char name[10];
};

/**
 *	Manages possible node scenarios. Reads data from
 *	whatever file is defined as mixim.Scenario.inputfile in omnetpp.ini
 */

class ScenarioManager: public MiximBaseModule {
	Module_Class_Members(ScenarioManager, MiximBaseModule, 0);

protected:
	std::vector <PatternDescription*> *patterns; /** Set of PatternDescription's **/

	/**
	 * Reads in a scenario file
	 * @param filename The file to read from
	 */
	void readInputFile(const char * filename);

	void createModules();
	cModule * createModule(PatternDescription * pd, cModule * parent);

	/**
	 * Calls readInputFile() with the net.scenario.inputfile
	 * and then calls createModules() to build the necessary modules
	 */
	void initialize();

	void finish();
	void handleMessage(cMessage * msg);
	virtual ~ScenarioManager();
	
};

#endif
