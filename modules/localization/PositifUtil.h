#ifndef _POSITIF_UTIL_H
#define _POSITIF_UTIL_H

#include <assert.h>

#include <omnetpp.h>
#include <string>
using std::string;

#ifndef EV
#define EV ev
#define EV_clear ev
#define coreEV ev
#define coreEV_clear ev
#endif

#include "Position.h"
#include "cores.h"

class PositifUtil {
public:
	static int nr_dims;
	static bool use_confs;

	FLOAT distance(Position, Position);
	FLOAT savvides_minmax(int n_pts, FLOAT ** positions,
			      FLOAT * ranges, FLOAT * confs, int target);
	FLOAT triangulate(int n_pts, FLOAT ** positions,
			  FLOAT * ranges, FLOAT * weights, int target);
	FLOAT hoptriangulate(int n_pts, FLOAT ** positions,
			     FLOAT * ranges, int target);

	string pos2str(Position a) {
		Coord pos = positionToCoord(a, nr_dims);
		return pos.info();
	}
};

#endif /* _POSITIF_UTIL_H */
