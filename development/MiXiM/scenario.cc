#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "pattern.h"
#include "scenario.h"

Define_Module( ScenarioManager );

void ScenarioManager::initialize() {
	MiximBaseModule::initialize();
	const char * fname = getStringParameter("inputfile", "").c_str();
	
	if (fname[0] == 0) {
		fprintf(stderr, "Scenario: inputfile must be specified\n");
		abort();
	}
	
	assert(fname);
	patterns = new std::vector<PatternDescription*>();
	readInputFile(fname);
	createModules();
}

void ScenarioManager::finish() {
	unsigned int i, j;
	for(i = 0; i < patterns->size(); i++) {
		// Print some info for scripts
		printf_nr(PRINT_STATS, "msglen=%d msginterval=%f", (*patterns)[i]->msglength, (*patterns)[i]->msginterval);
		
		for(j = 0; j < (*patterns)[i]->params->size(); j++) {
			cPar *p = (*(*patterns)[i]->params)[j];
			printf_clr(PRINT_STATS, " %s=%s", p->name(), (const char *)(*p));
		}
		printf_clr(PRINT_STATS, "\n");

	}
	MiximBaseModule::finish();
}

ScenarioManager::~ScenarioManager() {
	unsigned int i, j;
	for(i = 0; i < patterns->size(); i++) {
		for(j = 0; j < (*patterns)[i]->params->size(); j++) {
			delete (*(*patterns)[i]->params)[j];
		}
	}
	delete patterns;
}

void ScenarioManager::handleMessage(cMessage * msg) {
	assert(false); //shouldn't
}

void ScenarioManager::readInputFile(const char * filename) {
	assert(filename);

	FILE * fin = fopen(filename, "rt");
	if(!fin) {
		error("%s: %s", filename, strerror(errno));
		return;
	}

	char linebuf[1000];
	int linenr;
	static const char delim[] = " \r\n\t";
	int active_count = 0, idle_count = 0;
	for(linenr = 1; fgets(linebuf, 1000, fin); linenr++) {
		// test active line
		char * active;
		if(! (active = strtok(linebuf, delim)) ) {
			//printf("%d: ignoring empty line", linenr);
			continue;
		}
		assert(active[0]);
		// test comment
		if(active[0] == '#') {
			//printf("%d: ignoring comment", linenr);
			continue;
		}
		// create pattern descr
		PatternDescription * pd = new PatternDescription;
		pd->params = new std::vector<cPar*>;
		
		// check active / idle
		if(strcmp(active, "active") == 0) {
			pd->active = 1;
			sprintf(pd->name,"active%d", ++active_count);
		} else if(strcmp(active, "idle") == 0) {
			pd->active = 0;
			sprintf(pd->name,"idle%d", ++idle_count);
		} else {
			error("%s (%d): parse error", filename, linenr);
			return;
		}

		// get class name
		char * classname;
		if(! (classname = strtok(NULL, delim)) ) {
			error("%s (%d): parse error", filename, linenr);
			return;
		}

		// find corresponding module type
		if(! (pd->type = findModuleType(classname)) ) {
			error("%s (%d): no such module type: %s",
					filename, linenr, classname);
			return;
		}

		// msg length
		char * msglen;
		if(! (msglen = strtok(NULL, delim)) ) {
			error("%s (%d): parse error", filename, linenr);
			return;
		}
		if( (pd->msglength = atoi(msglen)) <=0 ) {
			error("%s (%d): invalid message length",
					filename, linenr);
			return;
		}

		// msg interval
		char * msgps;
		if(! (msgps = strtok(NULL, delim)) ) {
			error("%s (%d): parse error", filename, linenr);
			return;
		}
		char * endptr = NULL;
		double dmsgps = strtod(msgps, &endptr);
		if(!endptr || dmsgps < 0.00001) {
			error("%s (%d): invalid msg/s", filename, linenr);
			return;
		}
		pd->msginterval = 1.0/dmsgps;
		
		// get parameters
		char * param;
		while( (param = strtok(NULL, delim)) != NULL) {
			// param should be xyz=value
			char * eq = strchr(param, '=');
			if(!eq) {
				error("%s (%d): parse error",
						filename, linenr);
				return;
			}
			assert(*eq == '=');
			*eq = 0; // split name and value
			cPar * par = new cPar(param);
			(*par) = eq+1; // set value
			pd->params->push_back(par); // record in array
		}

		// remember all this
		patterns->push_back(pd);

	} // readline
	fclose(fin);
}
	
void ScenarioManager::createModules() {
	const int num_nodes = strtol(ev.getParameter(simulation.runNumber(), "net.nodeCount").c_str(), NULL, 0);

	for(int i=0; i<num_nodes; i++) {
		cModule * node = parentModule()->submodule("nodes", i);
		assert(node);
		cModule * appsel = node->submodule("software")->submodule("application");
		assert(appsel);

		appsel->addGate("toPattern", GateDir_Output);
		appsel->addGate("fromPattern", GateDir_Input);
		appsel->setGateSize("toPattern", patterns->size());
		appsel->setGateSize("fromPattern", patterns->size());

		for(unsigned int j=0; j<patterns->size(); j++) {
			PatternDescription * pd = (*patterns)[j];
			cModule * mod = createModule(pd, appsel);
			assert(mod);
			assert(appsel->findGate("toPattern", j)>=0);
			assert(appsel->findGate("fromPattern", j)>=0);
			assert(mod->findGate("in")>=0);
			assert(mod->findGate("out")>=0);
			connect(appsel,
				appsel->findGate("toPattern", j),
				NULL,
				mod,
				mod->findGate("in"));
			connect(mod,
				mod->findGate("out"),
				NULL,
				appsel,
				appsel->findGate("fromPattern", j));
			mod->scheduleStart(simTime());
			mod->callInitialize();
		}
	}
}

cModule * ScenarioManager::createModule(PatternDescription * pd, 
		cModule * parent) 
{
	// create module
	assert(pd);
	assert(pd->type);
	cModule * mod = pd->type->create(pd->name, parent);
	assert(mod);

	if(!(dynamic_cast<Pattern *>(mod))) {
		error("module is not of Pattern type");
		return 0;
	}

	// check the type
	Pattern * module = (Pattern *)mod;
	
	// set info
	module->is_active = pd->active;
	module->msglength = pd->msglength;
	module->msginterval = pd->msginterval;

	// set parameters
	unsigned int j;
	for(j=0; j<pd->params->size(); j++) {
		cPar *p = (*pd->params)[j];
		assert_type(p, cPar *);
		if(!module->hasPar(p->name())) {
			error("no such parameter %s", p->name());
			return 0;
		}
		module->par(p->name()) = (*p);
	}
	module->buildInside();

	return module;
}
