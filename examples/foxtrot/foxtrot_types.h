#ifndef FOXTROT_TYPES_H
#define FOXTROT_TYPES_H

typedef double foxtrot_point;
#define FP_PRINTF "%lf"
typedef uint32_t foxtrot_point_int; // intermediary storage for manipulation (e.g addition) of many foxtrot_points

typedef struct
{
	foxtrot_point min;
	foxtrot_point max;
} foxtrot_data;

void foxtrot_data_set(foxtrot_data *pt, const foxtrot_point a, const foxtrot_point b);
void foxtrot_data_uniset(foxtrot_data *pt, const foxtrot_point a);

typedef uint32_t seconds_t;

typedef struct
{
	foxtrot_point x;
	foxtrot_point y;
} point_ft;

#define PT(x) (x+3)

#endif
