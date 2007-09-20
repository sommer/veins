#include <assert.h>
#include <stdlib.h>

#include "Scenario.h"

/* This program generates a topology with random placed nodes, but
 * with fixed placed anchors.
 */

int create_configuration(int cols, int rows)
{
	node = new node_info[num_nodes];

	num_anchors = cols * rows;
	assert(num_anchors <= num_nodes);
	nr_dims = 2;
	bound = 500;
	range = 220;

	double x_offset = bound / (cols - 1);
	double y_offset = bound / (rows - 1);
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			int id = y * cols + x;
			node[id].anchor = true;
			node[id].true_pos[0] = x_offset * x;
			node[id].true_pos[1] = y_offset * y;
			node[id].true_pos[2] = 0;
		}
	}
	return true;
}

int main(int argc, char **argv, char **envp)
{
	assert(argc == 4);
	num_nodes = atoi(argv[1]);
	int cols = atoi(argv[2]);
	assert(cols > 1);
	int rows = atoi(argv[3]);
	assert(rows > 1);
	if (!create_configuration(cols, rows))
		abort();

	write_configuration(num_anchors);
		
	delete node;
	return 0;
}
