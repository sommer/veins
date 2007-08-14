#ifndef FOXTROT_TYPES_H
#define FOXTROT_TYPES_H

#include "winsupport.h"

typedef double foxtrot_point;
#define FP_PRINTF "%lf"
typedef uint32_t foxtrot_point_int;	// intermediary storage for manipulation (e.g addition) of many foxtrot_points

typedef struct
{
	foxtrot_point min;
	foxtrot_point max;
} foxtrot_data;

void foxtrot_data_set(foxtrot_data * pt, const foxtrot_point a, const foxtrot_point b);
void foxtrot_data_uniset(foxtrot_data * pt, const foxtrot_point a);

typedef uint32_t seconds_t;

typedef struct
{
	foxtrot_point x;
	foxtrot_point y;
} point_ft;

#define AXES 2
#define NO_TIME UINT_MAX

#include "bits.h"

#define GRID_WIDTH 8
#define GRID_PT(x,y) (y)*GRID_WIDTH+(x)
#define GRID_SET(set,x,y) ANY_SET(GRID_PT(x,y),set)
#define GRID_CLR(set,x,y) ANY_CLR(GRID_PT(x,y),set)
#define GRID_CHK(set,x,y) ANY_CHK(GRID_PT(x,y),set)
#define GRID_BYTES BYTES_USED(GRID_WIDTH*GRID_WIDTH)
#define GRID_RESET(set) ANY_RESET(set,GRID_PT(GRID_WIDTH,GRID_WIDTH))
#define GRID_FILL(set) ANY_FILL(set,GRID_PT(GRID_WIDTH,GRID_WIDTH))
#define NEW_GRID(set) uint8_t set[GRID_PT(GRID_WIDTH,GRID_WIDTH)]

#endif
