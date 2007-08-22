#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "Position.h"

#define FALSE 0
#define TRUE !FALSE

typedef struct {
	int ID;			// may be different from index in 'node'
	bool anchor;
	Position true_pos;
	Position init_pos;
} node_info;

void error(char *fmt, ...)
    __attribute__ ((format(printf, 1, 2)));

void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "PositifScenario: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

node_info *node;

static int num_nodes;
static int num_anchors;
static unsigned int nr_dims;
static FLOAT bound;
static FLOAT range;

int write_configuration()
{
	FLOAT factor = 500.0 / bound;
// 	FLOAT factor = 1;
	printf("sim.numHosts = %d\n", num_nodes);
	printf("\n");
	printf("sim.world.playgroundSizeX = %lg\n", bound * factor);
	printf("sim.world.playgroundSizeY = %lg\n", bound * factor);
	printf("sim.world.playgroundSizeZ = %lg\n", bound * factor);
	printf("\n");
	printf("sim.world.use2D = %s", (nr_dims == 2?"true":"false"));
	printf("\n");
	printf("sim.channelcontrol.radioRange = %lg\n", range * factor);
	printf("\n");

	for (int n = 0; n < num_nodes; n++) {
		for (unsigned int d = 0; d < nr_dims; d++) {
			switch (d) {
			case 0:
				printf("sim.node[%d].mobility.x = %lg\n", n, node[n].true_pos[d] * factor);
				break;
			case 1:
				printf("sim.node[%d].mobility.y = %lg\n", n, node[n].true_pos[d] * factor);
				break;
			case 2:
				printf("sim.node[%d].mobility.z = %lg\n", n, node[n].true_pos[d] * factor);
				break;
			default:
				error ("Invalid number of dimensions");
				abort();
			}
		}
		if (node[n].anchor) {
			printf("sim.node[%d].loc.anchor = true\n", n);
		} else {
			printf("sim.node[%d].loc.anchor = false\n", n);
		}
		printf("\n");
	}
    
	return TRUE;
}


int ID2idx(int ID)
{
	for (int n = 0; n < num_nodes; n++) {
		if (node[n].ID == ID) {
			return n;
		}
	}
	error("Read StandardFormat did not find position of node %d", ID);
	abort();
}

FLOAT distance(Position a, Position b)
{
	FLOAT sumsqr = 0;
	for (unsigned int i = 0; i < nr_dims; i++) {
		sumsqr += (a[i] - b[i]) * (a[i] - b[i]);
	}
	return sqrt(sumsqr);
}

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
	if (!read_configuration(argv[1]))
		abort();

	write_configuration();

	delete node;
	return 0;
}
