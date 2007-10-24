#include <stdlib.h>
#include <math.h>

#include "Scenario.h"

#define MIN_FUN(type) type min_##type(type a, type b) { return (a < b ? a : b) ;}
#define MAX_FUN(type) type max_##type(type a, type b) { return (a > b ? a : b) ;}

MAX_FUN(int)

/**
 * Generates cols * rows unknown nodes in a grid configuration. Node 0 is
 * the anchor node, that can be used as a mobile anchor.
 * 
 * The radio range is the minimum two-hop distance.
 */
int create_configuration(int cols, int rows) {
	num_anchors = 1;
	num_nodes = cols * rows + num_anchors;
	nr_dims = 2;
	bound = 500;
	range = 2 * bound / max_int(cols, rows);

	node = new node_info[num_nodes];
	
	node[0].anchor = true;
	node[0].true_pos[0] = 0.0;
	node[0].true_pos[1] = 0.0;
	node[0].true_pos[2] = 0.0;

	double x_offset = bound / (cols);
	double y_offset = bound / (rows);
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			int id = y * cols + x + 1;
			node[id].anchor = false;
			node[id].true_pos[0] = x_offset * (x + 0.5);
			node[id].true_pos[1] = y_offset * (y + 0.5);
			node[id].true_pos[2] = 0;
		}
	}
	return true;
}

int main(int argc, char ** argv, char ** envp)
{
	// Initial values
	int rows, cols;
	cols = atoi(argv[1]);
	rows = atoi(argv[2]);

	if (!create_configuration(cols, rows))
		abort();

	write_configuration(num_nodes);

	delete node;
	return 0;
}
