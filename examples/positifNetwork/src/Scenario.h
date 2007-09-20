#ifndef _SCENARIO_H
#define _SCENARIO_H

#include "Position.h"

#define FALSE 0
#define TRUE !FALSE

typedef struct {
	int ID;			// may be different from index in 'node'
	bool anchor;
	Position true_pos;
	Position init_pos;
} node_info;

extern node_info *node;

extern int num_nodes;
extern int num_anchors;
extern unsigned int nr_dims;
extern FLOAT bound;
extern FLOAT range;

void error(char *fmt, ...) __attribute__ ((format(printf, 1, 2)));
FLOAT distance(Position a, Position);
int ID2idx(int ID);
int write_configuration(int node_count);

#endif /* _SCENARIO_H */
