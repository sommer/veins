#include "simpleUSPropagationModel.h"

Define_Module_Like(SimpleUSPropagationModel, PropagationModelClass);

void SimpleUSPropagationModel::initialize() {
	PropagationModel::initialize();
	printfNoInfo(PRINT_INIT, "Initializing simple US propagation model...");
}

void SimpleUSPropagationModel::finish() {
	printfNoInfo(PRINT_INIT, "Ending simple US propagation model...");
	PropagationModel::finish();
}

