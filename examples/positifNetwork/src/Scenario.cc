#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "Scenario.h"

node_info *node;

int num_nodes;
int num_anchors;
unsigned int nr_dims;
FLOAT bound;
FLOAT range;

void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "PositifScenario: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

FLOAT distance(Position a, Position b)
{
	FLOAT sumsqr = 0;
	for (unsigned int i = 0; i < nr_dims; i++) {
		sumsqr += (a[i] - b[i]) * (a[i] - b[i]);
	}
	return sqrt(sumsqr);
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

int write_configuration(int node_count)
{
	FLOAT factor = 500.0 / bound;
// 	FLOAT factor = 1;
	printf("sim.numHosts = %d\n", num_nodes);
	printf("\n");
	printf("sim.world.use2D = %s\n", (nr_dims == 2?"true":"false"));
	printf("sim.playgroundSizeX = %lg\n", bound * factor);
	printf("sim.playgroundSizeY = %lg\n", bound * factor);
	printf("sim.playgroundSizeZ = %lg\n", bound * factor);
	printf("\n");
	printf("sim.propagationmodel.radioRange = %lg\n", range * factor);
	printf("\n");
	printf("sim.node[*].loc.anchor_frac = %lg\n", num_anchors / (double)num_nodes);
	printf("\n");

	for (int n = 0; n < node_count; n++) {
		switch (nr_dims) {
		case 2: printf("sim.node[%d].mobility.z = %lg\n", n, node[n].true_pos[2] * factor);
			/* FALLTHROUGH */
		case 1: printf("sim.node[%d].mobility.y = %lg\n", n, node[n].true_pos[1] * factor);
			/* FALLTHROUGH */
		case 0: printf("sim.node[%d].mobility.x = %lg\n", n, node[n].true_pos[0] * factor);
			break;
		default:
			error("Invalid number of dimensions");
			abort();
		}
		if (node[n].anchor) {
			printf("sim.node[%d].loc.isAnchor = true\n", n);
		} else {
			printf("sim.node[%d].loc.isAnchor = false\n", n);
		}
		printf("\n");
	}
    
	return TRUE;
}
