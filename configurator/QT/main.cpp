#include "main.h"

int main( int argc, char ** argv ) {
	int ret;
	char input[512];
	char* argvCpy;

	argvCpy = (char*)malloc(strlen(argv[0]) + 1);
	strcpy(argvCpy, argv[0]);

	QApplication a(argc, argv);
	GeneratorWizard w;

	// get dynamic parts here (if possible)
	int result, i;
	size_t lineLength;
	ssize_t read = 0;
	FILE* file;
	char* lineContents;
	char* sourceDirString = (char*)malloc(1024);
	char* fileString = (char*)malloc(1024);
	struct dirent** baseFiles;
	QRegExp rx("^\\s*Define_Module_Like\\s*\\(\\s*(\\w*)\\s*,\\s*(\\w*)\\s*\\)");
	QString captured;
	modules map;
	pair<modules::iterator, modules::iterator> modIt;
	Module* baseModules;

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

	init("omnetpp.ini", sourceDirString);

	baseModules = findBaseModules();
	if (baseModules == NULL) {
		printf("Couldn't find base modules!\n");
		exit(EXIT_FAILURE);
	}

skipComponentScan:
	w.show();
//	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	ret = a.exec();
	
	// write config file here
	
	/* general part */
	/*QTime maxSim = ui.maxSimTime->time();
	sprintf(input, "%dd%dh%dm%ds%c", ui.maxSimDaysSB->value(), maxSim.hour(), maxSim.minute(), maxSim.second(), '\0');
	writeConfig("sim-time-limit", input, false);
	QTime maxCPU = ui.maxCPUTime->time();
	sprintf(input, "%dd%dh%dm%ds%c", ui.maxCPUDaysSB->value(), maxCPU.hour(), maxCPU.minute(), maxCPU.second(), '\0');
	writeConfig("cpu-time-limit", input, false);

	startParameterSection();*/

	//int nodeCount = w.nodeCountSB->value();
	//addConfigInt("net.nodeCount", nodeCount);
	//addConfigString("net.nodeType", w.nodeTypeCB->currentText());
	// TODO make this configurable? Dependant on chosen radiopower?
	addConfigInt("net.radioRange", 25);
	//addConfigString("net.radioType", w.radioChipCB->currentText());
	//addConfigString("net.macType", w.macLayerCB->currentText());
	//addConfigString("net.routingType", w.routingCB->currentText());
//	addConfigString("net.applicationName", w.applicationNameCB->currentText());
	
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

	//dimX = strtod(w.dimXTB->text(), NULL);
	//dimY = strtod(w.dimYTB->text(), NULL);
	//dimZ = strtod(w.dimZTB->text(), NULL);

/*	if (w.fixedAnchorCB->isChecked()) {
		double x, y, z;
		sinkID = atoi(w.sinkIDTB->text());
		x = strtod(w.sinkXTB->text(), NULL);
		y = strtod(w.sinkYTB->text(), NULL);
		z = strtod(w.sinkZTB->text(), NULL);
		
		writeNode(sinkID, x, y, z);
	//	printf("%f\t%f\t%f\n", x, y, z);
		//nodeCount--;
	}*/

	/*if (w.regularGridRB->isChecked()) {
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
			//edge = sqrt((dimX * dimY) / (double)nodeCount);
		} else {
			//edge = cbrt((dimX * dimY * dimZ) / (double)nodeCount);
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

		/ *while (nodeCount > (layers * rows * columns)) {
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
		}* /

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
					//nr = (placedNodes == sinkID ? nodeCount : placedNodes);
					writeNode(nr, curX, curY, curZ);
					//printf("%f\t%f\t%f\n", curX, curY, curZ);
					curX += xSpacing;
					/ *if (++placedNodes >= nodeCount)
						goto endPlacement;* /
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

		/ *for (i = 0; i < nodeCount; i++) {
			nr = (i == sinkID ? nodeCount : i);
			x = fmod(random(), dimX);
			y = fmod(random(), dimY);
			if (dimZ != 0)
				z = fmod(random(), dimZ);
			else
				z = 0;
			writeNode(nr, x, y, z);
			printf("%f\t%f\t%f\n", x, y, z);
		}* /
	}*/

//endPlacement:
	// end writing config file

	finish();
	
	return ret;
}

