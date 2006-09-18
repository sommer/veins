#include <math.h>
#include <time.h>

#include <qapplication.h>
#include <qlineedit.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qregexp.h>

#include <dirent.h>
#include <libgen.h>

#include <map>

#include "generatorwizard.h"
#include "../mixim_generator.h"

using namespace std;

struct ltstr {
	bool operator()(const char* s1, const char* s2) const {
		return strcmp(s1, s2) < 0;
	}
};

typedef multimap<const char *, const char *, ltstr> modules;
typedef QMap<QString, QString> ComponentMap;

int componentFinder(const struct dirent* entry) {
//	printf("Checking entry: %s\t:", entry->d_name);
	if (entry->d_type == DT_REG) {
		char end[3];
		unsigned length = strlen(entry->d_name);
		end[0] = entry->d_name[length - 3];
		end[1] = entry->d_name[length - 2];
		end[2] = entry->d_name[length - 1];
		if (strncmp(end, ".cc", 3) == 0) {
//			printf("match\n");
			return 1; // match found
		}
	}
//	 else {
//		printf("not a file");
//	}
//	printf("\n");
	return 0;
}

int main( int argc, char ** argv ) {
	int ret;
	char input[512];
	char* argvCpy;

	argvCpy = (char*)malloc(strlen(argv[0]) + 1);
	strcpy(argvCpy, argv[0]);

	QApplication a( argc, argv );
	generatorWizard w;

	// get dynamic parts here (if possible)
	int result, i;
	size_t lineLength;
	ssize_t read = 0;
	FILE* file;
	char* lineContents;
	char* sourceDirString = (char*)malloc(1024);
	char* fileString = (char*)malloc(1024);
	struct dirent** files;
	QRegExp rx("^\\s*Define_Module_Like\\s*\\(\\s*(\\w*)\\s*,\\s*(\\w*)\\s*\\)");
	QString captured;
	modules map;
	pair<modules::iterator, modules::iterator> modIt;

	if (sourceDirString == NULL 
		|| fileString == NULL
		|| argvCpy == NULL)
	{
		printf("Memory trouble\n");
		goto skipComponentScan;
	}

	result = sprintf(sourceDirString, "%s/../../", dirname(argvCpy));
	if (result < 0 || result > 1024) {
		printf("Dir length trouble\n");
		goto skipComponentScan;
	}
	
	//printf("Opening %s to look for MiXiM components...\n", sourceDirString);
	result = scandir(sourceDirString, &files, componentFinder, alphasort);
	i = 0;
	while (result-- > 0) {
		int searchRes;
		sprintf(fileString, "%s%s", sourceDirString, files[i++]->d_name);
		//printf("Reading %s\n", fileString);
		file = fopen(fileString, "r");
		read = 0;
		if (file == NULL) {
			perror("Couldn't read file");
		} else {
			while (read != -1) {
				lineContents = NULL;
				lineLength = 0;
				read = getline(&lineContents, &lineLength, file);
				// do the regexp
				searchRes = rx.search(lineContents);
				if (searchRes != -1) {
					captured = rx.cap(0);
					//printf("Found component: %s is a %s module\n", rx.cap(1).latin1(), rx.cap(2).latin1());
					map.insert(pair<const char* const, const char* const>(strdup(rx.cap(2).latin1()), strdup(rx.cap(1).latin1())));
				}
			}
			fclose(file);
		}
	}
	// nodes
	modIt = map.equal_range("NodeClass");
	w.nodeTypeCB->clear();

	for (modules::iterator it = modIt.first; it != modIt.second; it++)
	{
		w.nodeTypeCB->insertItem(it->second);
	}
	// macs
	modIt = map.equal_range("MacClass");
	w.macLayerCB->clear();

	for (modules::iterator it = modIt.first; it != modIt.second; it++)
	{
		w.macLayerCB->insertItem(it->second);
	}
	// routing
	modIt = map.equal_range("Routing");
	w.routingCB->clear();

	for (modules::iterator it = modIt.first; it != modIt.second; it++)
	{
		w.routingCB->insertItem(it->second);
	}
	// radios
	modIt = map.equal_range("RadioClass");
	w.radioChipCB->clear();

	for (modules::iterator it = modIt.first; it != modIt.second; it++)
	{
		w.radioChipCB->insertItem(it->second);
	}
	// applications
	modIt = map.equal_range("Application");
	w.applicationNameCB->clear();
	w.applicationNameCB->insertItem("");

	for (modules::iterator it = modIt.first; it != modIt.second; it++)
	{
		w.applicationNameCB->insertItem(it->second);
	}

skipComponentScan:
	w.show();
	a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
	ret = a.exec();
	
	init("omnetpp.ini");

	// write config file here
	
	/* general part */
	addConfig("[General]");
	addConfigInt("ini-warnings", 1);
	addConfigInt("debug-on-errors", 1);
	addConfigString("network", "net");
	
	QTime maxSim = w.maxSimTime->time();
	sprintf(input, "%dd%dh%dm%ds%c", w.maxSimDaysSB->value(), maxSim.hour(), maxSim.minute(), maxSim.second(), '\0');
	writeConfig("sim-time-limit", input, false);
	QTime maxCPU = w.maxCPUTime->time();
	sprintf(input, "%dd%dh%dm%ds%c", w.maxCPUDaysSB->value(), maxCPU.hour(), maxCPU.minute(), maxCPU.second(), '\0');
	writeConfig("cpu-time-limit", input, false);

	writeConfig("num-rngs", "6", false);
	writeConfig("rng-class", "cLCG32", false);
	writeConfig("seed-0-lcg32 ", "1328964554", false);
	writeConfig("seed-1-lcg32 ", "1328964554", false);
	writeConfig("seed-2-lcg32 ", "1328964554", false);
	writeConfig("seed-3-lcg32 ", "1328964554", false);
	writeConfig("seed-4-lcg32 ", "1328964554", false);
	writeConfig("seed-5-lcg32 ", "1328964554", false);

	/* CmdEnv part */
	addConfig("\n");
	addConfig("[Cmdenv]");
	addConfigInt("event-banners", 0);

	/* parameters part */
	addConfig("\n");
	addConfig("[Parameters]");

	int nodeCount = w.nodeCountSB->value();
	addConfigInt("net.nodeCount", nodeCount);
	addConfigString("net.nodeType", w.nodeTypeCB->currentText());
	// TODO make this configurable? Dependant on chosen radiopower?
	addConfigInt("net.radioRange", 25);
	addConfigString("net.radioType", w.radioChipCB->currentText());
	addConfigString("net.macType", w.macLayerCB->currentText());
	addConfigString("net.routingType", w.routingCB->currentText());
	addConfigString("net.applicationName", w.applicationNameCB->currentText());
	
	// TODO not configurable yet (due to lack of choice ;) )
	addConfigString("net.ultrasoundType", "SimpleUS");
	addConfigString("net.radioPropagationType", "SimpleRadioPropagationModel");
	addConfigString("net.usPropagationType", "SimpleUSPropagationModel");

	// TODO: not configurable yet
	addConfigInt("net.lpl", 0);
	addConfigInt("net.lpl_on", 0);
	addConfigInt("net.lpl_off", 0);
	addConfigInt("net.siftNodes", 512);

	// TODO: move this to the general code base
	// layout nodes
	int placedNodes = 0;
	int sinkID = -1;
	double dimX, dimY, dimZ;

	dimX = strtod(w.dimXTB->text(), NULL);
	dimY = strtod(w.dimYTB->text(), NULL);
	dimZ = strtod(w.dimZTB->text(), NULL);

	if (w.fixedAnchorCB->isChecked()) {
		double x, y, z;
		sinkID = atoi(w.sinkIDTB->text());
		x = strtod(w.sinkXTB->text(), NULL);
		y = strtod(w.sinkYTB->text(), NULL);
		z = strtod(w.sinkZTB->text(), NULL);
		
		writeNode(sinkID, x, y, z);
	//	printf("%f\t%f\t%f\n", x, y, z);
		nodeCount--;
	}

	if (w.regularGridRB->isChecked()) {
		double xSpacing, ySpacing, zSpacing;
		double curX, curY, curZ;
		int row,  column,  layer;
		int rows, columns, layers;
		bool increasedLayers = false;
		bool increasedColumns = false;
		bool increasedRows = false;

		double orgDimZ;
		double edge; // i.e.: the optimal size of a cube around a node

		orgDimZ = dimZ;

		if (orgDimZ == 0) {
			dimZ = 1.0;
			edge = sqrt((dimX * dimY) / (double)nodeCount);
		} else {
			edge = cbrt((dimX * dimY * dimZ) / (double)nodeCount);
		}


//		printf("dims: %f, %f, %f; edge = %f\n", dimX, dimY, dimZ, edge);

		layers = (int)floor(dimZ / edge);
		rows = (int)floor(dimY / edge);
		columns = (int)floor(dimX / edge);

		// ensure minimum dimensions
		if (orgDimZ == 0)
			layers = 1;
		if (rows == 0)
			rows = 1;
		if (columns == 0)
			columns = 1;

//		printf("before viagra: layers: %d, row: %d, cols: %d, leftovers: %d\n", layers, rows, columns, nodeCount - (layers * rows * columns));

		while (nodeCount > (layers * rows * columns)) {
			// viagra time: increasing size...
			if (rows >= columns && rows >= layers && !increasedRows) {
				rows++;
				increasedRows = true;
			} else if (columns >= rows && columns >= layers  && !increasedColumns) {
				columns++;
				increasedColumns = true;
			} else if (orgDimZ != 0 && !increasedLayers) {
				layers++;
				increasedLayers = true;
			} else {
				// already increased most likely candidate
				if (increasedRows)
					columns++;
				else
					rows++;
			}
		}

//		printf("after viagra: layers: %d, row: %d, cols: %d, leftovers: %d\n", layers, rows, columns, nodeCount - (layers * rows * columns));
		
		xSpacing = dimX    / columns;
		ySpacing = dimY    / rows;
		zSpacing = orgDimZ / layers;

		curZ = 0.5 * zSpacing;
		for (layer = 0; layer < layers; layer++) {
			curY = 0.5 * ySpacing;
			for (row = 0; row < rows; row++) {
				curX = 0.5 * xSpacing;
				column = 0;
				for (column = 0; column < columns; column++) {
					int nr;
					nr = (placedNodes == sinkID ? nodeCount : placedNodes);
					writeNode(nr, curX, curY, curZ);
					//printf("%f\t%f\t%f\n", curX, curY, curZ);
					curX += xSpacing;
					if (++placedNodes >= nodeCount)
						goto endPlacement;
				}
				curY += ySpacing;
			}
			curZ += zSpacing;
		}
	} else {
		// random placement
		int i;
		int nr;
		double x, y, z;

		srandom(time(NULL));

		for (i = 0; i < nodeCount; i++) {
			nr = (i == sinkID ? nodeCount : i);
			x = fmod(random(), dimX);
			y = fmod(random(), dimY);
			if (dimZ != 0)
				z = fmod(random(), dimZ);
			else
				z = 0;
			writeNode(nr, x, y, z);
			printf("%f\t%f\t%f\n", x, y, z);
		}
	}

endPlacement:
	// end writing config file

	finish();
	
	return ret;
}
