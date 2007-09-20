#include <assert.h>
#include <stdlib.h>

#include "Scenario.h"

/*
 * This function initializes the first <num_nodes> nodes as anchors.
 * All positions will be randomly initialized by OmnetPP/MiXiM.
 */

int create_configuration()
{
	node = new node_info[num_nodes];

	nr_dims = 2;
	bound = 500;
	range = 220;

	for (int id = 0; id < num_anchors; id++) {
		node[id].anchor = true;
		node[id].true_pos[0] = drand48() * bound;
		node[id].true_pos[1] = drand48() * bound;
		node[id].true_pos[2] = 0;
	}
	return true;
}

int main(int argc, char **argv, char **envp)
{
	assert(argc == 4);
	num_nodes = atoi(argv[1]);
	num_anchors = atoi(argv[2]);
	assert(num_anchors <= num_nodes);
	if (!create_configuration())
		abort();

	write_configuration(num_anchors);
		
	delete node;
	return 0;
}
