#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>

#include "Scenario.h"

int read_configuration(const char *filename)
{
	FILE *f = fopen(filename, "r");

	if (f == NULL) {
		error("PositifScenario: can't open '%s'", filename);
		return FALSE;
	}

	int r;
// 	FLOAT area;

	r = fscanf(f, "%d %g %g", &nr_dims, &bound, &range);
	if (r != 3) {
		error("fscanf did not find the 'nr-dimensions', 'grid-bound', and 'radio-range'");
		return FALSE;
	}
// 	area = bound;
// 	for (unsigned int d = 1; d < nr_dims; d++)
// 		area *= bound;

	r = fscanf(f, "%d %d", &num_nodes, &num_anchors);
	if (r != 2) {
		error("fscanf did not find the 'nr-nodes' and 'nr-anchors'");
		return FALSE;
	}

	node = new node_info[num_nodes];

	// Coordinates
	for (int i = 0; i < num_nodes; i++) {
		node[i].anchor = false;
		r = fscanf(f, "%d", &node[i].ID);
		if (r != 1) {
			error("fscanf did not find ID of node %d", i);
			return FALSE;
		}
		for (unsigned int d = 0; d < nr_dims; d++) {
			r = fscanf(f, "%g", &node[i].true_pos[d]);
			if (r != 1) {
				error
				    ("fscanf did not find coordinate %d of node %d",
				     d, node[i].ID);
				return FALSE;
			}
			node[i].init_pos[d] = node[i].true_pos[d];
			if (node[i].true_pos[d] > bound) {
				error
				    ("node %d: coordinate out of bounds (%g > %g)",
				     node[i].ID, node[i].true_pos[d], bound);
				return FALSE;
			}
		}
//              ev << "node " << node[i].ID << "'s true location: " << pos2str(node[i].true_pos) << "\n";
		while (fgetc(f) != '\n');
	}

	// Anchors
	for (int i = 0; i < num_anchors; i++) {
		int n;
		r = fscanf(f, "%d", &n);
		if (r != 1) {
			error("fscanf did not find ID of anchor %d", i);
			return FALSE;
		}
		node[ID2idx(n)].anchor = true;
	}

	// Blurred ranges
	int i_ID, j_ID;
	double dist;
	while ((r =
		fscanf(f, "%d%*[ \t]%d%*[ \t]%lg\n", &i_ID, &j_ID,
		       &dist)) != EOF) {
// 		int i = ID2idx(i_ID);
// 		int j = ID2idx(j_ID);

// 		if (r == 2) {
// 			dist = distance(node[i].true_pos, node[j].true_pos);
// 			fprintf(stderr, "computed distance between %d %d: %g\n", i_ID,
// 			       j_ID, dist);
// 		} else if (r != 3) {
// 			error("fscanf did not find distance between %d and %d",
// 			      i_ID, j_ID);
// 			return FALSE;
// 		}
// 		if (i != j && 0 < dist && dist <= range) {
// 			fprintf(stderr, "[%d -> %d] true_dist: %lg, est_dist: %lg\n",
// 			       i_ID, j_ID,
// 			       distance(node[i].true_pos, node[j].true_pos),
// 			       dist);
// 		}
		if (r < 2 || r > 3) {
			error("fscanf did not find distance between %d and %d",
			      i_ID, j_ID);
			return FALSE;
		}
	}
	return TRUE;
}

int main(int argc, char **argv, char **envp)
{
	assert(argc == 2);
	if (!read_configuration(argv[1]))
		abort();

	write_configuration(num_nodes);

	delete node;
	return 0;
}
